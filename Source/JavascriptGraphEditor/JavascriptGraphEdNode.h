#pragma once

#include "JavascriptGraphEdGraph.h"
#include "JavascriptGraphEditorLibrary.h"
#include "SGraphPin.h"
#include "SJavascriptGraphEdNode.h"
#include "JavascriptIsolate.h"
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

	UPROPERTY()
	bool IsTitleOnly;

	UPROPERTY()
	bool IsCustomNode;

	UPROPERTY()
	FJavascriptFunction WidgetFinalized;

public:
	virtual void AllocateDefaultPins() override;
	virtual void NodeConnectionListChanged() override;
	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;

	UJavascriptGraphEdGraph* GetGenericGraphEdGraph();
	TSharedPtr<SJavascriptGraphEdNode> GetNodeSlateWidget() const;

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
		const FText& PinDisplayName,
		const FJavascriptPinParams& InPinParams
		);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	bool RemovePinByName(FName PinName);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	bool RemovePin(FJavascriptEdGraphPin Pin);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	void UpdateSlate();

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	FVector2D GetDesiredSize();

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	int32 GetNumOfPins(EEdGraphPinDirection Direction = EGPD_MAX) const;

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	void SetEnable(bool bEnable);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	void SetVisible(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	bool GetVisible();

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	void SetTitleSelectionMode(float InTitleHeight);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	void ResetTitleSelectionMode();

public:
	UPROPERTY()
	bool Bidirectional;

	UPROPERTY()
	int32 PriorityOrder;

	TWeakPtr<SJavascriptGraphEdNode> SlateGraphNode;

	bool bTitleSelectionOnly;
	float TitleHeight;
};
