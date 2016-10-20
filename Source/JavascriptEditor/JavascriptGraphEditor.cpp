#include "JavascriptEditor.h"
#include "JavascriptGraphEditor.h"

#define LOCTEXT_NAMESPACE "UMG"

UJavascriptGraphEditor::UJavascriptGraphEditor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TitleName = "GraphEditor";
	Graph = CreateDefaultSubobject<UEdGraph>(TEXT("EdGraph"));
	Graph->Schema = UEdGraphSchema::StaticClass();
	Graph->SetFlags(RF_Transient);
	GraphEditorCommands.Handle = MakeShareable(new FUICommandList);
}

TSharedRef<SWidget> UJavascriptGraphEditor::RebuildWidget()
{
	if (IsDesignTime())
	{
		return BuildDesignTimeWidget(SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("GraphEditor", "GraphEditor"))
			]);
	}
	else
	{
		// Make title bar
		TSharedRef<SWidget> TitleBarWidget =
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush(TEXT("Graph.TitleBackground")))
			.HAlign(HAlign_Fill)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
			.HAlign(HAlign_Center)
			.FillWidth(1.f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TitleName))
			.TextStyle(FEditorStyle::Get(), TEXT("GraphBreadcrumbButtonText"))
			]
			];

		SGraphEditor::FGraphEditorEvents InEvents;
		InEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateUObject(this, &UJavascriptGraphEditor::OnSelectedNodesChanged);
		InEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateUObject(this, &UJavascriptGraphEditor::OnNodeDoubleClicked);
		InEvents.OnCreateActionMenu = SGraphEditor::FOnCreateActionMenu::CreateUObject(this, &UJavascriptGraphEditor::OnCreateGraphActionMenu);

		const bool bGraphIsEditable = true;
		return SNew(SGraphEditor)
			.AdditionalCommands(GraphEditorCommands.Handle)
			.TitleBar(TitleBarWidget)
			.GraphToEdit(Graph)
			.GraphEvents(InEvents);
	}
}

void UJavascriptGraphEditor::OnNodeDoubleClicked(class UEdGraphNode* Node)
{

}

void UJavascriptGraphEditor::OnSelectedNodesChanged(const FGraphPanelSelectionSet& NewSelection)
{

}

FActionMenuContent UJavascriptGraphEditor::OnCreateGraphActionMenu(UEdGraph* InGraph, const FVector2D& InNodePosition, const TArray<UEdGraphPin*>& InDraggedPins, bool bAutoExpand, SGraphEditor::FActionMenuClosed InOnMenuClosed)
{
	TSharedRef<SWidget> ActionMenu =
		SNew(SBorder)
		.Padding(4.0f)
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SAssignNew(GraphActionMenu, SGraphActionMenu)
			.OnActionSelected(BIND_UOBJECT_DELEGATE(SGraphActionMenu::FOnActionSelected, OnActionSelected))
			.OnCollectAllActions(BIND_UOBJECT_DELEGATE(SGraphActionMenu::FOnCollectAllActions, CollectAllActions))
		];
	return FActionMenuContent(ActionMenu, GraphActionMenu->GetFilterTextBox());
}

void UJavascriptGraphEditor::CollectAllActions(FGraphActionListBuilderBase& OutAllActions)
{

}

void UJavascriptGraphEditor::OnActionSelected(const TArray< TSharedPtr<FEdGraphSchemaAction> >& SelectedAction, ESelectInfo::Type InSelectionType)
{

}

