#pragma once

#include "Widget.h"
#include "JavascriptGraphEditor.generated.h"

USTRUCT()
struct FJavascriptGraphAction
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FName Name;
	
	UPROPERTY()
	FName Category;
	
	UPROPERTY()
	UObject* Resource;
};

UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptGraphEditor : public UWidget
{
	GENERATED_UCLASS_BODY()

#if WITH_EDITOR
public:
	UPROPERTY()
	FString TitleName;

	UPROPERTY()
	UEdGraph* Graph;

	UPROPERTY()
	FJavascriptUICommandList GraphEditorCommands;

	UFUNCTION()
	void AddActionContext(FJavascriptGraphAction Action);

protected:

	TArray<FJavascriptGraphAction> Actions;

	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface

	bool InEditingMode(bool bGraphIsEditable) const;
	FActionMenuContent OnCreateGraphActionMenu(UEdGraph* InGraph, const FVector2D& InNodePosition, const TArray<UEdGraphPin*>& InDraggedPins, bool bAutoExpand, SGraphEditor::FActionMenuClosed InOnMenuClosed);

	void CollectAllActions(FGraphActionListBuilderBase& OutAllActions);
	TSharedRef<SWidget> OnCreateWidgetForAction(struct FCreateWidgetForActionData* const InCreateData);

	void OnActionSelected(const TArray< TSharedPtr<FEdGraphSchemaAction> >& SelectedActions, ESelectInfo::Type InSelectionType);
	void OnNodeDoubleClicked(class UEdGraphNode* Node);
	void OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection);

	TSharedPtr<SGraphActionMenu> GraphActionMenu;
	FVector2D NewNodePosition;

#endif
};