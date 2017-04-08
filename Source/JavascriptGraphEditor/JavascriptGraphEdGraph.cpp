#include "JavascriptGraphEditorPrivatePCH.h"
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

