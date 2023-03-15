#pragma once

#include "Commandlets/Commandlet.h"
#include "JavascriptEditorTick.h"
#include "JavascriptContext.h"
#include "JavascriptOnEditorCommandlet.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogJavascriptOnEditor, Log, All);

UCLASS()
class UJavascriptOnEditorCommandlet : public UCommandlet
{
	GENERATED_UCLASS_BODY()

	UFUNCTION()
	UEditorEngine* GetEngine();

	/** Parsed commandline tokens */
	UPROPERTY()
	TArray<FString> CmdLineTokens;

	/** Parsed commandline switches */
	UPROPERTY()
	TArray<FString> CmdLineSwitches;

	UJavascriptEditorTick* Tick{ nullptr };
	UJavascriptContext* JavascriptContext{ nullptr };

	bool bRegistered{ false };
	FDelegateHandle OnPropertyChangedDelegateHandle;

	FTimerHandle TimerHandle_GarbageCollect;

	void RegisterSettings();
	void UnregisterSettings();

	// Called when a property on the specified object is modified
	void OnPropertyChanged(UObject* ObjectBeingModified, FPropertyChangedEvent& PropertyChangedEvent);

	void Bootstrap();
	void Terminate();

	virtual int32 Main(const FString& Params) override;
};
