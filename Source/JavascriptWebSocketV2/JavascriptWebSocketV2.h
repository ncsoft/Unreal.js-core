#pragma once

#include "IWebSocket.h"
#include "JavascriptWebSocketV2.generated.h"

UCLASS()
class UJavascriptWebSocketV2 : public UObject {
  GENERATED_BODY()

public:
  UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
  static UJavascriptWebSocketV2 *Create(const FString &endpoint);

  UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
  void Connect();

private:
  TSharedPtr<IWebSocket> WebSocket;
};