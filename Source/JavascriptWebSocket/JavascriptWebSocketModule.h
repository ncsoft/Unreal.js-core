#pragma once

#include "Engine/Engine.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Modules/ModuleManager.h"

// Interfaces
#include "IJavascriptWebSocketModule.h"

class FJavascriptWebSocket;
class FJavascriptWebSocketServer;

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

typedef struct lws_context WebSocketInternalContext;
typedef struct lws WebSocketInternal;
typedef struct lws_protocols WebSocketInternalProtocol;

DECLARE_DELEGATE_TwoParams(FJavascriptWebSocketPacketRecievedCallBack, void* /*Data*/, int32 /*Data Size*/);
DECLARE_DELEGATE_OneParam(FJavascriptWebSocketClientConnectedCallBack, FJavascriptWebSocket* /*Socket*/);
DECLARE_DELEGATE(FJavascriptWebSocketInfoCallBack);

DECLARE_LOG_CATEGORY_EXTERN(LogWebsocket, Warning, All);
