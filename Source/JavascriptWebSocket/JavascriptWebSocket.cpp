// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "JavascriptWebSocket.h"
#include "JavascriptWebSocketServer.h"

#if WITH_JSWEBSOCKET
#include "Interfaces/IPv4/IPv4Endpoint.h"
#endif
PRAGMA_DISABLE_SHADOW_VARIABLE_WARNINGS

UJavascriptWebSocket* UJavascriptWebSocket::Connect(const FString& EndpointString)
{
#if WITH_JSWEBSOCKET
	FIPv4Endpoint Endpoint;

	if (!FIPv4Endpoint::Parse(EndpointString, Endpoint))
	{
		return nullptr;
	}

	auto addr = Endpoint.ToInternetAddr();
	return CreateFrom(new FJavascriptWebSocket(*addr), (UObject*)GetTransientPackage());
#else
	return nullptr;
#endif
}

UJavascriptWebSocket* UJavascriptWebSocket::CreateFrom(FJavascriptWebSocket* WebSocket, UObject* Outer)
{
#if WITH_JSWEBSOCKET
	auto instance = NewObject<UJavascriptWebSocket>(Outer);
	instance->WebSocket = MakeShareable<FJavascriptWebSocket>(WebSocket);

	{
		FJavascriptWebSocketPacketRecievedCallBack callback;
		callback.BindUObject(instance, &UJavascriptWebSocket::OnReceivedCallback);
		instance->WebSocket->SetRecieveCallBack(callback);
	}

	{
		FJavascriptWebSocketInfoCallBack callback;
		callback.BindUObject(instance, &UJavascriptWebSocket::OnErrorCallback);
		instance->WebSocket->SetErrorCallBack(callback);
	}

	{
		FJavascriptWebSocketInfoCallBack callback;
		callback.BindUObject(instance, &UJavascriptWebSocket::OnConnectedCallback);
		instance->WebSocket->SetConnectedCallBack(callback);
	}
	return instance;
#else
	return nullptr;
#endif
}

#if WITH_JSWEBSOCKET
void UJavascriptWebSocket::OnReceivedCallback(void* InData, int32 Count)
{
	Buffer = InData;
	Size = Count;
	OnReceived.Broadcast();
	Buffer = InData;
	Size = 0;
}
#endif

int32 UJavascriptWebSocket::GetReceivedBytes()
{
#if WITH_JSWEBSOCKET
	return Size;
#else
	return 0;
#endif
}

void UJavascriptWebSocket::CopyBuffer()
{
#if WITH_JSWEBSOCKET
	if (FArrayBufferAccessor::GetSize() >= Size)
	{
		FMemory::Memcpy(FArrayBufferAccessor::GetData(), Buffer, Size);
	}
#endif
}

#if WITH_JSWEBSOCKET
void UJavascriptWebSocket::OnConnectedCallback()
{
	OnConnected.Broadcast();
}

void UJavascriptWebSocket::OnErrorCallback()
{
	OnError.Broadcast();

	if (auto server = Cast<UJavascriptWebSocketServer>(GetOuter()))
	{
		server->OnConnectionLost(this);
	}
}
#endif

void UJavascriptWebSocket::SendMemory(int32 NumBytes)
{
#if WITH_JSWEBSOCKET
	if (!WebSocket.IsValid()) return;

	auto Buffer = FArrayBufferAccessor::GetData();
	auto Size = FArrayBufferAccessor::GetSize();

	if (NumBytes > Size) return;

	WebSocket->Send((uint8*)Buffer, NumBytes);
#endif
}

FString UJavascriptWebSocket::RemoteEndPoint(bool bAppendPort)
{
#if WITH_JSWEBSOCKET
	if (!WebSocket.IsValid()) return TEXT("Invalid");

	return WebSocket->RemoteEndPoint(bAppendPort);
#else
	return TEXT("Invalid");
#endif
}

FString UJavascriptWebSocket::LocalEndPoint(bool bAppendPort)
{
#if WITH_JSWEBSOCKET
	if (!WebSocket.IsValid()) return TEXT("Invalid");

	return WebSocket->LocalEndPoint(bAppendPort);
#else
	return TEXT("Invalid");
#endif
}

void UJavascriptWebSocket::Flush()
{
#if WITH_JSWEBSOCKET
	if (!WebSocket.IsValid()) return;

	WebSocket->Flush();
#endif
}

void UJavascriptWebSocket::Tick()
{
#if WITH_JSWEBSOCKET
	if (!WebSocket.IsValid()) return;

	WebSocket->Tick();
#endif
}

void UJavascriptWebSocket::Dispose()
{
#if WITH_JSWEBSOCKET
	WebSocket.Reset();
#endif
}

PRAGMA_ENABLE_SHADOW_VARIABLE_WARNINGS