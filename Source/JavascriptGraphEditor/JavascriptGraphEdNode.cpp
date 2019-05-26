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
	const FName PinCategory,
	const FName PinSubCategory,
	UObject* PinSubCategoryObject,
	const FName PinName,
	const FString& PinToolTip,
	const FText& PinDisplayName,
	const FJavascriptPinParams& InPinParams
	)
{
	FCreatePinParams PinParams;
	PinParams.bIsConst = InPinParams.bIsConst;
	PinParams.bIsReference = InPinParams.bIsReference;
	PinParams.ContainerType = InPinParams.ContainerType;
	PinParams.Index = InPinParams.Index;

	UEdGraphPin* GraphPin = Super::CreatePin(Dir, PinCategory, PinSubCategory, PinSubCategoryObject, PinName, PinParams);
	GraphPin->PinToolTip = PinToolTip;
#if WITH_EDITORONLY_DATA
	GraphPin->PinFriendlyName = PinDisplayName;
#endif
	return FJavascriptEdGraphPin(GraphPin);
}

bool UJavascriptGraphEdNode::RemovePinByName(FName PinName)
{
	UEdGraphPin* Pin = FindPin(PinName);
	if (Pin != nullptr)
	{
		return RemovePin(Pin);
	}

	return false;
}

bool UJavascriptGraphEdNode::RemovePin(FJavascriptEdGraphPin Pin)
{
	if (Pin.IsValid())
	{
		return Super::RemovePin(Pin);
	}

	return false;
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

int32 UJavascriptGraphEdNode::GetNumOfPins(EEdGraphPinDirection Direction /*= EGPD_MAX*/) const
{
	int32 NumOfPin;

	if (Direction == EGPD_Input || Direction == EGPD_Output)
	{
		NumOfPin = 0;
		for (UEdGraphPin* Pin : Pins)
		{
			if (Pin->Direction == Direction)
			{
				++NumOfPin;
			}
		}
	}
	else
	{
		NumOfPin = Pins.Num();
	}

	return NumOfPin;
}

void UJavascriptGraphEdNode::SetEnable(bool bEnable)
{
	if (SlateGraphNode)
	{
		SlateGraphNode->SetEnabled(bEnable);
	}
}

void UJavascriptGraphEdNode::SetVisible(bool bVisible)
{
	if (SlateGraphNode)
	{
		SlateGraphNode->SetVisibility(bVisible ? EVisibility::Visible : EVisibility::Hidden);
	}
}

bool UJavascriptGraphEdNode::GetVisible()
{
	if (SlateGraphNode)
	{
		return (SlateGraphNode->GetVisibility() == EVisibility::Visible);
	}
	return false;
}

void UJavascriptGraphEdNode::SetTitleSelectionMode(float InTitleHeight)
{
	this->bTitleSelectionOnly = true;
	this->TitleHeight = InTitleHeight;
}

void UJavascriptGraphEdNode::ResetTitleSelectionMode()
{
	this->bTitleSelectionOnly = false;
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