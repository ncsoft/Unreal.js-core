#pragma once

#include "JavascriptCommandExecutor.h"
#include "Modules/ModuleInterface.h"

class FJavascriptConsoleModule : public IModuleInterface, FTickableEditorObject
{
public:
	FJavascriptConsoleModule();

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsAllowedToTick() const override;

private:
	TMap<TSharedPtr<FString>, TSharedPtr<FJavascriptCommandExecutor>> CommandExecutors;
	bool bModuleIsRunning;
};
