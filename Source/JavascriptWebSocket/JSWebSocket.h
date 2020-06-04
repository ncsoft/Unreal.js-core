// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
//
// libwebsocket client wrapper.
//
#pragma  once
#if WITH_JSWEBSOCKET
#include "JavascriptWebSocketModule.h"
#endif

DECLARE_DELEGATE(FJavascriptWebSocketInfoCallBack);

class FJavascriptWebSocket
{

public:

	// Initialize as client side socket.
	FJavascriptWebSocket(const FInternetAddr& ServerAddress);

	// Initialize as server side socket.
	FJavascriptWebSocket(WebSocketInternalContext* InContext, WebSocketInternal* Wsi);

	// clean up.
	~FJavascriptWebSocket();

	/************************************************************************/
	/* Set various callbacks for Socket Events                              */
	/************************************************************************/
	void SetConnectedCallBack(FJavascriptWebSocketInfoCallBack CallBack);
	void SetErrorCallBack(FJavascriptWebSocketInfoCallBack CallBack);
	void SetRecieveCallBack(FJavascriptWebSocketPacketRecievedCallBack CallBack);

	/** Send raw data to remote end point. */
	bool Send(uint8* Data, uint32 Size);

	/** service libwebsocket.			   */
	void Tick();
	/** service libwebsocket until outgoing buffer is empty */
	void Flush();

	/** Helper functions to describe end points. */
	TArray<uint8> GetRawRemoteAddr(int32& OutPort);
	FString RemoteEndPoint(bool bAppendPort);
	FString LocalEndPoint(bool bAppendPort);
	struct sockaddr_in* GetRemoteAddr() { return &RemoteAddr; }

	// this was made public because of cross-platform build issues
public:

	void HandlePacket();
	void OnRawRecieve(void* Data, uint32 Size);
	void OnRawWebSocketWritable(WebSocketInternal* wsi);

	/************************************************************************/
	/*	Various Socket callbacks											*/
	/************************************************************************/
	FJavascriptWebSocketPacketRecievedCallBack  RecievedCallBack;
	FJavascriptWebSocketInfoCallBack ConnectedCallBack;
	FJavascriptWebSocketInfoCallBack ErrorCallBack;

	/**  Recv and Send Buffers, serviced during the Tick */
	TArray<uint8> RecievedBuffer;
	TArray<TArray<uint8>> OutgoingBuffer;

#if WITH_JSWEBSOCKET
	/** libwebsocket internal context*/
	WebSocketInternalContext* Context;

	/** libwebsocket web socket */
	WebSocketInternal* Wsi;

	/** libwebsocket Protocols that can be serviced by this implemenation*/
	WebSocketInternalProtocol* Protocols;
#else // ! USE_LIBWEBSOCKET -- HTML5 uses BSD network API
	int SockFd;
#endif
	struct sockaddr_in RemoteAddr;

	/** Server side socket or client side*/
	bool IsServerSide;

	friend class FJavascriptWebSocketServer;
};


