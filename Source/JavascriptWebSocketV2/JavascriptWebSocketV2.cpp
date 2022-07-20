#include "JavascriptWebSocketV2.h"
#include "IWebSocket.h"
#include "JavascriptContext.h"
#include "WebSocketsModule.h"

UJavascriptWebSocketV2 *
UJavascriptWebSocketV2::Create(const FString &endpoint) {
  auto instance = NewObject<UJavascriptWebSocketV2>(GetTransientPackage());
  if (!FModuleManager::Get().IsModuleLoaded("WebSockets")) {
    FModuleManager::Get().LoadModule("WebSockets");
  }
  instance->WebSocket =
      FWebSocketsModule::Get().CreateWebSocket(endpoint, "wss");
  return instance;
}

void UJavascriptWebSocketV2::Connect() {
  UJavascriptWebSocketV2::WebSocket->OnConnected().AddLambda(
      []() { UE_LOG(LogTemp, Log, TEXT("Connected to websocket server.")); });
  UJavascriptWebSocketV2::WebSocket->Connect();
}

// TODO: Refer this to impl callbacks.
// Socket->OnConnected().AddLambda([Socket]() {
//   UE_LOG(LogTemp, Log, TEXT("Connected to websocket server."));
// });

// Socket->OnConnectionError().AddLambda([](const FString &Error) {
//   UE_LOG(LogTemp, Log,
//          TEXT("Failed to connect to websocket server with error:
//              \"%s\"."), *Error);
// });

// Socket->OnMessage().AddLambda([](const FString &Message) {
//   UE_LOG(LogTemp, Log, TEXT("Received message from websocket server:
//   \"%s\"."),
//          *Message);
// });

// Socket->OnClosed().AddLambda([](int32 StatusCode, const FString &Reason,
//                                 bool bWasClean) {
//   UE_LOG(LogTemp, Log,
//          TEXT("Connection to websocket server has been closed with
//               "status code: \"%d\" and reason: \"%s\"."),
//          StatusCode, *Reason);
// });
