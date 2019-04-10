// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#if WITH_JSWEBSOCKET

#include "JSWebSocketServer.h"
#include "JSWebSocket.h"

#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#endif

#ifndef THIRD_PARTY_INCLUDES_START
#	define THIRD_PARTY_INCLUDES_START
#	define THIRD_PARTY_INCLUDES_END
#endif

// Work around a conflict between a UI namespace defined by engine code and a typedef in OpenSSL
#define UI UI_ST
THIRD_PARTY_INCLUDES_START
#include "libwebsockets.h"
THIRD_PARTY_INCLUDES_END
#undef UI

#if PLATFORM_WINDOWS
#include "Windows/HideWindowsPlatformTypes.h"
#endif

// a object of this type is associated by libwebsocket to every connected session. 
struct PerSessionDataServer
{
	FJavascriptWebSocket *Socket; // each session is actually a socket to a client
};


#if !UE_BUILD_SHIPPING
	void lws_debugLog_JS(int level, const char *line)
	{ 
		UE_LOG(LogWebsocket, Log, TEXT("websocket server: %s"), ANSI_TO_TCHAR(line));
	}
#endif 

bool FJavascriptWebSocketServer::Init(uint32 Port, FJavascriptWebSocketClientConnectedCallBack CallBack)
{
	// setup log level.
#if !UE_BUILD_SHIPPING
	lws_set_log_level(LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_DEBUG | LLL_INFO, lws_debugLog_JS);
#endif 

	Protocols = new lws_protocols[3];
	FMemory::Memzero(Protocols, sizeof(lws_protocols) * 3);

	Protocols[0].name = "binary";
	Protocols[0].callback = [](lws *Wsi, lws_callback_reasons Reason, void *User, void *In, size_t Len) {
		auto context = lws_get_context(Wsi);
		return reinterpret_cast<FJavascriptWebSocketServer*>(lws_context_user(context))->unreal_networking_server(Wsi, Reason, User, In, Len);
	};
	Protocols[0].per_session_data_size = sizeof(PerSessionDataServer);
	Protocols[0].rx_buffer_size = 10 * 1024 * 1024;

	Protocols[1].name = nullptr;
	Protocols[1].callback = nullptr;
	Protocols[1].per_session_data_size = 0;

	struct lws_context_creation_info Info;
	memset(&Info, 0, sizeof(lws_context_creation_info));
	// look up libwebsockets.h for details. 
	Info.port = Port;
	ServerPort = Port;
	// we listen on all available interfaces. 
	Info.iface = NULL;
	Info.protocols = &Protocols[0];
	// no extensions
	Info.extensions = NULL;
	Info.gid = -1;
	Info.uid = -1;
	Info.options = 0;
	// tack on this object. 
	Info.user = this;
	Info.port = Port; 
	Context = lws_create_context(&Info);

	if (Context == NULL) 
	{
		ServerPort = 0;
		delete Protocols;
		Protocols = NULL;
		IsAlive = false;
		return false; // couldn't create a server.
	}
	ConnectedCallBack = CallBack; 	
	IsAlive = true;

	return true; 
}

bool FJavascriptWebSocketServer::Tick()
{
	if (IsAlive)
	{
		lws_service(Context, 0);
		lws_callback_on_writable_all_protocol(Context, &Protocols[0]);
	}
	return true;
}

FJavascriptWebSocketServer::FJavascriptWebSocketServer()
{}

FJavascriptWebSocketServer::~FJavascriptWebSocketServer()
{
	if (Context)
	{
		lws_context_destroy(Context);
		Context = NULL;
	}

	 delete Protocols; 
	 Protocols = NULL; 

	 IsAlive = false;
}

FString FJavascriptWebSocketServer::Info()
{
	return FString(ANSI_TO_TCHAR(lws_canonical_hostname(Context)));
}

// callback. 
int FJavascriptWebSocketServer::unreal_networking_server(lws *InWsi, lws_callback_reasons Reason, void* User, void *In, size_t Len) 
{
	struct lws_context *LwsContext = lws_get_context(InWsi);
	PerSessionDataServer* BufferInfo = (PerSessionDataServer*)User;	
	FJavascriptWebSocketServer* Server = (FJavascriptWebSocketServer*)lws_context_user(LwsContext);
	if (!Server->IsAlive)
	{
		return 0;
	}

	switch (Reason)
	{
		case LWS_CALLBACK_ESTABLISHED: 
			{
				BufferInfo->Socket = new FJavascriptWebSocket(LwsContext, InWsi);
				ConnectedCallBack.ExecuteIfBound(BufferInfo->Socket);
				lws_set_timeout(InWsi, NO_PENDING_TIMEOUT, 0);
			}
			break;

		case LWS_CALLBACK_RECEIVE:
			{
				BufferInfo->Socket->OnRawRecieve(In, Len);
				lws_set_timeout(InWsi, NO_PENDING_TIMEOUT, 0);
			}
			break; 

		case LWS_CALLBACK_SERVER_WRITEABLE: 
			if (BufferInfo->Socket->Context == LwsContext) // UE-68340 -- bandaid until this file is removed in favor of using LwsWebSocketsManager.cpp & LwsWebSocket.cpp
			{
				BufferInfo->Socket->OnRawWebSocketWritable(InWsi);
			}
			lws_set_timeout(InWsi, NO_PENDING_TIMEOUT, 0);
			break;
		case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
			{
				BufferInfo->Socket->ErrorCallBack.ExecuteIfBound();
			}
			break;
		case LWS_CALLBACK_WSI_DESTROY:
		case LWS_CALLBACK_PROTOCOL_DESTROY:
		case LWS_CALLBACK_CLOSED:
		case LWS_CALLBACK_CLOSED_HTTP:
			break;
	}

	return 0; 
}

#endif