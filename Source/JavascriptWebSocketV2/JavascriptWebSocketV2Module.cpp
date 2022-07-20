#include "IJavascriptWebSocketV2Module.h"

class FJavascriptWebSocketV2 : public IJavascriptWebSocketV2Module {
  // Begin IModuleInterface
  virtual void StartupModule() override;
  virtual void ShutdownModule() override;
  // End IModuleInterface
};

void FJavascriptWebSocketV2::StartupModule() {}

void FJavascriptWebSocketV2::ShutdownModule() {}

IMPLEMENT_MODULE(FJavascriptWebSocketV2, JavascriptWebSocketV2);