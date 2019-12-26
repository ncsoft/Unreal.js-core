#include "JavascriptGraphEdNodeWidget.h"
#include "NodeFactory.h"
#include "Widgets/Layout/SBox.h"
#include "JavascriptGraphEdNode.h"

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

	return StaticCastSharedRef<SWidget>(NewNode.ToSharedRef());
}
