#pragma once

#include "JavascriptGraphSchema.generated.h"

UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptGraphSchema : public UEdGraphSchema
{
	GENERATED_UCLASS_BODY()

	virtual void GetGraphNodeContextActions(FGraphContextMenuBuilder& ContextMenuBuilder, int32 SubNodeFlags) const;

};