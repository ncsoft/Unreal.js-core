#pragma once

#include "Engine/BlueprintGeneratedClass.h"
#include "UObject/Class.h"
#include "Runtime/Launch/Resources/Version.h"
#include "JavascriptGeneratedClass.generated.h"

struct FJavascriptContext;

UCLASS()
class V8_API UJavascriptGeneratedClass : public UBlueprintGeneratedClass
{
	GENERATED_BODY()

public:		
	TWeakPtr<FJavascriptContext> JavascriptContext;	

	virtual void InitPropertiesFromCustomList(uint8* DataPtr, const uint8* DefaultDataPtr) override;

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
	virtual void PostInitInstance(UObject* InObj, FObjectInstancingGraph* InstanceGraph) override;
#else
	virtual void PostInitInstance(UObject* InObj) override;
#endif
};
