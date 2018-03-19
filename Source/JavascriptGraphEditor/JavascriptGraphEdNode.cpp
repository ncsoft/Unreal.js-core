#include "JavascriptGraphEdNode.h"
#include "JavascriptGraphAssetGraphSchema.h"

#define LOCTEXT_NAMESPACE "JavascriptGraphEdNode"

void UJavascriptGraphEdNode::AllocateDefaultPins()
{
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GetSchema());
	Schema->OnAllocateDefaultPins.ExecuteIfBound(this);
}

void UJavascriptGraphEdNode::NodeConnectionListChanged()
{
	Super::NodeConnectionListChanged();

	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GetSchema());
	Schema->OnNodeConnectionListChanged.ExecuteIfBound(this);
}

void UJavascriptGraphEdNode::PinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::PinConnectionListChanged(Pin);

	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GetSchema());
	Schema->OnPinConnectionListChanged.ExecuteIfBound(FJavascriptEdGraphPin{ const_cast<UEdGraphPin*>(Pin) });
}

UJavascriptGraphEdGraph* UJavascriptGraphEdNode::GetGenericGraphEdGraph()
{
	return Cast<UJavascriptGraphEdGraph>(GetGraph());
}

FText UJavascriptGraphEdNode::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	if (GraphNode)
	{
		auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GetSchema());
		if (Schema->OnGetString.IsBound())
		{
			return Schema->OnGetString.Execute(this, EGraphSchemaGetStringQuery::Title);
		}
	}

	return FText();
}

FText UJavascriptGraphEdNode::GetDescription() const
{
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GetSchema());
	if (Schema->OnGetString.IsBound())
	{
		return Schema->OnGetString.Execute(this, EGraphSchemaGetStringQuery::Description);
	}
	
	return FText();
}

FJavascriptEdGraphPin UJavascriptGraphEdNode::CreatePin(
	EEdGraphPinDirection Dir,
	const FString& PinCategory,
	const FString& PinSubCategory,
	UObject* PinSubCategoryObject,
	const FString& PinName,
	EPinContainerType PinContainerType /* EPinContainerType::None */,
	bool bIsReference,
	bool bIsConst /*= false*/,
	//int32 Index /*= INDEX_NONE*/
	const FString& PinToolTip
	)
{
	UEdGraphPin* GraphPin = Super::CreatePin(Dir, PinCategory, PinSubCategory, PinSubCategoryObject, PinName, PinContainerType, bIsReference, bIsConst, INDEX_NONE);
	GraphPin->PinToolTip = PinToolTip;
	return FJavascriptEdGraphPin{ GraphPin };
}

void UJavascriptGraphEdNode::UpdateSlate()
{
	if (SlateGraphNode)
	{
		SlateGraphNode->UpdateGraphNode();
	}
}

FVector2D UJavascriptGraphEdNode::GetDesiredSize()
{
	FVector2D Size;
	if (SlateGraphNode)
	{
		Size = SlateGraphNode->GetDesiredSize();
	}
	else
	{
		Size.X = NodeWidth;
		Size.Y = NodeHeight;
	}
	return Size;
}

void UJavascriptGraphEdNode::ResizeNode(const FVector2D& NewSize)
{
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GetSchema());
	if (Schema->OnIsNodeComment.IsBound() && Schema->OnIsNodeComment.Execute(this))
	{
		if (bCanResizeNode)
		{
			NodeWidth = NewSize.X;
			NodeHeight = NewSize.Y;
		}
	}
}

#undef LOCTEXT_NAMESPACE