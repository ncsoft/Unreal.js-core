// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#include "JavascriptWebSocketModule.h"

#if WITH_JSWEBSOCKET

#include "JSWebSocket.h"

uint8 PREPADDING_JS[LWS_PRE];

static void lws_debugLogS_JS(int level, const char *line)
{
	UE_LOG(LogWebsocket, Log, TEXT("client: %s"), ANSI_TO_TCHAR(line));
}

FJavascriptWebSocket::FJavascriptWebSocket(
		const FInternetAddr& ServerAddress
)
:IsServerSide(false)
{

#if !UE_BUILD_SHIPPING
	lws_set_log_level(LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_DEBUG | LLL_INFO, lws_debugLogS_JS);
#endif 

	Protocols = new lws_protocols[3];
	FMemory::Memzero(Protocols, sizeof(lws_protocols) * 3);

	Protocols[0].name = "binary";
	Protocols[0].callback = [](lws *Wsi, lws_callback_reasons Reason, void *User, void *In, size_t Len) {
		auto context = lws_get_context(Wsi);
		return reinterpret_cast<FJavascriptWebSocket*>(lws_context_user(context))->unreal_networking_client(Wsi, Reason, User, In, Len);
	};
	Protocols[0].per_session_data_size = 0;
	Protocols[0].rx_buffer_size = 10 * 1024 * 1024;

	Protocols[1].name = nullptr;
	Protocols[1].callback = nullptr;
	Protocols[1].per_session_data_size = 0;

	struct lws_context_creation_info Info;
	memset(&Info, 0, sizeof Info);

	Info.port = CONTEXT_PORT_NO_LISTEN;
	Info.protocols = &Protocols[0];
	Info.gid = -1;
	Info.uid = -1;
	Info.user = this;

	Context = lws_create_context(&Info);

	check(Context); 
	
	Wsi = lws_client_connect_extended
							(Context, 
							TCHAR_TO_ANSI(*ServerAddress.ToString(false)), 
							ServerAddress.GetPort(), 
							false, "/", TCHAR_TO_ANSI(*ServerAddress.ToString(false)), TCHAR_TO_ANSI(*ServerAddress.ToString(false)), Protocols[1].name, -1,this);

	check(Wsi);

}

FJavascriptWebSocket::FJavascriptWebSocket(WebSocketInternalContext* InContext, WebSocketInternal* InWsi)
	: Context(InContext)
	, Wsi(InWsi)
	, IsServerSide(true)
	, Protocols(nullptr)
{
}


bool FJavascriptWebSocket::Send(uint8* Data, uint32 Size)
{
	TArray<uint8> Buffer;
	// insert size. 

	Buffer.Append((uint8*)&PREPADDING_JS, sizeof(PREPADDING_JS));
	Buffer.Append((uint8*)Data, Size);

	OutgoingBuffer.Add(Buffer);

	return true;
}

void FJavascriptWebSocket::SetRecieveCallBack(FJavascriptWebSocketPacketRecievedCallBack CallBack)
{
	RecievedCallBack = CallBack; 
}

FString FJavascriptWebSocket::RemoteEndPoint()
{
	ANSICHAR Peer_Name[128];
	ANSICHAR Peer_Ip[128];
	lws_get_peer_addresses(Wsi, lws_get_socket_fd(Wsi), Peer_Name, sizeof Peer_Name, Peer_Ip, sizeof Peer_Ip);
	return FString(Peer_Name);
}

FString FJavascriptWebSocket::LocalEndPoint()
{
	return FString(ANSI_TO_TCHAR(lws_canonical_hostname(Context)));
}

void FJavascriptWebSocket::Tick()
{
	HandlePacket();
}

void FJavascriptWebSocket::HandlePacket()
{
	lws_service(Context, 0);
	if (!IsServerSide)
		lws_callback_on_writable_all_protocol(Context, &Protocols[0]);
}

void FJavascriptWebSocket::Flush()
{
	auto PendingMesssages = OutgoingBuffer.Num();
	while (OutgoingBuffer.Num() > 0 && !IsServerSide)
	{
		if (Protocols)
			lws_callback_on_writable_all_protocol(Context, &Protocols[0]);
		else
			lws_callback_on_writable(Wsi);
	};
}

void FJavascriptWebSocket::SetConnectedCallBack(FJavascriptWebSocketInfoCallBack CallBack)
{
	ConnectedCallBack = CallBack; 
}

void FJavascriptWebSocket::SetErrorCallBack(FJavascriptWebSocketInfoCallBack CallBack)
{
	ErrorCallBack = CallBack; 
}

void FJavascriptWebSocket::OnRawRecieve(void* Data, uint32 Size)
{
	RecievedCallBack.ExecuteIfBound(Data, Size);
}

void FJavascriptWebSocket::OnRawWebSocketWritable(WebSocketInternal* wsi)
{
	if (OutgoingBuffer.Num() == 0)
		return;

	TArray <uint8>& Packet = OutgoingBuffer[0];

	uint32 TotalDataSize = Packet.Num() - LWS_PRE - 0;
	uint32 DataToSend = TotalDataSize;
	while (DataToSend)
	{
		int Sent = lws_write(Wsi, Packet.GetData() + LWS_PRE + (DataToSend-TotalDataSize), DataToSend, (lws_write_protocol)LWS_WRITE_BINARY);
		if (Sent < 0)
		{
			ErrorCallBack.ExecuteIfBound();
			return;
		}
		if ((uint32)Sent < DataToSend)
		{
			UE_LOG(LogWebsocket, Warning, TEXT("Could not write all '%d' bytes to socket"), DataToSend);
		}
		DataToSend-=Sent;
	}

	check(Wsi == wsi);

	// this is very inefficient we need a constant size circular buffer to efficiently not do unnecessary allocations/deallocations. 
	OutgoingBuffer.RemoveAt(0);
}

FJavascriptWebSocket::~FJavascriptWebSocket()
{
	RecievedCallBack.Unbind();
	Flush();

	if ( !IsServerSide)
	{
		lws_context_destroy(Context);
		Context = NULL;
		delete Protocols;
		Protocols = NULL;
	}
}

int FJavascriptWebSocket::unreal_networking_client(lws *InWsi, lws_callback_reasons Reason, void* User, void *In, size_t Len)
{
	switch (Reason)
	{
	case LWS_CALLBACK_CLIENT_ESTABLISHED:
		{
			check(Wsi == InWsi);
			ConnectedCallBack.ExecuteIfBound();
			lws_set_timeout(Wsi, NO_PENDING_TIMEOUT, 0);			
		}
		break;
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		{
			ErrorCallBack.ExecuteIfBound();
			return -1;
		}
		break;
	case LWS_CALLBACK_CLIENT_RECEIVE:
		{
			check(Wsi == InWsi);
			// push it on the socket. 
			OnRawRecieve(In, (uint32)Len); 			
			lws_set_timeout(Wsi, NO_PENDING_TIMEOUT, 0);
			break;
		}
	case LWS_CALLBACK_CLIENT_WRITEABLE:
		{
			check(Wsi == InWsi);
			OnRawWebSocketWritable(Wsi); 
			lws_callback_on_writable(Wsi);
			lws_set_timeout(Wsi, NO_PENDING_TIMEOUT, 0);
			break; 
		}
	case LWS_CALLBACK_CLOSED:
		{
			ErrorCallBack.ExecuteIfBound();
			return -1;
		}
	}

	return 0; 
}
#endif