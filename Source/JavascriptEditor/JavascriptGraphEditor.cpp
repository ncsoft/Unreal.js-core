#include "JavascriptEditor.h"
#include "JavascriptEdGraphNode.h"
#include "JavascriptGraphSchema.h"
#include "JavascriptGraphEditor.h"

#define LOCTEXT_NAMESPACE "UMG"

UJavascriptGraphEditor::UJavascriptGraphEditor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	TitleName = "GraphEditor";
	Graph = CreateDefaultSubobject<UEdGraph>(TEXT("EdGraph"));
	Graph->Schema = UJavascriptGraphSchema::StaticClass();
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
	NewNodePosition = InNodePosition;
	TSharedRef<SWidget> ActionMenu =
		SNew(SBorder)
		.Padding(4.0f)
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.GroupBorder"))
		[
			SAssignNew(GraphActionMenu, SGraphActionMenu)
			.OnActionSelected(BIND_UOBJECT_DELEGATE(SGraphActionMenu::FOnActionSelected, OnActionSelected))
			.OnCreateWidgetForAction(BIND_UOBJECT_DELEGATE(SGraphActionMenu::FOnCreateWidgetForAction, OnCreateWidgetForAction))
			.OnCollectAllActions(BIND_UOBJECT_DELEGATE(SGraphActionMenu::FOnCollectAllActions, CollectAllActions))
		];
	return FActionMenuContent(ActionMenu, GraphActionMenu->GetFilterTextBox());
}

void UJavascriptGraphEditor::CollectAllActions(FGraphActionListBuilderBase& OutAllActions)
{
	for (auto& Action : Actions)
	{
		if (nullptr == Action.Resource) continue;
		FGraphContextMenuBuilder ContextMenuBuilder(Graph);
		FFormatNamedArguments Arguments;
		Arguments.Add(TEXT("Name"), FText::FromName(Action.Name));
		Arguments.Add(TEXT("SelectedItems"), FText::FromString(Action.Resource->GetName()));
		const FText MenuDesc = FText::Format(LOCTEXT("NewJavascriptAction", "{Name}({SelectedItems})"), Arguments);
		const FText ToolTip = FText::Format(LOCTEXT("NewJavascriptActionTooltip", "Adds a {Name} node for {SelectedItems} here"), Arguments);
		const UJavascriptGraphSchema* Schema = Cast<const UJavascriptGraphSchema>(Graph->GetSchema());
		if (Schema)
		{
			Schema->GetGraphNodeContextActions(ContextMenuBuilder, 0);
		}
		TSharedPtr<FJSEdGraphSchemaAction_NewNode> NewNodeAction(new FJSEdGraphSchemaAction_NewNode(Action.Resource, FText::FromName(Action.Name), FText::FromName(Action.Category)));
		NewNodeAction->NodeTemplate = NewObject<UJavascriptEdGraphNode>(Graph);
		ContextMenuBuilder.AddAction(NewNodeAction);
		OutAllActions.Append(ContextMenuBuilder);
	}
}

void UJavascriptGraphEditor::OnActionSelected(const TArray< TSharedPtr<FEdGraphSchemaAction> >& SelectedActions, ESelectInfo::Type InSelectionType)
{
	if (InSelectionType == ESelectInfo::OnMouseClick || InSelectionType == ESelectInfo::OnKeyPress || SelectedActions.Num() == 0)
	{
		bool bDoDismissMenus = false;
		for (int32 ActionIndex = 0; ActionIndex < SelectedActions.Num(); ActionIndex++)
		{
			FEdGraphSchemaAction* CurrentAction = SelectedActions[ActionIndex].Get();
			if (CurrentAction != nullptr)
			{
				CurrentAction->PerformAction(Graph, nullptr, NewNodePosition);
				//UE_LOG(LogHAL, Log, TEXT("Category : %s, Key : %s"), *CurrentAction->GetCategory().ToString(), *CurrentAction->Key.ToString());
				bDoDismissMenus = true;
			}
		}
	
		if (bDoDismissMenus)
		{
			FSlateApplication::Get().DismissAllMenus();
		}
	}
}
TSharedRef<SWidget> UJavascriptGraphEditor::OnCreateWidgetForAction(struct FCreateWidgetForActionData* const InCreateData)
{
	return SNew(SDefaultGraphActionWidget, InCreateData);
}

void UJavascriptGraphEditor::AddActionContext(FJavascriptGraphAction Action)
{
	Actions.Add(Action);
}