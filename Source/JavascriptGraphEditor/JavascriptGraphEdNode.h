#pragma once

#include "JavascriptGraphEdGraph.h"
#include "JavascriptGraphEditorLibrary.h"
#include "SGraphPin.h"
#include "JavascriptGraphEdNode.generated.h"

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
		const FString& PinCategory,
		const FString& PinSubCategory,
		UObject* PinSubCategoryObject,
		const FString& PinName,
		EPinContainerType PinContainerType /* EPinContainerType::None */,
		bool bIsReference,
		bool bIsConst /*= false*/,
		//int32 Index /*= INDEX_NONE*/
		const FString& PinToolTip
		);

	UFUNCTION(BlueprintCallable)
	void UpdateSlate();

	UFUNCTION(BlueprintCallable)
	FVector2D GetDesiredSize();

public:
	UPROPERTY()
	bool Bidirectional;

	UPROPERTY()
	int32 PriorityOrder;

	SGraphNode* SlateGraphNode;
};
