#pragma once

#include "JavascriptIsolate.h"
#include "JavascriptObject.generated.h"

UCLASS()
class V8_API UJavascriptObject : public UObject
{
	GENERATED_BODY()

public:		
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GenericGraphNode")
	FJavascriptRef Ref;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GenericGraphNode")
	FJavascriptFunction Func;
};
