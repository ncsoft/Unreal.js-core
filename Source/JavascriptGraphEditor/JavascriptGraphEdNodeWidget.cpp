#include "JavascriptGraphEdNodeWidget.h"
#include "NodeFactory.h"
#include "Widgets/Layout/SBox.h"

UJavascriptGraphEdNodeWidget::UJavascriptGraphEdNodeWidget(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{}

void UJavascriptGraphEdNodeWidget::SetNode(UJavascriptGraphEdNode* InEdNode)
{
	EdNode = InEdNode;
}

TSharedRef<SWidget> UJavascriptGraphEdNodeWidget::RebuildWidget()
{
	if (!EdNode) return SNew(SBox);

	TSharedPtr<SGraphNode> NewNode = FNodeFactory::CreateNodeWidget(EdNode);

	NewNode->UpdateGraphNode();
	// TODO:
	//NewNode->SetOwner();
	EdNode->SlateGraphNode = static_cast<SJavascriptGraphEdNode*>(NewNode.Get());

	return StaticCastSharedRef<SWidget>(NewNode.ToSharedRef());
}