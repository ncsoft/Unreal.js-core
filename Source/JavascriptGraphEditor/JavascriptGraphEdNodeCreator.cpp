#include "JavascriptGraphEdNodeCreator.h"
#include "JavascriptGraphEdNode.h"

// Begin of FDefaultJavascriptGraphNodeCreator
FDefaultJavascriptGraphNodeCreator::FDefaultJavascriptGraphNodeCreator(UJavascriptGraphEdGraph* InGraph)
	: IJavascriptGraphNodeCreator(InGraph)
{
	Instance = MakeShareable(new FGraphNodeCreator<UJavascriptGraphEdNode>(*InGraph));
}

UJavascriptGraphEdNode* FDefaultJavascriptGraphNodeCreator::CreateNode(bool bSelectNewNode /*= true*/)
{
	return Instance->CreateNode(bSelectNewNode);
}

UJavascriptGraphEdNode* FDefaultJavascriptGraphNodeCreator::CreateUserInvokedNode(bool bSelectNewNode /*= true*/)
{
	return Instance->CreateUserInvokedNode(bSelectNewNode);
}

void FDefaultJavascriptGraphNodeCreator::Finalize()
{
	Instance->Finalize();
}
// End of FDefaultJavascriptGraphNodeCreator

// Begin of FCustomJavascriptGraphNodeCreator
FCustomJavascriptGraphNodeCreator::FCustomJavascriptGraphNodeCreator(UJavascriptGraphEdGraph* InGraph)
	: IJavascriptGraphNodeCreator(InGraph)
	, Graph(InGraph)
	, Node(nullptr)
	, bPlaced(false)
{
}

FCustomJavascriptGraphNodeCreator::~FCustomJavascriptGraphNodeCreator()
{
	checkf(bPlaced, TEXT("Created node was not finalized in a FCustomJavascriptGraphNodeCreator"));
}

UJavascriptGraphEdNode* FCustomJavascriptGraphNodeCreator::CreateNode(bool bSelectNewNode /*= true*/)
{
	check(Graph.IsValid());
	check(Node == nullptr);

	Node = Graph->CreateCustomNode(UJavascriptGraphEdNode::StaticClass());
	return Node;
}

UJavascriptGraphEdNode* FCustomJavascriptGraphNodeCreator::CreateUserInvokedNode(bool bSelectNewNode /*= true*/)
{
	check(Graph.IsValid());
	check(Node == nullptr);

	Node = Graph->CreateUserInvokedCustomNode(UJavascriptGraphEdNode::StaticClass());
	return Node;
}

void FCustomJavascriptGraphNodeCreator::Finalize()
{
	check(!bPlaced);
	Node->CreateNewGuid();
	Node->PostPlacedNewNode();
	bPlaced = true;
	if (Node->Pins.Num() == 0)
	{
		Node->AllocateDefaultPins();
	}
}
// End of FCustomJavascriptGraphNodeCreator
