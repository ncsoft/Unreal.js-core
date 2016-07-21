#pragma once

#include "JavascriptLibrary.h"
#include "JavascriptOutputDevice.generated.h"


UCLASS()
class V8_API UJavascriptOutputDevice : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	void Kill();

	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
	void OnMessage(const FString& Message, ELogVerbosity_JS Verbosity, FName Category);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void Log(FName Category, ELogVerbosity_JS Verbosity, const FString& Filename, int32 LineNumber, const FString& Message);

	TSharedPtr<FOutputDevice> OutputDevice;
};