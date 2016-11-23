#include "JavascriptEditor.h"
#include "ScopedTransaction.h"
#include "JavascriptEdGraphNode.h"

#define LOCTEXT_NAMESPACE "UMG"
#define SNAP_GRID (16) // @todo ensure this is the same as SNodePanel::GetSnapGridSize()

const int32 NodeDistance = 60;

FName FJSEdGraphSchemaAction_NewNode::StaticGetTypeId()
{
	static FName Type("FJSEdGraphSchemaAction_NewNode"); return Type;
}

FName FJSEdGraphSchemaAction_NewNode::GetTypeId() const
{
	return StaticGetTypeId();
}

FJSEdGraphSchemaAction_NewNode::FJSEdGraphSchemaAction_NewNode(UObject* InData, const FText& InKey, const FText& InCategory)
	: FEdGraphSchemaAction_NewNode()
	, Data(InData)
	, Key(InKey)
{
	check(Data);
	Update();
	UpdateCategory(InCategory);
}

void FJSEdGraphSchemaAction_NewNode::Update()
{
	UpdateSearchData(Key, FText::Format(LOCTEXT("JavascriptSchemaActionFormat", "Schema {0}"), Key).ToString(), FText(), FText());
}
//
//UEdGraphNode* FJSEdGraphSchemaAction_NewNode::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
//{
//	UEdGraphNode* ResultNode = NULL;
//
//	if (NodeTemplate != NULL)
//	{
//		const FScopedTransaction Transaction(LOCTEXT("AddNode", "Add Node"));
//		ParentGraph->Modify();
//		if (FromPin)
//		{
//			FromPin->Modify();
//		}
//		// set outer to be the graph so it doesn't go away
//		NodeTemplate->Rename(NULL, ParentGraph, REN_NonTransactional);
//		ParentGraph->AddNode(NodeTemplate, true);
//		NodeTemplate->SetFlags(RF_Transactional);
//
//
//		//// For input pins, new node will generally overlap node being dragged off
//		//// Work out if we want to visually push away from connected node
//		int32 XLocation = Location.X;
//		if (FromPin && FromPin->Direction == EGPD_Input)
//		{
//			UEdGraphNode* PinNode = FromPin->GetOwningNode();
//			const float XDelta = FMath::Abs(PinNode->NodePosX - Location.X);
//
//			if (XDelta < NodeDistance)
//			{
//				// Set location to edge of current node minus the max move distance
//				// to force node to push off from connect node enough to give selection handle
//				XLocation = PinNode->NodePosX - NodeDistance;
//			}
//		}
//
//		NodeTemplate->NodePosX = XLocation;
//		NodeTemplate->NodePosY = Location.Y;
//		NodeTemplate->SnapToGrid(SNAP_GRID);
//
//		// setup pins after placing node in correct spot, since pin sorting will happen as soon as link connection change occurs
//		NodeTemplate->AllocateDefaultPins();
//		NodeTemplate->AutowireNewNode(FromPin);
//
//		ResultNode = NodeTemplate;
//	}
//	return ResultNode;
//}

UJavascriptEdGraphNode::UJavascriptEdGraphNode(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

#undef LOCTEXT_NAMESPACE
