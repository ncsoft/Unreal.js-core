#pragma once

#include "UObject/Class.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Stack.h"
#include "UObject/ScriptMacros.h"
#include "JavascriptGeneratedFunction.generated.h"

struct FJavascriptContext;

UCLASS()
class V8_API UJavascriptGeneratedFunction : public UFunction
{
	GENERATED_BODY()

public:		
	TWeakPtr<FJavascriptContext> JavascriptContext;	

	DECLARE_FUNCTION(Thunk);
};
