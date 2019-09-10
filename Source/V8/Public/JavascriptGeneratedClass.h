#pragma once

#include "Engine/BlueprintGeneratedClass.h"
#include "UObject/Class.h"
#include "JavascriptGeneratedClass.generated.h"

struct FJavascriptContext;

UCLASS()
class V8_API UJavascriptGeneratedClass : public UBlueprintGeneratedClass
{
	GENERATED_BODY()

public:		
	TWeakPtr<FJavascriptContext> JavascriptContext;	

	virtual void InitPropertiesFromCustomList(uint8* DataPtr, const uint8* DefaultDataPtr) override;
	virtual void PostInitInstance(UObject* InObj) override;
};
