#include "JavascriptGraphEdCustomNodeWidget.h"
#include "NodeFactory.h"
#include "SGraphPanel.h"
#include "Widgets/Layout/SBox.h"
#include "JavascriptGraphEdNode.h"

UJavascriptGraphEdCustomNodeWidget::UJavascriptGraphEdCustomNodeWidget(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{}

void UJavascriptGraphEdCustomNodeWidget::SetNode(UJavascriptGraphEdNode* InEdNode)
{
	EdNode = InEdNode;
}

void UJavascriptGraphEdCustomNodeWidget::SetGraphPanel(FJavascriptSlateWidget InGraphPanel)
{
	TSharedPtr<SGraphPanel> NewGraphPanel = StaticCastSharedPtr<SGraphPanel>(InGraphPanel.Widget);

	bool bInvalidated = (GraphPanel != NewGraphPanel);

	GraphPanel = NewGraphPanel;

	if (bInvalidated)
	{
		InvalidateLayoutAndVolatility();
	}
}

TSharedRef<SWidget> UJavascriptGraphEdCustomNodeWidget::RebuildWidget()
{
	if (!EdNode)
	{
		UE_LOG(LogTemp, Error, TEXT("JavascriptGraphEdCustomNodeWidget requires setting JavascriptGraphEdNode by calling SetNode."));
		return SNew(SBox);
	}

	TSharedPtr<SGraphNode> NewNodeWidget = FNodeFactory::CreateNodeWidget(EdNode);
	NewNodeWidget->UpdateGraphNode();

	TSharedPtr<SGraphPanel> OwnerGraphPanel = GraphPanel.Pin();
	if (OwnerGraphPanel.IsValid())
	{
		NewNodeWidget->SetOwner(OwnerGraphPanel.ToSharedRef());
		OwnerGraphPanel->AttachGraphEvents(NewNodeWidget);
	}

	return NewNodeWidget->AsShared();
}
