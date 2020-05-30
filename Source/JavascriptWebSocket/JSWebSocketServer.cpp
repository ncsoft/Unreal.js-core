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


// real networking handler.
static int unreal_networking_server(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len);

#if !UE_BUILD_SHIPPING
	inline void lws_debugLog_JS(int level, const char *line)
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
	Protocols[0].callback = unreal_networking_server;
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
	// tack on this object.
	Info.user = this;

	Info.options |= LWS_SERVER_OPTION_DISABLE_IPV6;
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
static int unreal_networking_server
(
	struct lws* Wsi,
	enum lws_callback_reasons Reason,
	void* User,
	void* In,
	size_t Len
)
{
	struct lws_context* Context = lws_get_context(Wsi);
	PerSessionDataServer* BufferInfo = (PerSessionDataServer*)User;
	FJavascriptWebSocketServer* Server = (FJavascriptWebSocketServer*)lws_context_user(Context);

	switch (Reason)
	{
	case LWS_CALLBACK_ESTABLISHED:
	{
		BufferInfo->Socket = new FJavascriptWebSocket(Context, Wsi);
		Server->ConnectedCallBack.ExecuteIfBound(BufferInfo->Socket);
		lws_set_timeout(Wsi, NO_PENDING_TIMEOUT, 0);
	}
	break;

	case LWS_CALLBACK_RECEIVE:
		if (BufferInfo->Socket->Context == Context) // UE-74107 -- bandaid until this file is removed in favor of using LwsWebSocketsManager.cpp & LwsWebSocket.cpp
		{
			BufferInfo->Socket->OnRawRecieve(In, Len);
		}
		lws_set_timeout(Wsi, NO_PENDING_TIMEOUT, 0);
		break;

	case LWS_CALLBACK_SERVER_WRITEABLE:
		if (BufferInfo->Socket->Context == Context) // UE-68340 -- bandaid until this file is removed in favor of using LwsWebSocketsManager.cpp & LwsWebSocket.cpp
		{
			BufferInfo->Socket->OnRawWebSocketWritable(Wsi);
		}
		lws_set_timeout(Wsi, NO_PENDING_TIMEOUT, 0);
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