#include "V8PCH.h"
#include "../../Launch/Resources/Version.h"

#ifndef THIRD_PARTY_INCLUDES_START
#	define THIRD_PARTY_INCLUDES_START
#	define THIRD_PARTY_INCLUDES_END
#endif

PRAGMA_DISABLE_SHADOW_VARIABLE_WARNINGS

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION >= 14
static const int32 CONTEXT_GROUP_ID = 1;

#if PLATFORM_WINDOWS
#include "AllowWindowsPlatformTypes.h"
#endif
#define UI UI_ST
THIRD_PARTY_INCLUDES_START
#include "libwebsockets.h"
THIRD_PARTY_INCLUDES_END
#undef UI
#if PLATFORM_WINDOWS
#include "HideWindowsPlatformTypes.h"
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
#include "Translator.h"
#include "IV8.h"

namespace {
	class AgentImpl : public v8_inspector::V8Inspector::Channel, public TSharedFromThis<AgentImpl>
	{
	public:
		AgentImpl(v8::Isolate* isolate)
		: isolate_(isolate)
		{}

		virtual ~AgentImpl() {}

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
		TArray<TArray<uint8>> OutgoingBuffer;

		ChannelImpl(lws_context *InContext, lws* InWsi, v8::Isolate* isolate, v8::Platform* platform, const std::unique_ptr<v8_inspector::V8Inspector>& v8inspector)
			: AgentImpl(isolate), platform_(platform), WebSocketContext(InContext), Wsi(InWsi)
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
			platform_->CallOnForegroundThread(isolate_, new DispatchOnInspectorBackendTask(AsShared()));
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
			UE_LOG(Javascript, Log, TEXT("Received command line api"));
		}

		void sendMessage(const void* Data, uint32 Size)
		{
			TArray<uint8> Buffer;

			// Reserve space for WS header data
			Buffer.AddDefaulted(LWS_PRE);
			Buffer.Append((uint8*)Data, Size);
			OutgoingBuffer.Add(Buffer);

			OnWritable();
		}

		void OnWritable()
		{
			if (OutgoingBuffer.Num() == 0)
			{
				return;
			}

			TArray <uint8>& Packet = OutgoingBuffer[0];

			uint32 TotalDataSize = Packet.Num() - LWS_PRE;
			uint32 DataToSend = TotalDataSize;
			while (DataToSend)
			{
				int Sent = lws_write(Wsi, Packet.GetData() + LWS_PRE + (DataToSend - TotalDataSize), DataToSend, (lws_write_protocol)LWS_WRITE_TEXT);
				if (Sent < 0)
				{
					UE_LOG(Javascript, Warning, TEXT("Could not write any bytes to socket"));
					return;
				}
				if ((uint32)Sent < DataToSend)
				{
					UE_LOG(Javascript, Warning, TEXT("Could not write all '%d' bytes to socket"), DataToSend);
				}
				DataToSend -= Sent;
			}

			// this is very inefficient we need a constant size circular buffer to efficiently not do unnecessary allocations/deallocations.
			OutgoingBuffer.RemoveAt(0);
		}

		void flushProtocolNotifications()
		{}
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
		return FString::Printf(TEXT("chrome-devtools://devtools/bundled/inspector.html?experiments=true&v8only=true&ws=127.0.0.1:%d"), Port);
	}

	FInspector(v8::Platform* platform, int32 InPort, Local<Context> InContext)
		: Port(InPort)
	{
		platform_ = platform;
		isolate_ = InContext->GetIsolate();
		context_.Reset(isolate_, InContext);

		FIsolateHelper I(isolate_);

		{
			auto console = InContext->Global()->Get(I.Keyword("console"));
			InContext->Global()->Set(I.Keyword("$console"), console);
		}

		v8inspector = v8_inspector::V8Inspector::create(InContext->GetIsolate(), this);
		const uint8_t CONTEXT_NAME[] = "Unreal.js";
		v8_inspector::StringView context_name(CONTEXT_NAME, sizeof(CONTEXT_NAME) - 1);
		v8inspector->contextCreated(v8_inspector::V8ContextInfo(InContext, CONTEXT_GROUP_ID, context_name));

		Install(InPort);

		{
			Isolate::Scope isolate_scope(isolate_);
			Context::Scope context_scope(InContext);

			TryCatch try_catch;

			auto source = TEXT("'log error warn info void assert'.split(' ').forEach(x => { let o = console[x].bind(console); let y = $console[x].bind($console); console['$'+x] = o; console[x] = function () { y(...arguments); return o(...arguments); }})");
			auto script = v8::Script::Compile(I.String(source));
			auto result = script->Run();
		}

		UE_LOG(Javascript, Log, TEXT("open %s"), *DevToolsFrontEndUrl());

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
		if (GLog != NULL)
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

			Isolate::Scope isolate_scope(isolate_);
			Context::Scope context_scope(context());

			TryCatch try_catch;

			auto console = context()->Global()->Get(I.Keyword("console")).As<v8::Object>();

			auto method =
				Verbosity == ELogVerbosity::Fatal || Verbosity == ELogVerbosity::Error ? I.Keyword("$error") :
				Verbosity == ELogVerbosity::Warning ? I.Keyword("$warn") :
				Verbosity == ELogVerbosity::Display ? I.Keyword("info") :
				I.Keyword("$log");
			auto function = console->Get(method).As<v8::Function>();

			if (Verbosity == ELogVerbosity::Display)
			{
				Handle<Value> argv[2];
				argv[0] = I.String(FString::Printf(TEXT("%%c%s: %s"), *Category.ToString(), V));
				argv[1] = I.String(TEXT("color:gray"));
				function->Call(console, 2, argv);
			}
			else
			{
				Handle<Value> argv[1];
				argv[0] = I.String(FString::Printf(TEXT("%s: %s"), *Category.ToString(), V));
				function->Call(console, 1, argv);
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

			while (v8::platform::PumpMessageLoop(platform_, isolate_))
			{}
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
	{}

	v8::Local<v8::Context> ensureDefaultContextInGroup(int) override
	{
		return context();
	}

	virtual void Tick(float DeltaTime) override
	{
		if (IsAlive)
		{
			lws_service(WebSocketContext, 0);
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

	int inspector_server( lws *Wsi, lws_callback_reasons Reason,	void *User, void *In, size_t Len )
	{
		PerSessionDataServer* BufferInfo = (PerSessionDataServer*)User;
		if (!IsAlive)
		{
			return 0;
		}

		switch (Reason)
		{
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
		case LWS_CALLBACK_HTTP:
		{
			auto request = (char*)In;

			UE_LOG(Javascript, Log, TEXT("requested URI:%s"), UTF8_TO_TCHAR(request));

			FString res[][2] = {
				{
					TEXT("description"), TEXT("unreal.js instance")
				},
				{
					TEXT("devtoolsFrontendUrl"), DevToolsFrontEndUrl()
				},
				{
					TEXT("type"), TEXT("node")
				},
				{
					TEXT("id"), TEXT("0")
				},
				{
					TEXT("title"), TEXT("unreal.js")
				},
				{
					TEXT("webSocketDebuggerUrl"), WebSocketDebuggerUrl()
				}
			};

			TArray<FString> res2;
			for (auto pair : res)
			{
				res2.Add(FString::Printf(TEXT("\"%s\" : \"%s\""), *pair[0], *pair[1]));
			}

			{
				auto response = FString(TEXT("HTTP/1.0 200 OK\r\nContent-Type: application/json; charset=UTF-8\r\nCache-Control: no-cache\r\n\r\n"));
				FTCHARToUTF8 utf8(*response);
				lws_write_http(Wsi, utf8.Get(), utf8.Length());
			}

			{
				auto response = FString::Printf(TEXT("[{%s}]"), *FString::Join(res2, TEXT(",")));
				FTCHARToUTF8 utf8(*response);
				lws_write_http(Wsi, utf8.Get(), utf8.Length());
			}
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
			//BufferInfo->Socket->ErrorCallBack.ExecuteIfBound();
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
		UE_LOG(Javascript, Log, TEXT("websocket server: %s"), ANSI_TO_TCHAR(line));
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
		WebSocketProtocols[0].rx_buffer_size = 1024 * 1024 * 4;

		struct lws_context_creation_info Info;
		memset(&Info, 0, sizeof(lws_context_creation_info));
		// look up libwebsockets.h for details.
		Info.port = Port;
		// we listen on all available interfaces.
		Info.iface = NULL;
		Info.protocols = WebSocketProtocols;
		// no extensions
		Info.extensions = NULL;
		Info.gid = -1;
		Info.uid = -1;
		Info.options = 0;
		// tack on this object.
		Info.user = this;
		WebSocketContext = lws_create_context(&Info);

		IsAlive = true;
	}

	void Uninstall()
	{
		lws_context_destroy(WebSocketContext);
		WebSocketContext = NULL;
		delete[] WebSocketProtocols;

		IsAlive = false;
	}
};

IJavascriptInspector* IJavascriptInspector::Create(int32 InPort, Local<Context> InContext)
{
	return new FInspector(reinterpret_cast<v8::Platform*>(IV8::Get().GetV8Platform()), InPort,InContext);
}
#else
IJavascriptInspector* IJavascriptInspector::Create(int32 InPort, Local<Context> InContext)
{
	return nullptr;
}
#endif

PRAGMA_ENABLE_SHADOW_VARIABLE_WARNINGS
