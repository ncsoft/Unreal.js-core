#pragma once

#include "JavascriptEdGraphNode.generated.h"

class FJSEdGraphSchemaAction_NewNode : public FEdGraphSchemaAction_NewNode
{
public:
	FJSEdGraphSchemaAction_NewNode(UObject* InData, const FText& InKey, const FText& InCategory);

	static FName StaticGetTypeId();
	virtual FName GetTypeId() const;

	void Update();

	UObject* Data;
	FText Key;
};

UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptEdGraphNode : public UEdGraphNode
{
	GENERATED_UCLASS_BODY()

	/** instance class */
	UPROPERTY()
	struct FGraphNodeClassData ClassData;

	UPROPERTY()
	UObject* NodeInstance;

	UPROPERTY()
	TArray<UJavascriptEdGraphNode*> SubNodes;

};