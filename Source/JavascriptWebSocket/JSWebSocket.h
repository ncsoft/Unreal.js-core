// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
//
// libwebsocket client wrapper. 
//
#pragma  once

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
	FString RemoteEndPoint();
	FString LocalEndPoint(); 

private:

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

	/** libwebsocket internal context*/
	WebSocketInternalContext* Context;

	/** libwebsocket web socket */
	WebSocketInternal* Wsi;

	/** libwebsocket Protocols that can be serviced by this implemenation*/
	WebSocketInternalProtocol* Protocols;

	/** Server side socket or client side*/
	bool IsServerSide; 

	/* libwebsocket service functions */ 	
	int unreal_networking_client(lws *Wsi, lws_callback_reasons Reason, void* User, void *In, size_t Len);

	friend class FJavascriptWebSocketServer;
	int SockFd;
};


