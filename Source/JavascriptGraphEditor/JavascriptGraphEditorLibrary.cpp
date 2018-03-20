#include "JavascriptGraphEditorLibrary.h"
#include "JavascriptGraphConnectionDrawingPolicy.h"
#include "JavascriptGraphEdNode.h"
#include "SJavascriptGraphEdNode.h"
#include "Editor/GraphEditor/Public/SGraphPanel.h"

#define LOCTEXT_NAMESPACE "JavascriptGraph"

#define DO_OP() OP(WireColor);	OP(AssociatedPin1);	OP(AssociatedPin2);	OP(WireThickness);	OP(bDrawBubbles);	OP(bUserFlag1);	OP(bUserFlag2);	OP(StartDirection);	OP(EndDirection);
FJavascriptConnectionParams::FJavascriptConnectionParams(const FConnectionParams& In)
{
#define OP(x) x = In.x
	DO_OP()
#undef OP
}

FJavascriptConnectionParams::operator FConnectionParams () const
{
	FConnectionParams Out;
#define OP(x) Out.x = x
	DO_OP()
#undef OP
		return Out;
}
#undef DO_OP

FJavascriptNodeCreator UJavascriptGraphEditorLibrary::NodeCreator(UJavascriptGraphEdGraph* Graph)
{
	FJavascriptNodeCreator Out;
	auto Creator = new FGraphNodeCreator<UJavascriptGraphEdNode>(*Graph);
	Out.Instance = MakeShareable(reinterpret_cast<FGraphNodeCreator<UEdGraphNode>*>(Creator));
	Out.Node = Creator->CreateNode();
	return Out;
}

void UJavascriptGraphEditorLibrary::Finalize(FJavascriptNodeCreator& Creator)
{
	Creator.Instance->Finalize();
}

bool UJavascriptGraphEditorLibrary::SetNodeMetaData(UEdGraphSchema* Schema, UEdGraphNode* Node, FName KeyValue)
{
	return Schema->SetNodeMetaData(Node, KeyValue);
}

void UJavascriptGraphEditorLibrary::MakeLinkTo(FJavascriptEdGraphPin A, FJavascriptEdGraphPin B)
{
	if (A.GraphPin && B.GraphPin)
	{
		A.GraphPin->MakeLinkTo(B.GraphPin);
	}	
}

void UJavascriptGraphEditorLibrary::TryConnection(UEdGraphSchema* Schema, FJavascriptEdGraphPin A, FJavascriptEdGraphPin B)
{
	if (A.GraphPin && B.GraphPin)
	{
		Schema->TryCreateConnection(A.GraphPin, B.GraphPin);
	}
}

void UJavascriptGraphEditorLibrary::BreakLinkTo(FJavascriptEdGraphPin A, FJavascriptEdGraphPin B)
{
	if (A.GraphPin && B.GraphPin)
	{
		A.GraphPin->BreakLinkTo(B.GraphPin);
	}
}

void UJavascriptGraphEditorLibrary::BreakAllPinLinks(FJavascriptEdGraphPin A)
{
	if (A.GraphPin)
	{
		A.GraphPin->BreakAllPinLinks();
	}
}

FEdGraphPinType UJavascriptGraphEditorLibrary::GetPinType(FJavascriptEdGraphPin A)
{
	FEdGraphPinType PinType;
	if (A.GraphPin)
	{
		PinType = A.GraphPin->PinType;
	}

	return PinType;
}

EJavascriptPinContainerType::Type UJavascriptGraphEditorLibrary::GetPinContainerType(FJavascriptEdGraphPin A)
{
	EJavascriptPinContainerType::Type ContainerType = EJavascriptPinContainerType::Type::None;
	if (A.GraphPin)
	{
		
		ContainerType = (EJavascriptPinContainerType::Type)(int32)(A.GraphPin->PinType.ContainerType);
	}

	return ContainerType;
}

void UJavascriptGraphEditorLibrary::SetPinType(FJavascriptEdGraphPin Pin, FEdGraphPinType PinType)
{
	if (Pin.GraphPin)
	{
		Pin.GraphPin->PinType = PinType;
	}
}

FJavascriptEdGraphPin UJavascriptGraphEditorLibrary::FindPin(UEdGraphNode* Node, const FString& PinName, EEdGraphPinDirection Direction)
{
	return FJavascriptEdGraphPin{ Node->FindPin(PinName, Direction) };
}

FName UJavascriptGraphEditorLibrary::GetPinName(FJavascriptEdGraphPin A)
{
	return A.GraphPin ? A.GraphPin->PinName : TEXT("");
}

void UJavascriptGraphEditorLibrary::SetPinInfo(FJavascriptEdGraphPin A, FName InPinName, FString InPinToolTip)
{
	if (A.GraphPin)
	{
		A.GraphPin->PinName = InPinName;
		A.GraphPin->PinToolTip = InPinToolTip;
	}
}

FGuid UJavascriptGraphEditorLibrary::GetPinGUID(FJavascriptEdGraphPin A)
{
	FGuid GUID;
	if (A.GraphPin)
	{
		GUID = A.GraphPin->PinId;
	}
	return GUID;
}

bool UJavascriptGraphEditorLibrary::IsValid(FJavascriptEdGraphPin A)
{
	return A.GraphPin != nullptr;
}

class UEdGraphNode* UJavascriptGraphEditorLibrary::GetOwningNode(FJavascriptEdGraphPin A)
{
	return A.GraphPin ? A.GraphPin->GetOwningNode() : nullptr;
}

EEdGraphPinDirection UJavascriptGraphEditorLibrary::GetDirection(FJavascriptEdGraphPin A)
{
	return A.GraphPin ? ((EEdGraphPinDirection)A.GraphPin->Direction) : EEdGraphPinDirection::EGPD_Input;
}

TArray<FJavascriptEdGraphPin> TransformPins(const TArray<UEdGraphPin*>& Pins)
{
	TArray<FJavascriptEdGraphPin> Out;
	for (auto x : Pins)
	{
		Out.Add(FJavascriptEdGraphPin{ x });
	}
	return Out;
}

TArray<FJavascriptEdGraphPin> UJavascriptGraphEditorLibrary::GetLinkedTo(FJavascriptEdGraphPin A)
{
	return TransformPins(A.GraphPin->LinkedTo);
}

TArray<FJavascriptEdGraphPin> UJavascriptGraphEditorLibrary::GetPins(UEdGraphNode* Node)
{
	return TransformPins(Node->Pins);
}
// @unused
FJavascriptArrangedWidget UJavascriptGraphEditorLibrary::FindPinGeometries(FJavascriptDetermineLinkGeometryContainer Container, FJavascriptPinWidget PinWidget)
{
	FJavascriptArrangedWidget Widget;
	Widget.Handle = Container.PinGeometries->Find(*(PinWidget.Handle));

	return Widget;
}
// @unused
FJavascriptPinWidget UJavascriptGraphEditorLibrary::FindPinToPinWidgetMap(FJavascriptDetermineLinkGeometryContainer Container, FJavascriptEdGraphPin Pin)
{
	FJavascriptPinWidget Widget = FJavascriptPinWidget();
	
	TSharedRef<SGraphPin>* SGraphPinHandle = Container.PinToPinWidgetMap->Find(Pin.GraphPin);
	if (SGraphPinHandle)
	{
		TSharedRef<SWidget> SWidgetHandle  = static_cast<SWidget&>(SGraphPinHandle->Get()).AsShared();
		Widget.Handle = &SWidgetHandle;
	}

	return Widget;
}
// @unused
FJavascriptArrangedWidget UJavascriptGraphEditorLibrary::GetArrangedNodes(FJavascriptDetermineLinkGeometryContainer Container, UEdGraphNode* Node)
{
	FJavascriptArrangedWidget Widget = FJavascriptArrangedWidget();

	int32* Index = Container.NodeWidgetMap->Find(Node);
	if (Index != NULL)
	{
		Widget.Handle = &(*Container.ArrangedNodes)[*Index];
	}

	return Widget;
}
// @unused
FJavascriptPinWidget UJavascriptGraphEditorLibrary::GetOutputPinWidget(FJavascriptDetermineLinkGeometryContainer Container)
{
	FJavascriptPinWidget Widget = FJavascriptPinWidget();
	Widget.Handle = Container.OutputPinWidget;

	return Widget;
}

void UJavascriptGraphEditorLibrary::DrawConnection(FJavascriptGraphConnectionDrawingPolicyContainer Container, const FVector2D& A, const FVector2D& B, const FJavascriptConnectionParams& Params)
{
	FJavascriptGraphConnectionDrawingPolicy* DrawingPolicy = Container.Handle;
	if (DrawingPolicy)
	{
		int32 WireLayerID = DrawingPolicy->GetWireLayerID();
		DrawingPolicy->DrawConnection(WireLayerID, A, B, Params);
	}
}

void UJavascriptGraphEditorLibrary::MakeRotatedBox(FJavascriptGraphConnectionDrawingPolicyContainer Container, FVector2D ArrowDrawPos, float AngleInRadians, FLinearColor WireColor)
{
	FJavascriptGraphConnectionDrawingPolicy* DrawingPolicy = Container.Handle;
	if (DrawingPolicy)
	{
		DrawingPolicy->MakeRotatedBox(ArrowDrawPos, AngleInRadians, WireColor);
	}
}

int UJavascriptGraphEditorLibrary::GetHorveredPinNum(FJavascriptGraphConnectionDrawingPolicyContainer Container)
{
	FJavascriptGraphConnectionDrawingPolicy* DrawingPolicy = Container.Handle;
	if (DrawingPolicy)
	{
		return DrawingPolicy->GetHoveredPins().Num();
	}

	return 0;
}

bool UJavascriptGraphEditorLibrary::IsContainedHoveredPins(FJavascriptGraphConnectionDrawingPolicyContainer Container, FJavascriptEdGraphPin Pin)
{
	FJavascriptGraphConnectionDrawingPolicy* DrawingPolicy = Container.Handle;
	if (DrawingPolicy)
	{
		return DrawingPolicy->GetHoveredPins().Contains(Pin.GraphPin);
	}

	return false;
}

void UJavascriptGraphEditorLibrary::ApplyHoverDeemphasis(FJavascriptGraphConnectionDrawingPolicyContainer Container, FJavascriptEdGraphPin OutputPin, FJavascriptEdGraphPin InputPin, float Thickness, FLinearColor WireColor)
{
	FJavascriptGraphConnectionDrawingPolicy* DrawingPolicy = Container.Handle;
	if (DrawingPolicy)
	{
		DrawingPolicy->ApplyHoverDeemphasis(OutputPin.GraphPin, InputPin.GraphPin, Thickness, WireColor);
	}
}

void UJavascriptGraphEditorLibrary::DetermineWiringStyle(FJavascriptGraphConnectionDrawingPolicyContainer Container, FJavascriptEdGraphPin OutputPin, FJavascriptEdGraphPin InputPin, FJavascriptConnectionParams& Params)
{
	FJavascriptGraphConnectionDrawingPolicy* DrawingPolicy = Container.Handle;
	if (DrawingPolicy)
	{
		FConnectionParams X = Params;
		DrawingPolicy->DetermineWiringStyle(OutputPin.GraphPin, InputPin.GraphPin, X);
		Params = X;
	}
}

void UJavascriptGraphEditorLibrary::DrawSplineWithArrow(FJavascriptGraphConnectionDrawingPolicyContainer Container, FVector2D StartAnchorPoint, FVector2D EndAnchorPoint, FJavascriptConnectionParams Params)
{
	FJavascriptGraphConnectionDrawingPolicy* DrawingPolicy = Container.Handle;
	if (DrawingPolicy)
	{
		DrawingPolicy->DrawSplineWithArrow(StartAnchorPoint, EndAnchorPoint, Params);
	}
}

void UJavascriptGraphEditorLibrary::AddPinToHoverSet(const FJavascriptSlateEdNode& InSlateEdNode, FJavascriptEdGraphPin Pin)
{
	SJavascriptGraphEdNode* SlateEdNode = InSlateEdNode.Handle;
	if (SlateEdNode && Pin.GraphPin)
	{
		SlateEdNode->GetOwnerPanel()->AddPinToHoverSet(Pin.GraphPin);
	}
}

void UJavascriptGraphEditorLibrary::RemovePinFromHoverSet(const FJavascriptSlateEdNode& InSlateEdNode, FJavascriptEdGraphPin Pin)
{
	SJavascriptGraphEdNode* SlateEdNode = InSlateEdNode.Handle;
	if (SlateEdNode && Pin.GraphPin)
	{
		SlateEdNode->GetOwnerPanel()->RemovePinFromHoverSet(Pin.GraphPin);
	}
}

void UJavascriptGraphEditorLibrary::ResizeNode(UEdGraphNode * Node, const FVector2D & NewSize)
{
	Node->ResizeNode(NewSize);
}

#undef LOCTEXT_NAMESPACE
