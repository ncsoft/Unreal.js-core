#include "JavascriptGraphEditorPrivatePCH.h"
#include "JavascriptGraphEditorWidget.h"
#include "JavascriptGraphAssetGraphSchema.h"
#include "JavascriptGraphEdGraph.h"

UJavascriptGraphEditorWidget::UJavascriptGraphEditorWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

TSharedRef<SWidget> UJavascriptGraphEditorWidget::RebuildWidget()
{
	SGraphEditor::FGraphEditorEvents InEvents;
	InEvents.OnSelectionChanged = BIND_UOBJECT_DELEGATE(SGraphEditor::FOnSelectionChanged, HandleOnSelectedNodesChanged);
	InEvents.OnNodeDoubleClicked = BIND_UOBJECT_DELEGATE(FSingleNodeEvent, HandleOnNodeDoubleClicked);
	InEvents.OnDropActor = BIND_UOBJECT_DELEGATE(SGraphEditor::FOnDropActor, HandleDropActors);
	InEvents.OnDisallowedPinConnection = BIND_UOBJECT_DELEGATE(SGraphEditor::FOnDisallowedPinConnection, HandleDisallowedPinConnection);

	if (!EdGraph) return SNew(SBox);

	FGraphAppearanceInfo Info = AppearanceInfo;

	return SNew(SGraphEditor)
		.AdditionalCommands(CommandList)
		.IsEditable(true)
		.Appearance(Info)
		.GraphToEdit(EdGraph)
		.GraphEvents(InEvents)
		.AutoExpandActionMenu(true)
		.ShowGraphStateOverlay(false);
}

void UJavascriptGraphEditorWidget::SetGraph(UJavascriptGraphEdGraph* InEdGraph)
{
	EdGraph = InEdGraph;
}

UJavascriptGraphEdGraph* UJavascriptGraphEditorWidget::NewGraph(UObject* ParentScope)
{
	auto EdGraph = CastChecked<UJavascriptGraphEdGraph>(FBlueprintEditorUtils::CreateNewGraph(ParentScope, NAME_None, UJavascriptGraphEdGraph::StaticClass(), UJavascriptGraphAssetGraphSchema::StaticClass()));
	EdGraph->bAllowDeletion = false;

	// Give the schema a chance to fill out any required nodes (like the results node)
	const UEdGraphSchema* Schema = EdGraph->GetSchema();
	Schema->CreateDefaultNodesForGraph(*EdGraph);

	return EdGraph;
}

void UJavascriptGraphEditorWidget::HandleOnSelectedNodesChanged(const FGraphPanelSelectionSet& Selection)
{
	TArray<UObject*> Array;
	for (auto It = Selection.CreateConstIterator(); It; ++It)
	{
		Array.Add(*It);
	}

	OnSelectedNodesChanged.ExecuteIfBound(Array);
}

void UJavascriptGraphEditorWidget::HandleOnNodeDoubleClicked(UEdGraphNode* Node)
{
	OnNodeDoubleClicked.ExecuteIfBound(Node);
}

void UJavascriptGraphEditorWidget::ClearSelectionSet()
{
	auto Widget = StaticCastSharedPtr<SGraphEditor>(GetCachedWidget());
	Widget->ClearSelectionSet();
}

TArray<UObject*> UJavascriptGraphEditorWidget::GetSelectedNodes()
{
	auto Widget = StaticCastSharedPtr<SGraphEditor>(GetCachedWidget());
	auto Set = Widget->GetSelectedNodes();
	TArray<UObject*> Out;
	for (auto x : Set)
	{
		Out.Add(x);
	}
	return Out;
}

void UJavascriptGraphEditorWidget::SetNodeSelection(UEdGraphNode* Node, bool bSelect)
{
	auto Widget = StaticCastSharedPtr<SGraphEditor>(GetCachedWidget());
	Widget->SetNodeSelection(Node, bSelect);
}

void UJavascriptGraphEditorWidget::SelectAllNodes()
{
	auto Widget = StaticCastSharedPtr<SGraphEditor>(GetCachedWidget());
	Widget->SelectAllNodes();
}

void UJavascriptGraphEditorWidget::HandleDropActors(const TArray< TWeakObjectPtr<class AActor> >& Actors, class UEdGraph* Graph, const FVector2D& Point)
{
	TArray<AActor*> Out;
	for (auto x : Actors)
	{
		if (auto y = x.Get())
		{
			Out.Add(y);
		}
	}

	OnDropActor.ExecuteIfBound(Out, Graph, Point);
}

void UJavascriptGraphEditorWidget::HandleDisallowedPinConnection(const UEdGraphPin* A, const UEdGraphPin* B)
{
	OnDisallowedPinConnection.ExecuteIfBound(const_cast<UEdGraphPin*>(A), const_cast<UEdGraphPin*>(B));
}