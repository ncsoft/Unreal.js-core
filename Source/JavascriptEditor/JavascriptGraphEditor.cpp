#include "JavascriptEditor.h"
#include "JavascriptGraphEditor.h"

#define LOCTEXT_NAMESPACE "UMG"

FName FEdGraphSchemaAction_Javascript::StaticGetTypeId()
{
	static FName Type("FEdGraphSchemaAction_Javascript"); return Type;
}

FName FEdGraphSchemaAction_Javascript::GetTypeId() const
{
	return StaticGetTypeId();
}

FEdGraphSchemaAction_Javascript::FEdGraphSchemaAction_Javascript(UObject* InData, const FText& InKey, const FText& InCategory)
	: FEdGraphSchemaAction_Dummy()
	, Data(Data)
	, Key(InKey)
{
	check(Data);
	Update();
	UpdateCategory(InCategory);
}

void FEdGraphSchemaAction_Javascript::Update()
{
	UpdateSearchData(Key, FText::Format(LOCTEXT("JavascriptSchemaActionFormat", "Schema {0}"), Key).ToString(), FText(), FText());
}


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
	for (auto& Action : Actions)
	{
		if (nullptr == Action.Resource) continue;
		FGraphActionMenuBuilder ActionMenuBuilder;
		FFormatNamedArguments Arguments;
		Arguments.Add(TEXT("Name"), FText::FromName(Action.Name));
		Arguments.Add(TEXT("SelectedItems"), FText::FromString(Action.Resource->GetName()));
		const FText MenuDesc = FText::Format(LOCTEXT("NewJavascriptAction", "{Name}({SelectedItems})"), Arguments);
		const FText ToolTip = FText::Format(LOCTEXT("NewJavascriptActionTooltip", "Adds a {Name} node for {SelectedItems} here"), Arguments);
		TSharedPtr<FEdGraphSchemaAction_Javascript> NewNodeAction(new FEdGraphSchemaAction_Javascript(Action.Resource, FText::FromName(Action.Name), FText::FromName(Action.Category)));
		ActionMenuBuilder.AddAction(NewNodeAction);
		OutAllActions.Append(ActionMenuBuilder);
	}
}

void UJavascriptGraphEditor::OnActionSelected(const TArray< TSharedPtr<FEdGraphSchemaAction> >& SelectedActions, ESelectInfo::Type InSelectionType)
{
	if (InSelectionType == ESelectInfo::OnMouseClick || InSelectionType == ESelectInfo::OnKeyPress || SelectedActions.Num() == 0)
	{
		bool bDoDismissMenus = false;
		for (int32 ActionIndex = 0; ActionIndex < SelectedActions.Num(); ActionIndex++)
		{
			FEdGraphSchemaAction_Javascript* CurrentAction = (FEdGraphSchemaAction_Javascript*)(SelectedActions[ActionIndex].Get());
			if (CurrentAction != nullptr)
			{
				UE_LOG(LogHAL, Log, TEXT("Category : %s, Key : %s"), *CurrentAction->GetCategory().ToString(), *CurrentAction->Key.ToString());
				bDoDismissMenus = true;
			}
		}
	
		if (bDoDismissMenus)
		{
			FSlateApplication::Get().DismissAllMenus();
		}
	}
}

void UJavascriptGraphEditor::AddActionContext(FJavascriptGraphAction Action)
{
	Actions.Add(Action);
}