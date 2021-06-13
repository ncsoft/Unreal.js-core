#include "JavascriptGraphAssetGraphSchema.h"
#include "JavascriptGraphEdNode.h"
#include "JavascriptGraphConnectionDrawingPolicy.h"

FJavascriptGraphAction_NewNode::FJavascriptGraphAction_NewNode(const FEdGraphSchemaAction& Template, const UJavascriptGraphAssetGraphSchema* InSchema)
	: FEdGraphSchemaAction(Template.GetCategory(), Template.GetMenuDescription(), Template.GetTooltipDescription(), Template.GetGrouping(), Template.GetKeywords(), Template.GetSectionID()), Schema(InSchema)
{}

UEdGraphNode* FJavascriptGraphAction_NewNode::PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode)
{
	TArray<UEdGraphPin*> Array;
	Array.Add(FromPin);
	return PerformAction(ParentGraph, Array, Location, bSelectNewNode);
}

UEdGraphNode* FJavascriptGraphAction_NewNode::PerformAction(class UEdGraph* ParentGraph, TArray<UEdGraphPin*>& FromPins, const FVector2D Location, bool bSelectNewNode)
{
	if (Schema->OnPerformAction.IsBound())
	{
		FPerformActionContext Context;
		Context.ParentGraph = ParentGraph;
		for (auto pin : FromPins)
		{
			Context.FromPins.Add(FJavascriptEdGraphPin(pin));
		}
		Context.Location = Location;
		Context.bSelectNewNode = bSelectNewNode;
		return Schema->OnPerformAction.Execute(*this, Context);
	}
	else
	{
		return nullptr;
	}
}

void UJavascriptGraphAssetGraphSchema::BreakPinLinks(FJavascriptEdGraphPin TargetPin, bool bSendsNodeNotifcation)
{
	if (TargetPin.IsValid())
	{
		BreakPinLinks(TargetPin, bSendsNodeNotifcation);
	}
}

void UJavascriptGraphAssetGraphSchema::BreakSinglePinLink(FJavascriptEdGraphPin SourcePin, FJavascriptEdGraphPin TargetPin)
{
	if (SourcePin.IsValid() && TargetPin.IsValid())
	{
		Super::BreakSinglePinLink(SourcePin, TargetPin);
	}
}

void UJavascriptGraphAssetGraphSchema::BreakNodeLinks(UEdGraphNode* TargetNode)
{
	if (TargetNode)
	{
		Super::BreakNodeLinks(*TargetNode);
	}
}

void UJavascriptGraphAssetGraphSchema::GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const
{
	const bool bNoParent = (ContextMenuBuilder.FromPin == NULL);

	FJavascriptEdGraphPin FromPin = FJavascriptEdGraphPin{ const_cast<UEdGraphPin*>(ContextMenuBuilder.FromPin) };

	if (OnContextActions.IsBound())
	{
		auto Actions = OnContextActions.Execute(FromPin);

		for (const auto& Action : Actions)
		{
			TSharedPtr<FJavascriptGraphAction_NewNode> NewNodeAction(new FJavascriptGraphAction_NewNode(Action,this));

			ContextMenuBuilder.AddAction(NewNodeAction);
		}
	}
}

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 24
void UJavascriptGraphAssetGraphSchema::GetContextMenuActions(const UEdGraph* CurrentGraph, const UEdGraphNode* InGraphNode, const UEdGraphPin* InGraphPin, class FMenuBuilder* MenuBuilder, bool bIsDebugging) const
{
	if (OnBuildMenu.IsBound())
	{
		FJavascriptGraphMenuBuilder Builder;
		Builder.Menu = MenuBuilder;
		Builder.MultiBox = MenuBuilder;
		Builder.Graph = CurrentGraph;
		Builder.GraphNode = InGraphNode;
		Builder.GraphPin.GraphPin = const_cast<UEdGraphPin*>(InGraphPin);
		Builder.bIsDebugging = bIsDebugging;

		OnBuildMenu.Execute(Builder);
		return;
	}

	Super::GetContextMenuActions(CurrentGraph, InGraphNode, InGraphPin, MenuBuilder, bIsDebugging);
}
#else
void UJavascriptGraphAssetGraphSchema::GetContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const
{
	//@ Todo
	
	if (OnBuildMenu.IsBound())
	{
		FJavascriptGraphMenuBuilder Builder;
		Builder.ToolMenu = Menu;
		Builder.Graph = Context->Graph;
		Builder.GraphNode = Context->Node;
		Builder.GraphPin.GraphPin = const_cast<UEdGraphPin*>(Context->Pin);
		Builder.bIsDebugging = Context->bIsDebugging;

		OnBuildMenu.Execute(Builder);
		return;
	}

	Super::GetContextMenuActions(Menu, Context);
}
#endif

const FPinConnectionResponse UJavascriptGraphAssetGraphSchema::CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const
{
	if (OnCanCreateConnection.IsBound())
	{
		auto Result = OnCanCreateConnection.Execute(FJavascriptEdGraphPin{ const_cast<UEdGraphPin*>(A) }, FJavascriptEdGraphPin{ const_cast<UEdGraphPin*>(B) });
		return FPinConnectionResponse(Result.Response, Result.Message);
	}

	return FPinConnectionResponse(CONNECT_RESPONSE_MAKE, FText::FromString(TEXT("Connect")));
}

class FConnectionDrawingPolicy* UJavascriptGraphAssetGraphSchema::CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const
{
	return new FJavascriptGraphConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements, InGraphObj);
}

// To make UBT happy
void UJavascriptGraphAssetGraphSchema::BreakNodeLinks(UEdGraphNode& TargetNode) const
{
	Super::BreakNodeLinks(TargetNode);
}

// To make UBT happy
void UJavascriptGraphAssetGraphSchema::BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const
{
	Super::BreakPinLinks(TargetPin, bSendsNodeNotifcation);
}

// To make UBT happy
void UJavascriptGraphAssetGraphSchema::BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) const
{
	Super::BreakSinglePinLink(SourcePin, TargetPin);
}

bool UJavascriptGraphAssetGraphSchema::TryCreateConnection(UEdGraphPin* PinA, UEdGraphPin* PinB) const
{
	if (OnTryCreateConnection.IsBound())
	{
		FJavascriptEdGraphPin A = FJavascriptEdGraphPin{ const_cast<UEdGraphPin*>(PinA) };
		FJavascriptEdGraphPin B = FJavascriptEdGraphPin{ const_cast<UEdGraphPin*>(PinB) };
		OnTryCreateConnection.Execute(A, B);
		PinA = A.GraphPin;
		PinB = B.GraphPin;
	}

	const bool bModified = UEdGraphSchema::TryCreateConnection(PinA, PinB);
	return bModified;
}

bool UJavascriptGraphAssetGraphSchema::CreateAutomaticConversionNodeAndConnections(UEdGraphPin* PinA, UEdGraphPin* PinB) const
{
	if (OnCreateAutomaticConversionNodeAndConnections.IsBound())
	{
		return OnCreateAutomaticConversionNodeAndConnections.Execute(FJavascriptEdGraphPin{ const_cast<UEdGraphPin*>(PinA) }, FJavascriptEdGraphPin{ const_cast<UEdGraphPin*>(PinB) });
	}

	return false;
}

bool UJavascriptGraphAssetGraphSchema::ShouldAlwaysPurgeOnModification() const
{
	if (OnShouldAlwaysPurgeOnModification.IsBound())
	{
		return OnShouldAlwaysPurgeOnModification.Execute();
	}

	return Super::ShouldAlwaysPurgeOnModification();
}

bool UJavascriptGraphEditorLibrary::CanUserDeleteNode(UEdGraphNode* Node)
{
	return Node && Node->CanUserDeleteNode();
}

bool UJavascriptGraphEditorLibrary::CanDuplicateNode(UEdGraphNode* Node)
{
	return Node && Node->CanDuplicateNode();
}

void UJavascriptGraphEditorLibrary::DestroyNode(UEdGraphNode* Node)
{
	if (Node)
	{
		Node->DestroyNode();
	}
}

void UJavascriptGraphEditorLibrary::AutowireNewNode(UEdGraphNode* Node, FJavascriptEdGraphPin FromPin)
{
	if (Node)
	{
		Node->AutowireNewNode(FromPin);
	}
}