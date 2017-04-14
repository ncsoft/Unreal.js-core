#pragma once

#include "Engine/BlueprintGeneratedClass.h"
#include "JavascriptGeneratedClass.generated.h"

struct FJavascriptContext;

UCLASS()
class V8_API UJavascriptGeneratedClass : public UBlueprintGeneratedClass
{
	GENERATED_BODY()

public:		
	TWeakPtr<FJavascriptContext> JavascriptContext;	
};
