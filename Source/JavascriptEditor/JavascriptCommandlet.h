#pragma once

#include "Commandlets/Commandlet.h"
#include "JavascriptCommandlet.generated.h"

UCLASS()
class UJavascriptCommandlet : public UCommandlet
{
	GENERATED_UCLASS_BODY()

	/** Parsed commandline tokens */
	TArray<FString> CmdLineTokens;

	/** Parsed commandline switches */
	TArray<FString> CmdLineSwitches;

	virtual int32 Main(const FString& Params) override;
};
