#include "JavascriptGraphEdGraph.h"
#include "JavascriptGraphEdNode.h"

UJavascriptGraphEdGraph::UJavascriptGraphEdGraph()
{
}

UJavascriptGraphEdGraph::~UJavascriptGraphEdGraph()
{
}

void UJavascriptGraphEdGraph::RebuildGenericGraph()
{
	//LOG_WARNING(TEXT("UJavascriptGraphEdGraph::RebuildGenericGraph has been called"));

	//UJavascriptGraph* G = CastChecked<UJavascriptGraph>(GetOuter());

	//G->ClearGraph();

	//for (int i = 0; i < Nodes.Num(); ++i)
	//{
	//	UJavascriptGraphEdNode* EdNode = Cast<UJavascriptGraphEdNode>(Nodes[i]);

	//	if (EdNode == nullptr || EdNode->GenericGraphNode == nullptr)
	//		continue;

	//	UGenericGraphNode* GNode = EdNode->GenericGraphNode;

	//	G->AllNodes.Add(GNode);

	//	for (int PinIdx = 0; PinIdx < EdNode->Pins.Num(); ++PinIdx)
	//	{
	//		UEdGraphPin* Pin = EdNode->Pins[PinIdx];

	//		if (Pin->Direction != EEdGraphPinDirection::EGPD_Output)
	//			continue;

	//		for (int LinkToIdx = 0; LinkToIdx < Pin->LinkedTo.Num(); ++LinkToIdx)
	//		{
	//			UJavascriptGraphEdNode* ChildEdNode = Cast<UJavascriptGraphEdNode>(Pin->LinkedTo[LinkToIdx]->GetOwningNode());

	//			if (ChildEdNode == nullptr)
	//				continue;

	//			UGenericGraphNode* ChildNode = ChildEdNode->GenericGraphNode;

	//			GNode->ChildrenNodes.Add(ChildNode);

	//			ChildNode->ParentNodes.Add(GNode);
	//		}
	//	}
	//}

	//for (int i = 0; i < G->AllNodes.Num(); ++i)
	//{
	//	UGenericGraphNode* Node = G->AllNodes[i];
	//	if (Node->ParentNodes.Num() == 0)
	//	{
	//		G->RootNodes.Add(Node);
	//	}
	//}
}

#if WITH_EDITOR
void UJavascriptGraphEdGraph::PostEditUndo()
{
	Super::PostEditUndo();

	RebuildGenericGraph();

	Modify();
}
#endif

void UJavascriptGraphEdGraph::AddCustomNode(UJavascriptGraphEdNode* NodeToAdd, bool bUserAction /*= false*/)
{
	check(NodeToAdd->GetOuter() == this);

	NodeToAdd->IsCustomNode = true;
	this->CustomNodes.Add(NodeToAdd);

	// Skip notifying graph action to prevent generating and adding node widget to SGraphPanel.
	// TODO: To prevent only generating and adding node widget but still notify changement of graph,
	//       we need to do following tasks.
	//       - Add a new class, SJavascriptGraphPanel which is inherited from SGraphPanel.
	//       - Implement SJavascriptGraphPanel::Construct like SGraphPanel::Construct
	//         but call UJavascriptGraphEdGraph.AddOnGraphChangedHandler to add a custom handler
	//         instead of SGraphPanel::OnGraphChanged.
	//       - Make a new custom handler provide chance to skip creating and adding node widget.
	//       Because SGraphPanel::AddNode doesn't provide chance to override its implementation,
	//       we have no choice other than inheriting SGraphPanel not to modify UE code.

	// Current implementation does not use bUserAction because there's no notification.
}

bool UJavascriptGraphEdGraph::RemoveCustomNode(UJavascriptGraphEdNode* NodeToRemove)
{
	Modify();

	int32 NumTimesNodeRemoved = CustomNodes.Remove(NodeToRemove);
#if WITH_EDITOR
	NodeToRemove->BreakAllNodeLinks();
#endif	//#if WITH_EDITOR

	// Skip notifying graph action to prevent removing node widget from SGraphPanel.
	// TODO: Refer to description in AddCustomNode.

	return NumTimesNodeRemoved > 0;
}

UJavascriptGraphEdNode* UJavascriptGraphEdGraph::CreateCustomNode(TSubclassOf<UJavascriptGraphEdNode> NewNodeClass, bool bFromUI)
{
	UJavascriptGraphEdNode* NewNode = NewObject<UJavascriptGraphEdNode>(this, NewNodeClass, NAME_None, RF_Transactional);

	if (HasAnyFlags(RF_Transient))
	{
		NewNode->SetFlags(RF_Transient);
	}

	AddCustomNode(NewNode, bFromUI);
	return NewNode;
}
