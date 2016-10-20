#pragma once

#include "Widget.h"
#include "GraphEditor.h"
#include "Editor/GraphEditor/Public/SGraphActionMenu.h"
#include "JavascriptGraphEditor.generated.h"

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

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface

	bool InEditingMode(bool bGraphIsEditable) const;
	FActionMenuContent OnCreateGraphActionMenu(UEdGraph* InGraph, const FVector2D& InNodePosition, const TArray<UEdGraphPin*>& InDraggedPins, bool bAutoExpand, SGraphEditor::FActionMenuClosed InOnMenuClosed);

	void CollectAllActions(FGraphActionListBuilderBase& OutAllActions);
	void OnActionSelected(const TArray< TSharedPtr<FEdGraphSchemaAction> >& SelectedAction, ESelectInfo::Type InSelectionType);
	void OnNodeDoubleClicked(class UEdGraphNode* Node);
	void OnSelectedNodesChanged(const TSet<class UObject*>& NewSelection);

	TSharedPtr<SGraphActionMenu> GraphActionMenu;

#endif
};