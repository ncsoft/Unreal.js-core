#pragma once

#include "UObject/Class.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Stack.h"
#include "UObject/ScriptMacros.h"
#include "JavascriptGeneratedFunction.generated.h"

struct FJavascriptContext;

struct FNewFrame : public FFrame
{
	FNewFrame(UObject* InObject, UFunction* InNode, void* InLocals, FFrame* InPreviousFrame,
		UField* InPropertyChainForCompiledIn)
		: FFrame(InObject, InNode, InLocals, InPreviousFrame, InPropertyChainForCompiledIn)
	{
	}
};

UCLASS()
class V8_API UJavascriptGeneratedFunction : public UFunction
{
	GENERATED_BODY()

public:		
	TWeakPtr<FJavascriptContext> JavascriptContext;	

	DECLARE_FUNCTION(Thunk);
};
