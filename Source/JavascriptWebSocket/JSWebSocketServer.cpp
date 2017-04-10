// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#if WITH_JSWEBSOCKET

#include "JSWebSocketServer.h"
#include "JSWebSocket.h"

#if PLATFORM_WINDOWS
#include "AllowWindowsPlatformTypes.h"
#endif

#ifndef THIRD_PARTY_INCLUDES_START
#	define THIRD_PARTY_INCLUDES_START
#	define THIRD_PARTY_INCLUDES_END
#endif

THIRD_PARTY_INCLUDES_START

#ifndef LWS_INCLUDED
#include "libwebsockets.h"
#define LWS_INCLUDED
#define LWS_EXTERN extern
#include "private-libwebsockets.h"
#endif

THIRD_PARTY_INCLUDES_END

#if PLATFORM_WINDOWS
#include "HideWindowsPlatformTypes.h"
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
		return false; // couldn't create a server. 
	}

	ConnectedCallBack = CallBack; 	

	return true; 
}

bool FJavascriptWebSocketServer::Tick()
{
	lws_service(Context, 0);
	lws_callback_on_writable_all_protocol(Context, &Protocols[0]);
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
}

FString FJavascriptWebSocketServer::Info()
{
	return FString(ANSI_TO_TCHAR(lws_canonical_hostname(Context)));
}

// callback. 
int FJavascriptWebSocketServer::unreal_networking_server(lws *InWsi, lws_callback_reasons Reason, void* User, void *In, size_t Len) 
{
	PerSessionDataServer* BufferInfo = (PerSessionDataServer*)User;	

	switch (Reason)
	{
		case LWS_CALLBACK_ESTABLISHED: 
			{
				BufferInfo->Socket = new FJavascriptWebSocket(Context, InWsi);
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
			{
				BufferInfo->Socket->OnRawWebSocketWritable(InWsi);
				lws_set_timeout(InWsi, NO_PENDING_TIMEOUT, 0);
			}
			break; 
		case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
			{
				BufferInfo->Socket->ErrorCallBack.ExecuteIfBound();
			}
			break;
	}

	return 0; 
}

#endif