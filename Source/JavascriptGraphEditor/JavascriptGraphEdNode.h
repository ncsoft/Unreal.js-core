#pragma once

#include "JavascriptGraphEdGraph.h"
#include "JavascriptGraphEditorLibrary.h"
#include "SGraphPin.h"
#include "JavascriptGraphEdNode.generated.h"


USTRUCT(BlueprintType)
struct FJavascriptPinParams
{
	GENERATED_USTRUCT_BODY()

	FJavascriptPinParams()
		: ContainerType(EPinContainerType::None)
		, bIsReference(false)
		, bIsConst(false)
		, Index(INDEX_NONE)
	{
	}

	UPROPERTY()
	EPinContainerType ContainerType;
	UPROPERTY()
	bool bIsReference;
	UPROPERTY()
	bool bIsConst;
	UPROPERTY()
	int32 Index;
};

UCLASS(MinimalAPI)
class UJavascriptGraphEdNode : public UEdGraphNode
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FSlateColor BackgroundColor;

	UPROPERTY(VisibleAnywhere, instanced, Category = "JavascriptGraph")
	UObject* GraphNode;
	
public:
	virtual void AllocateDefaultPins() override;
	virtual void NodeConnectionListChanged() override;
	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;

	UJavascriptGraphEdGraph* GetGenericGraphEdGraph();

	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const;
	virtual FText GetDescription() const;

	virtual void ResizeNode(const FVector2D& NewSize) override;

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	FJavascriptEdGraphPin CreatePin(
		EEdGraphPinDirection Dir,
		const FName PinCategory,
		const FName PinSubCategory,
		UObject* PinSubCategoryObject,
		const FName PinName,
		const FString& PinToolTip,
		const FJavascriptPinParams& InPinParams
		);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	void UpdateSlate();

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	FVector2D GetDesiredSize();

public:
	UPROPERTY()
	bool Bidirectional;

	UPROPERTY()
	int32 PriorityOrder;

	SGraphNode* SlateGraphNode;
};
