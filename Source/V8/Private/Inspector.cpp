#include "V8PCH.h"
#include "../../Launch/Resources/Version.h"

#ifndef THIRD_PARTY_INCLUDES_START
#	define THIRD_PARTY_INCLUDES_START
#	define THIRD_PARTY_INCLUDES_END
#endif

PRAGMA_DISABLE_SHADOW_VARIABLE_WARNINGS

#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 14) || ENGINE_MAJOR_VERSION > 4
static const int32 CONTEXT_GROUP_ID = 1;

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#endif
#define UI UI_ST
THIRD_PARTY_INCLUDES_START
#include "libwebsockets.h"
THIRD_PARTY_INCLUDES_END
#undef UI
#if PLATFORM_WINDOWS
#include "Windows/HideWindowsPlatformTypes.h"
#endif

THIRD_PARTY_INCLUDES_START
#include "v8-inspector.h"
#include "v8-platform.h"
#include "libplatform/libplatform.h"
THIRD_PARTY_INCLUDES_END

#if WITH_EDITOR 
#include "TickableEditorObject.h"
typedef FTickableEditorObject FTickableAnyObject;
#else
#include "Tickable.h"
typedef FTickableGameObject FTickableAnyObject;
#endif

#include "Helpers.h"
#include "Config.h"
#include "Containers/Queue.h"

using namespace v8;

enum class EAgentImplType : uint8
{
	AgentBase,
	Channel,
	HttpResponseChannel,
};

namespace {
	class AgentImpl : public v8_inspector::V8Inspector::Channel, public TSharedFromThis<AgentImpl>
	{
	public:
		AgentImpl(v8::Isolate* isolate, EAgentImplType AgentImplType = EAgentImplType::AgentBase)
			: AgentImplType(AgentImplType)
			, isolate_(isolate)
		{}

		virtual ~AgentImpl() {}

	
		const EAgentImplType AgentImplType;
		v8::Isolate* isolate_;
		TArray<uint8> Buffer;

		void receive(void *Data, size_t Size, size_t Remaining)
		{
			Buffer.Append((uint8*)Data, Size);

			// Is this the final packet?
			if (Remaining == 0)
			{
				enqueueFrontendMessage(MoveTemp(Buffer));
				Buffer.Empty();
			}
		}

		void DispatchMessages()
		{
			// Dispatch within scope
			Isolate::Scope isolate_scope(isolate_);

			// Copy messages first from queue
			TArray<TArray<uint8>> Messages = MoveTemp(PendingMessages);
			PendingMessages.Empty();

			// Pump messages
			for (auto& Buffer : Messages)
			{
				dispatchFrontendMessage(Buffer);
			}
		}

		virtual void dispatchFrontendMessage(const TArray<uint8>& Buffer) {}
		virtual void PostReceiveMessage() {}
		virtual void OnWritable() {}

		void enqueueFrontendMessage(TArray<uint8>&& Buffer)
		{
			PendingMessages.Add(Buffer);
			PostReceiveMessage();
		}

		TArray<TArray<uint8>> PendingMessages;
	};

	struct AgentRef
	{
		TSharedPtr<AgentImpl> agent;
	};

	void InterruptCallback(v8::Isolate*, void* _ref)
	{
		auto ref = reinterpret_cast<AgentRef*>(_ref);
		ref->agent->DispatchMessages();
		delete ref;
	}

	class DispatchOnInspectorBackendTask : public v8::Task
	{
	public:
		explicit DispatchOnInspectorBackendTask(TSharedPtr<AgentImpl> agent)
			: agent_(agent)
		{}

		void Run() override
		{
			agent_->DispatchMessages();
		}

	private:
		TSharedPtr<AgentImpl> agent_;
	};

	class ChannelImpl : public AgentImpl
	{
	public:
		v8::Platform* platform_;
		lws_context* WebSocketContext;
		lws* Wsi;
		std::unique_ptr<v8_inspector::V8InspectorSession> v8session;
		TQueue<TArray<uint8>> OutgoingBuffer;
		bool bFlushing = false;

		ChannelImpl(lws_context *InContext, lws* InWsi, v8::Isolate* isolate, v8::Platform* platform, const std::unique_ptr<v8_inspector::V8Inspector>& v8inspector)
			: AgentImpl(isolate, EAgentImplType::Channel), platform_(platform), WebSocketContext(InContext), Wsi(InWsi)
		{
			v8_inspector::StringView state;
			v8session = v8inspector->connect(CONTEXT_GROUP_ID, this, state);
		}

		virtual void dispatchFrontendMessage(const TArray<uint8>& Buffer) override
		{
			TArray<uint8> str(Buffer);
			str.Add(0);

			FUTF8ToTCHAR unicode((char*)str.GetData());

			// For platforms in where TCHAR is not 16 bit.
			TArray<uint16> chars;
			auto* buf = unicode.Get();
			auto len = unicode.Length();
			for (decltype(len) i=0; i<len; ++i)
			{
				chars.Add(buf[i]);
			}

			v8_inspector::StringView messageview((uint16_t*)chars.GetData(), len);
			v8session->dispatchProtocolMessage(messageview);
		}

		virtual void PostReceiveMessage() override
		{
			std::shared_ptr<v8::TaskRunner> taskrunner = platform_->GetForegroundTaskRunner(isolate_);
			taskrunner->PostTask(std::make_unique<DispatchOnInspectorBackendTask>(AsShared()));
			isolate_->RequestInterrupt(InterruptCallback, new AgentRef{AsShared()});
		}

		void sendMessageToFrontend(std::unique_ptr<v8_inspector::StringBuffer> message)
		{
			auto view = message->string();
			TArray<uint16> str;
			str.Append(view.characters16(), view.length());
			str.Add(0);

			// For platforms in where TCHAR is not 16 bit.
			TArray<TCHAR> chars;
			for (auto ch : str)
			{
				chars.Add(ch);
			}

			FTCHARToUTF8 utf8(chars.GetData());
			sendMessage(utf8.Get(), utf8.Length());
		}

		void sendResponse(int callId, std::unique_ptr<v8_inspector::StringBuffer> message) override
		{
			sendMessageToFrontend(std::move(message));
		}

		void sendNotification(std::unique_ptr<v8_inspector::StringBuffer> message) override
		{
			sendMessageToFrontend(std::move(message));
		}

		virtual void installAdditionalCommandLineAPI(v8::Local<v8::Context>,
			v8::Local<v8::Object>)
		{
			UE_LOG(LogJavascript, Log, TEXT("Received command line api"));
		}

		void sendMessage(const void* Data, uint32 Size)
		{
			TArray<uint8> Buffer;

			// Reserve space for WS header data
			Buffer.AddDefaulted(LWS_PRE);
			Buffer.Append((uint8*)Data, Size);
			OutgoingBuffer.Enqueue(Buffer);
		}

		void OnWritable() override
		{
			if (OutgoingBuffer.IsEmpty())
			{
				return;
			}

			if (FV8Config::FlushInspectorWebSocketOnWritable() && bFlushing == false)
			{
				Flush();
			}
			else
			{
				TArray<uint8> Packet;
				// this is very inefficient we need a constant size circular buffer to efficiently not do unnecessary allocations/deallocations.
				if (OutgoingBuffer.Dequeue(Packet))
				{
					uint32 TotalDataSize = Packet.Num() - LWS_PRE;
					uint32 DataToSend = TotalDataSize;
					while (DataToSend)
					{
						int Sent = lws_write(Wsi, Packet.GetData() + LWS_PRE + (DataToSend - TotalDataSize), DataToSend, (lws_write_protocol)LWS_WRITE_TEXT);
						if (Sent < 0)
						{
							UE_LOG(LogJavascript, Warning, TEXT("Could not write any bytes to socket"));
							return;
						}
						if ((uint32)Sent < DataToSend)
						{
							UE_LOG(LogJavascript, Warning, TEXT("Could not write all '%d' bytes to socket"), DataToSend);
						}
						DataToSend -= Sent;
					}
				}
			}
		}

		void Flush()
		{
			if (bFlushing)
				return;

			bFlushing = true;

			while (OutgoingBuffer.IsEmpty() == false)
			{
				lws_callback_on_writable(Wsi);
				lws_service(WebSocketContext, 0);
			}

			bFlushing = false;
		}

		void flushProtocolNotifications()
		{}
	};

	class HttpResponseChannelImpl : public AgentImpl
	{
	public:
		enum class ERequestedUriType
		{
			Unknown,
			JsonVersion,
			JsonList
		};

		static ERequestedUriType GetRequestedUriType(const FString& InUri)
		{
			if (InUri == TEXT("/json/version"))
				return ERequestedUriType::JsonVersion;
			else if (InUri == TEXT("/json") || InUri == TEXT("/json/list"))
				return ERequestedUriType::JsonList;

			return ERequestedUriType::Unknown;
		}

		lws* Wsi;
		
		TQueue<TArray<uint8>> OutgoingBuffer;

		// These members are not related with this channel itself but with the context for this channel.
		// Consider to move them to PerSessionDataServer.
		bool bHeaderSent = false;
		bool bBodySent = false;

		HttpResponseChannelImpl(lws* InWsi, v8::Isolate* isolate)
			: AgentImpl(isolate, EAgentImplType::HttpResponseChannel), Wsi(InWsi)
		{
		}

		void SendResponse(const FString& InMessage)
		{
			FTCHARToUTF8 utf8(*InMessage);
			sendMessage(utf8.Get(), utf8.Length());
		}

		void sendResponse(int callId, std::unique_ptr<v8_inspector::StringBuffer> message) override
		{
			sendMessageToFrontend(std::move(message));
		}

		void sendNotification(std::unique_ptr<v8_inspector::StringBuffer> message) override
		{
			sendMessageToFrontend(std::move(message));
		}

		void sendMessageToFrontend(std::unique_ptr<v8_inspector::StringBuffer> message)
		{
			auto view = message->string();
			TArray<uint16> str;
			str.Append(view.characters16(), view.length());
			str.Add(0);

			// For platforms in where TCHAR is not 16 bit.
			TArray<TCHAR> chars;
			for (auto ch : str)
			{
				chars.Add(ch);
			}

			FTCHARToUTF8 utf8(chars.GetData());
			sendMessage(utf8.Get(), utf8.Length());
		}

		void OnWritable() override
		{
			if (OutgoingBuffer.IsEmpty())
				return;

			TArray<uint8> Packet;
			// this is very inefficient we need a constant size circular buffer to efficiently not do unnecessary allocations/deallocations.
			if (OutgoingBuffer.Dequeue(Packet))
			{
				uint32 TotalDataSize = Packet.Num() - LWS_PRE;
				uint32 DataToSend = TotalDataSize;
				while (DataToSend)
				{
					int Sent = lws_write_http(Wsi, Packet.GetData() + LWS_PRE + (DataToSend - TotalDataSize), DataToSend);
					if (Sent < 0)
					{
						UE_LOG(LogJavascript, Warning, TEXT("Could not write any response to socket"));
						return;
					}
					if ((uint32)Sent < DataToSend)
					{
						UE_LOG(LogJavascript, Warning, TEXT("Could not write all '%d' bytes to socket"), DataToSend);
					}
					DataToSend -= Sent;
				}
			}
		}

		void flushProtocolNotifications() {}

	private:
		void sendMessage(const void* Data, uint32 Size)
		{
			TArray<uint8> Buffer;

			// Reserve space for WS header data
			Buffer.AddDefaulted(LWS_PRE);
			Buffer.Append((uint8*)Data, Size);
			OutgoingBuffer.Enqueue(Buffer);
		}
	};
}


class FInspector : public IJavascriptInspector, public FTickableAnyObject, public v8_inspector::V8InspectorClient, public FOutputDevice
{
public:
	Isolate* isolate_;
	Persistent<Context> context_;
	bool terminated_{ false };
	bool running_nested_loop_{ false };
	std::unique_ptr<v8_inspector::V8Inspector> v8inspector;
	v8::Platform* platform_;
	bool IsAlive{ false };
	lws_context* WebSocketContext;
	lws_protocols* WebSocketProtocols;
	int32 Port;

	FString WebSocketDebuggerUrl() const
	{
		return FString::Printf(TEXT("ws://127.0.0.1:%d"), Port);
	}

	FString DevToolsFrontEndUrl() const
	{
		return FString::Printf(TEXT("devtools://devtools/bundled/inspector.html?v8only=true&ws=127.0.0.1:%d"), Port);
	}

	FInspector(v8::Platform* platform, int32 InPort, Local<Context> InContext)
		: Port(InPort)
	{
		platform_ = platform;
		isolate_ = InContext->GetIsolate();
		context_.Reset(isolate_, InContext);

		FIsolateHelper I(isolate_);

		{
			auto console = InContext->Global()->Get(InContext, I.Keyword("console"));
			if (!console.IsEmpty())
			{
				(void)InContext->Global()->Set(InContext, I.Keyword("$console"), console.ToLocalChecked());
			}
		}

		v8inspector = v8_inspector::V8Inspector::create(isolate_, this);
		const uint8_t CONTEXT_NAME[] = "Unreal.js";
		v8_inspector::StringView context_name(CONTEXT_NAME, sizeof(CONTEXT_NAME) - 1);
		v8inspector->contextCreated(v8_inspector::V8ContextInfo(InContext, CONTEXT_GROUP_ID, context_name));

		Install(InPort);

		UE_LOG(LogJavascript, Log, TEXT("open %s"), *DevToolsFrontEndUrl());

		InstallRelay();
	}

	~FInspector()
	{
		Uninstall();

		context_.Reset();

		UninstallRelay();
	}

	void InstallRelay()
	{
		GLog->AddOutputDevice(this);
		GLog->SerializeBacklog(this);
	}

	void UninstallRelay()
	{
		// At shutdown, GLog may already be null
		if (GLog != nullptr)
		{
			GLog->RemoveOutputDevice(this);
		}
	}

	virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category) override
	{
		static FName NAME_Javascript("Javascript");

		if (Category != NAME_Javascript)
		{
			HandleScope handle_scope(isolate_);

			FIsolateHelper I(isolate_);
			auto context_ = context();
			Isolate::Scope isolate_scope(isolate_);
			Context::Scope context_scope(context_);

			TryCatch try_catch(isolate_);

			auto maybe_console = context_->Global()->Get(context_, I.Keyword("console"));
			if (!maybe_console.IsEmpty())
			{
				auto console = maybe_console.ToLocalChecked().As<v8::Object>();

				auto method =
					Verbosity == ELogVerbosity::Fatal || Verbosity == ELogVerbosity::Error ? I.Keyword("$error") :
					Verbosity == ELogVerbosity::Warning ? I.Keyword("$warn") :
					Verbosity == ELogVerbosity::Display ? I.Keyword("info") :
					I.Keyword("$log");

				auto maybe_function = console->Get(context_, method);

				if (!maybe_function.IsEmpty())
				{
					auto function = maybe_function.ToLocalChecked().As<v8::Function>();

					if (Verbosity == ELogVerbosity::Display)
					{
						v8::Handle<v8::Value> argv[2];
						argv[0] = I.String(FString::Printf(TEXT("%%c%s: %s"), *Category.ToString(), V));
						argv[1] = I.String(TEXT("color:gray"));
						(void)function->Call(context(), console, 2, argv);
					}
					else
					{
						v8::Handle<v8::Value> argv[1];
						argv[0] = I.String(FString::Printf(TEXT("%s: %s"), *Category.ToString(), V));
						(void)function->Call(context(), console, 1, argv);
					}
				}
			}
		}
	}

	virtual void Destroy() override
	{
		delete this;
	}

	Local<Context> context() { return Local<v8::Context>::New(isolate_, context_); }

	void runMessageLoopOnPause(int context_group_id) override
	{
		if (running_nested_loop_) return;
		terminated_ = false;
		running_nested_loop_ = true;
		while (!terminated_)
		{
			lws_service(WebSocketContext, 0);
			lws_callback_on_writable_all_protocol(WebSocketContext, &WebSocketProtocols[0]);

			while (v8::platform::PumpMessageLoop(platform_, isolate_))
			{
			}
		}
		terminated_ = false;
		running_nested_loop_ = false;
	}

	void quitMessageLoopOnPause() override
	{
		terminated_ = true;
	}

	double currentTimeMS() override
	{
		return FPlatformTime::Seconds() * 1000.f;
	}

	void runIfWaitingForDebugger(int contextGroupId) override
	{
	}

	v8::Local<v8::Context> ensureDefaultContextInGroup(int) override
	{
		return context();
	}

	virtual void Tick(float DeltaTime) override
	{
		if (IsAlive)
		{
			lws_service(WebSocketContext, 0);
			lws_callback_on_writable_all_protocol(WebSocketContext, &WebSocketProtocols[0]);
		}
	}

	virtual bool IsTickable() const override
	{
		return true;
	}

	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FV8Inspector, STATGROUP_Tickables);
	}

	// a object of this type is associated by libwebsocket to every connected session.
	struct PerSessionDataServer
	{
		TSharedPtr<AgentImpl> Channel; // each session is actually a socket to a client
	};

	int inspector_server( lws *Wsi, lws_callback_reasons Reason, void *User, void *In, size_t Len )
	{
		PerSessionDataServer* BufferInfo = (PerSessionDataServer*)User;
		if (!IsAlive)
		{
			return 0;
		}

		switch (Reason)
		{
		case LWS_CALLBACK_HTTP:
			{
				auto request = (char*)In;

				FString Uri = UTF8_TO_TCHAR(request);
				UE_LOG(LogJavascript, Log, TEXT("requested URI: '%s'"), *Uri);

				auto RequestUriType = HttpResponseChannelImpl::GetRequestedUriType(Uri);
				if (RequestUriType == HttpResponseChannelImpl::ERequestedUriType::Unknown)
					// Close connection
					return -1;

				auto channel = MakeShared<HttpResponseChannelImpl>(Wsi, isolate_);
				BufferInfo->Channel = channel;
				lws_set_timeout(Wsi, NO_PENDING_TIMEOUT, 0);

				// Enqueue header to send buffer, first.
				static const FString HeaderResponse = TEXT("HTTP/1.0 200 OK\r\nContent-Type: application/json; charset=UTF-8\r\nCache-Control: no-cache\r\n\r\n");
				channel->SendResponse(HeaderResponse);

				// Enqueue response body to send buffer, next.
				switch (RequestUriType)
				{
				case HttpResponseChannelImpl::ERequestedUriType::JsonVersion:
					{
						static const FString ResponseBodyCache =
							TEXT("{\r\n")
							// Current version of Unreal.JS is based on v8 6.1.534.30,
							// and the nearest version of v8 is integrated into Node.JS 8.7.
							// Refer to https://nodejs.org/en/download/releases
							TEXT("\"Browser\": \"node.js/v8.7.0\",\r\n")
							TEXT("\"Protocol-Version\": \"1.1\"\r\n")
							TEXT("}");
						channel->SendResponse(ResponseBodyCache);
					}
					break;

				case HttpResponseChannelImpl::ERequestedUriType::JsonList:
					{
						static const FString ResponseBodyCache = FString::Printf(
							TEXT("[{\r\n")
							TEXT("\"description\": \"unreal.js instance\",\r\n")
							TEXT("\"devtoolsFrontendUrl\": \"%s\",\r\n")
							TEXT("\"type\":\"node\",\r\n")
							TEXT("\"id\": \"C889497F-7C93-433D-A1F1-08F93A2F12E2\",\r\n")
							TEXT("\"title\": \"unreal.js\",\r\n")
							TEXT("\"webSocketDebuggerUrl\": \"%s/C889497F-7C93-433D-A1F1-08F93A2F12E2\"\r\n")
							TEXT("}]"),
							*DevToolsFrontEndUrl(), *WebSocketDebuggerUrl());

						channel->SendResponse(ResponseBodyCache);
					}
					break;

				default:
					// Close connection
					// * Unreachable code: (RequestUriType == HttpResponseChannelImpl::ERequestedUriType::Unknown) is handled above.
					return -1;
				}

				// Request http_writable callback
				lws_callback_on_writable_all_protocol(WebSocketContext, &WebSocketProtocols[0]);
			}
			break;

		case LWS_CALLBACK_HTTP_WRITEABLE:
			{
				if (BufferInfo->Channel.IsValid() &&
					BufferInfo->Channel->AgentImplType == EAgentImplType::HttpResponseChannel)
				{
					auto channel = StaticCastSharedPtr<HttpResponseChannelImpl>(BufferInfo->Channel);
					if (channel.IsValid())
					{
						bool* bResponseSentFlagPtr = nullptr;
						if (channel->bHeaderSent == false)
							bResponseSentFlagPtr = &channel->bHeaderSent;
						else if (channel->bBodySent == false)
							bResponseSentFlagPtr = &channel->bBodySent;

						if (bResponseSentFlagPtr != nullptr)
						{
							*bResponseSentFlagPtr = true;
							channel->OnWritable();
							lws_set_timeout(Wsi, NO_PENDING_TIMEOUT, 0);

							// Request another http_writable callback to send next packet or close connection.
							lws_callback_on_writable_all_protocol(WebSocketContext, &WebSocketProtocols[0]);
						}
						else
						{
							// Dispose channel and close connection.
							BufferInfo->Channel.Reset();
							return -1;
						}
					}
				}
			}
			break;

		case LWS_CALLBACK_ESTABLISHED:
			{
				BufferInfo->Channel = MakeShared<ChannelImpl>(
					WebSocketContext,
					Wsi,
					isolate_,
					platform_,
					v8inspector
				);
				lws_set_timeout(Wsi, NO_PENDING_TIMEOUT, 0);
			}
			break;

		case LWS_CALLBACK_RECEIVE:
			{
				auto Remaining = lws_remaining_packet_payload(Wsi);
				BufferInfo->Channel->receive(In, Len, Remaining);
				lws_set_timeout(Wsi, NO_PENDING_TIMEOUT, 0);
			}
			break;

		case LWS_CALLBACK_SERVER_WRITEABLE:
			{
				BufferInfo->Channel->OnWritable();
				lws_set_timeout(Wsi, NO_PENDING_TIMEOUT, 0);
			}
			break;

		case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
			{
				// TODO: error handling
			}
			break;

		case LWS_CALLBACK_WSI_DESTROY:
		case LWS_CALLBACK_PROTOCOL_DESTROY:
		case LWS_CALLBACK_CLOSED:
		case LWS_CALLBACK_CLOSED_HTTP:
			{
				if (BufferInfo && BufferInfo->Channel.IsValid())
				{
					BufferInfo->Channel.Reset();
				}
			}
			break;
		}

		return 0;
	}

	static void lws_debugLog(int level, const char *line)
	{
		UE_LOG(LogJavascript, Log, TEXT("websocket server: %s"), ANSI_TO_TCHAR(line));
	}

	void Install(int32 Port)
	{
		lws_set_log_level(LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_DEBUG | LLL_INFO, lws_debugLog);

		WebSocketProtocols = new lws_protocols[2];
		FMemory::Memzero(WebSocketProtocols, sizeof(lws_protocols) * 2);

		WebSocketProtocols[0].name = "inspector";
		WebSocketProtocols[0].callback = [](lws *Wsi, lws_callback_reasons Reason, void *User, void *In, size_t Len) {
			auto context = lws_get_context(Wsi);
			return reinterpret_cast<FInspector*>(lws_context_user(context))->inspector_server(Wsi, Reason, User, In, Len);
		};
		WebSocketProtocols[0].per_session_data_size = sizeof(PerSessionDataServer);
		WebSocketProtocols[0].rx_buffer_size = 10 * 1024 * 1024;

		struct lws_context_creation_info Info;
		memset(&Info, 0, sizeof(lws_context_creation_info));
		// look up libwebsockets.h for details.
		Info.port = Port;
		// we listen on all available interfaces.
		Info.iface = nullptr;
		Info.protocols = WebSocketProtocols;
		// no extensions
		Info.extensions = nullptr;
		Info.gid = -1;
		Info.uid = -1;
		Info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
		// tack on this object.
		Info.user = this;
		// affect by lws>=3.0
		Info.options |= LWS_SERVER_OPTION_DISABLE_IPV6;
		
		WebSocketContext = lws_create_context(&Info);
		if (WebSocketContext == nullptr)
		{
			delete[] WebSocketProtocols;
			WebSocketProtocols = nullptr;
		}
		else
		{
			IsAlive = true;
		}
	}

	void Uninstall()
	{
		if (WebSocketContext)
		{
			lws_context_destroy(WebSocketContext);
			WebSocketContext = nullptr;
		}
		delete[] WebSocketProtocols;
		WebSocketProtocols = nullptr;

		IsAlive = false;
	}
};

IJavascriptInspector* IJavascriptInspector::Create(int32 InPort, Local<Context> InContext)
{
	return new FInspector(reinterpret_cast<v8::Platform*>(IV8::Get().GetV8Platform()), InPort, InContext);
}
#else
IJavascriptInspector* IJavascriptInspector::Create(int32 InPort, Local<Context> InContext)
{
	return nullptr;
}
#endif

PRAGMA_ENABLE_SHADOW_VARIABLE_WARNINGS
