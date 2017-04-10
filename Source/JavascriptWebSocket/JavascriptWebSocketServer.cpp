// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#include "JavascriptWebSocketServer.h"

#if WITH_JSWEBSOCKET
#include "JSWebSocket.h"
#include "JSWebSocketServer.h"
#include "JavascriptWebSocket.h"
#endif

UJavascriptWebSocketServer* UJavascriptWebSocketServer::Create(int32 Port)
{	
#if WITH_JSWEBSOCKET
	auto instance = NewObject<UJavascriptWebSocketServer>();
	auto server = instance->WebSocketServer = MakeShareable<FJavascriptWebSocketServer>(new FJavascriptWebSocketServer);
	FJavascriptWebSocketClientConnectedCallBack callback;
	callback.BindUObject(instance, &UJavascriptWebSocketServer::OnConnectedCallback);
	if (!server->Init(Port, callback))
	{
		return nullptr;
	}
	return instance;
#else
	return nullptr;
#endif
}

FString UJavascriptWebSocketServer::Info()
{
#if WITH_JSWEBSOCKET
	if (!WebSocketServer.IsValid()) return TEXT("Invalid");

	return WebSocketServer->Info();
#else
	return TEXT("Invalid");
#endif
}

#if WITH_JSWEBSOCKET
void UJavascriptWebSocketServer::OnConnectedCallback(FJavascriptWebSocket* WebSocket)
{
	auto instance = UJavascriptWebSocket::CreateFrom(WebSocket, this);
	Connections.Add(instance);
	OnConnected.Broadcast(instance);
}

void UJavascriptWebSocketServer::OnConnectionLost(UJavascriptWebSocket* Connection)
{
	Connections.Remove(Connection);
}
#endif

void UJavascriptWebSocketServer::Tick()
{
#if WITH_JSWEBSOCKET
	if (!WebSocketServer.IsValid()) return;

	WebSocketServer->Tick();

	for (auto Connection : Connections)
	{
		Connection->Tick();
	}	
#endif
}

void UJavascriptWebSocketServer::Dispose()
{
#if WITH_JSWEBSOCKET
	WebSocketServer.Reset();
#endif
}
