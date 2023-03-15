#include "JavascriptGraphEditorLibrary.h"
#include "JavascriptGraphConnectionDrawingPolicy.h"
#include "JavascriptGraphEdNode.h"
#include "JavascriptGraphEdNodeCreator.h"
#include "SJavascriptGraphEdNode.h"
#include "SGraphPanel.h"

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

FJavascriptNodeCreator UJavascriptGraphEditorLibrary::NodeCreator(UJavascriptGraphEdGraph* Graph, bool bSelectNewNode/* = true*/)
{
	FJavascriptNodeCreator Out;
	auto Creator = new FDefaultJavascriptGraphNodeCreator(Graph);
	Out.Instance = MakeShareable(static_cast<IJavascriptGraphNodeCreator*>(Creator));
	Out.Node = Creator->CreateNode(bSelectNewNode);
	return Out;
}

FJavascriptNodeCreator UJavascriptGraphEditorLibrary::CustomNodeCreator(UJavascriptGraphEdGraph* Graph)
{
	FJavascriptNodeCreator Out;
	auto Creator = new FCustomJavascriptGraphNodeCreator(Graph);
	Out.Instance = MakeShareable(static_cast<IJavascriptGraphNodeCreator*>(Creator));
	Out.Node = Creator->CreateNode(false);
	return Out;
}

FJavascriptNodeCreator UJavascriptGraphEditorLibrary::CommentNodeCreator(UJavascriptGraphEdGraph* Graph, bool bSelectNewNode/* = true*/)
{
	FJavascriptNodeCreator Out;
	auto Creator = new FCommentJavascriptGraphNodeCreator(Graph);
	Out.Instance = MakeShareable(static_cast<IJavascriptGraphNodeCreator*>(Creator));
	Out.Node = Creator->CreateNode(bSelectNewNode);
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
	if (A.IsValid() && B.IsValid())
	{
		A->MakeLinkTo(B);
	}
}

bool UJavascriptGraphEditorLibrary::TryConnection(UEdGraphSchema* Schema, FJavascriptEdGraphPin A, FJavascriptEdGraphPin B)
{
	if (A.IsValid() && B.IsValid())
	{
		return Schema->TryCreateConnection(A, B);
	}
	return false;
}

void UJavascriptGraphEditorLibrary::BreakLinkTo(FJavascriptEdGraphPin A, FJavascriptEdGraphPin B)
{
	if (A.IsValid() && B.IsValid())
	{
		A->BreakLinkTo(B);
	}
}

void UJavascriptGraphEditorLibrary::BreakAllPinLinks(FJavascriptEdGraphPin A)
{
	if (A.IsValid())
	{
		A->BreakAllPinLinks(true);
	}
}

FEdGraphPinType UJavascriptGraphEditorLibrary::GetPinType(FJavascriptEdGraphPin A)
{
	FEdGraphPinType PinType;
	if (A.IsValid())
	{
		PinType = A->PinType;
	}

	return PinType;
}

EJavascriptPinContainerType::Type UJavascriptGraphEditorLibrary::GetPinContainerType(FJavascriptEdGraphPin A)
{
	EJavascriptPinContainerType::Type ContainerType = EJavascriptPinContainerType::Type::None;
	if (A.IsValid())
	{
		ContainerType = (EJavascriptPinContainerType::Type)(int32)(A->PinType.ContainerType);
	}

	return ContainerType;
}

void UJavascriptGraphEditorLibrary::SetPinType(FJavascriptEdGraphPin Pin, FEdGraphPinType PinType)
{
	if (Pin.IsValid())
	{
		Pin->PinType = PinType;
	}
}

void UJavascriptGraphEditorLibrary::SetPinContainerType(FJavascriptEdGraphPin Pin, EJavascriptPinContainerType::Type ContainerType)
{
	if (Pin.IsValid())
	{
		Pin->PinType.ContainerType = (EPinContainerType)(uint8)ContainerType;
	}
}

FJavascriptEdGraphPin UJavascriptGraphEditorLibrary::FindPin(UEdGraphNode* Node, const FString& PinName, EEdGraphPinDirection Direction)
{
	return FJavascriptEdGraphPin{ Node->FindPin(PinName, Direction) };
}

FName UJavascriptGraphEditorLibrary::GetPinName(FJavascriptEdGraphPin A)
{
	return A.IsValid() ? A->PinName : NAME_None;
}

void UJavascriptGraphEditorLibrary::SetPinInfo(FJavascriptEdGraphPin A, FName InPinName, FString InPinToolTip)
{
	if (A.IsValid())
	{
		A->PinName = InPinName;
		A->PinToolTip = InPinToolTip;
	}
}

FGuid UJavascriptGraphEditorLibrary::GetPinGUID(FJavascriptEdGraphPin A)
{
	FGuid GUID;
	if (A.IsValid())
	{
		GUID = A->PinId;
	}
	return GUID;
}

int32 UJavascriptGraphEditorLibrary::GetPinIndex(FJavascriptEdGraphPin A)
{
	UEdGraphNode* Node = A.IsValid() ? A->GetOwningNode() : nullptr;
	return (Node != nullptr) ? Node->GetPinIndex(A) : INDEX_NONE;
}

FJavascriptEdGraphPin UJavascriptGraphEditorLibrary::GetParentPin(FJavascriptEdGraphPin A)
{
	return { A.IsValid() ? A->ParentPin : nullptr };
}

TArray<FJavascriptEdGraphPin> UJavascriptGraphEditorLibrary::GetSubPins(FJavascriptEdGraphPin A)
{
	return A.IsValid() ? TransformPins(A->SubPins) : TArray<FJavascriptEdGraphPin>();
}

void UJavascriptGraphEditorLibrary::SetParentPin(FJavascriptEdGraphPin A, FJavascriptEdGraphPin Parent)
{
	if (A.IsValid())
	{
		if (A->ParentPin != nullptr)
		{
			return;
		}

		A->ParentPin = Parent;

		if (Parent.IsValid())
		{
			Parent->SubPins.AddUnique(A);
		}
	}
}

bool UJavascriptGraphEditorLibrary::IsValid(FJavascriptEdGraphPin A)
{
	return A.IsValid();
}

void UJavascriptGraphEditorLibrary::SetPinHidden(FJavascriptEdGraphPin A, bool bHidden)
{
	if (A.IsValid())
	{
		A->bHidden = bHidden;
	}
}

bool UJavascriptGraphEditorLibrary::IsPinHidden(FJavascriptEdGraphPin A)
{
	return (A.IsValid() && A->bHidden);
}

class UEdGraphNode* UJavascriptGraphEditorLibrary::GetOwningNode(FJavascriptEdGraphPin A)
{
	return A.IsValid() ? A->GetOwningNode() : nullptr;
}

EEdGraphPinDirection UJavascriptGraphEditorLibrary::GetDirection(FJavascriptEdGraphPin A)
{
	return A.IsValid() ? ((EEdGraphPinDirection)A->Direction) : EEdGraphPinDirection::EGPD_Input;
}

TArray<FJavascriptEdGraphPin> UJavascriptGraphEditorLibrary::GetLinkedTo(FJavascriptEdGraphPin A)
{
	return TransformPins(A->LinkedTo);
}

// Using GetLinkedTo to count number of linked pins cause unnecessary call to TransformPins.
int32 UJavascriptGraphEditorLibrary::GetLinkedPinNum(FJavascriptEdGraphPin A)
{
	return A->LinkedTo.Num();
}

TArray<FJavascriptEdGraphPin> UJavascriptGraphEditorLibrary::GetPins(UEdGraphNode* Node)
{
	return TransformPins(Node->Pins);
}

void UJavascriptGraphEditorLibrary::SetPinAdvancedView(FJavascriptEdGraphPin A, bool bAdvancedView)
{
	if (A.IsValid())
	{
		A->bAdvancedView = bAdvancedView;
	}
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
#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION > 22) || ENGINE_MAJOR_VERSION > 4
	TSharedPtr<SGraphPin>* SGraphPinHandle = Container.PinToPinWidgetMap->Find(Pin);
	if (SGraphPinHandle)
	{
		TSharedRef<SWidget> SWidgetHandle = static_cast<SWidget&>(*SGraphPinHandle->Get()).AsShared();
		Widget.Handle = &SWidgetHandle;
	}
#else
	TSharedRef<SGraphPin>* SGraphPinHandle = Container.PinToPinWidgetMap->Find(Pin);
	if (SGraphPinHandle)
	{
		TSharedRef<SWidget> SWidgetHandle = static_cast<SWidget&>(SGraphPinHandle->Get()).AsShared();
		Widget.Handle = &SWidgetHandle;
	}
#endif

	return Widget;
}
// @unused
FJavascriptArrangedWidget UJavascriptGraphEditorLibrary::GetArrangedNodes(FJavascriptDetermineLinkGeometryContainer Container, UEdGraphNode* Node)
{
	FJavascriptArrangedWidget Widget = FJavascriptArrangedWidget();

	int32* Index = Container.NodeWidgetMap->Find(Node);
	if (Index != nullptr)
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
		return DrawingPolicy->GetHoveredPins().Contains(Pin.Get());
	}

	return false;
}

void UJavascriptGraphEditorLibrary::ApplyHoverDeemphasis(FJavascriptGraphConnectionDrawingPolicyContainer Container, FJavascriptEdGraphPin OutputPin, FJavascriptEdGraphPin InputPin, float Thickness, FLinearColor WireColor)
{
	FJavascriptGraphConnectionDrawingPolicy* DrawingPolicy = Container.Handle;
	if (DrawingPolicy)
	{
		DrawingPolicy->ApplyHoverDeemphasis(OutputPin, InputPin, Thickness, WireColor);
	}
}

void UJavascriptGraphEditorLibrary::DetermineWiringStyle(FJavascriptGraphConnectionDrawingPolicyContainer Container, FJavascriptEdGraphPin OutputPin, FJavascriptEdGraphPin InputPin, FJavascriptConnectionParams& Params)
{
	FJavascriptGraphConnectionDrawingPolicy* DrawingPolicy = Container.Handle;
	if (DrawingPolicy)
	{
		FConnectionParams X = Params;
		DrawingPolicy->DetermineWiringStyle(OutputPin, InputPin, X);
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
	if (SlateEdNode && Pin.IsValid())
	{
		SlateEdNode->GetOwnerPanel()->AddPinToHoverSet(Pin);
	}
}

void UJavascriptGraphEditorLibrary::RemovePinFromHoverSet(const FJavascriptSlateEdNode& InSlateEdNode, FJavascriptEdGraphPin Pin)
{
	SJavascriptGraphEdNode* SlateEdNode = InSlateEdNode.Handle;
	if (SlateEdNode && Pin.IsValid())
	{
		SlateEdNode->GetOwnerPanel()->RemovePinFromHoverSet(Pin);
	}
}

void UJavascriptGraphEditorLibrary::ResizeNode(UEdGraphNode * Node, const FVector2D & NewSize)
{
	Node->ResizeNode(NewSize);
}

FJavascriptSlateWidget UJavascriptGraphEditorLibrary::GetOwnerPanel(UJavascriptGraphEdNode* Node)
{
	FJavascriptSlateWidget Out;

	TSharedPtr<SGraphNode> SlateNode = Node->GetNodeSlateWidget();
	if (SlateNode.IsValid())
	{
		Out.Widget = SlateNode->GetOwnerPanel();
	}

	return Out;
}

TArray<FJavascriptEdGraphPin> UJavascriptGraphEditorLibrary::TransformPins(const TArray<UEdGraphPin*>& Pins)
{
	TArray<FJavascriptEdGraphPin> Out;
	for (auto x : Pins)
	{
		Out.Add(FJavascriptEdGraphPin{ x });
	}
	return Out;
}

void UJavascriptGraphEditorLibrary::SetPinRefObject(FJavascriptEdGraphPin InPin, UObject* InObject)
{
	UEdGraphPin* GraphPin = InPin.Get();
	if (GraphPin)
	{
		UJavascriptGraphEdNode* EdNode = Cast<UJavascriptGraphEdNode>(GraphPin->GetOwningNode());
		if (EdNode)
		{
			EdNode->PinRefMap.Add(GraphPin->PinName, InObject);
		}
	}
}

UObject* UJavascriptGraphEditorLibrary::GetPinRefObject(FJavascriptEdGraphPin InPin)
{
	UEdGraphPin* GraphPin = InPin.Get();
	if (GraphPin)
	{
		UJavascriptGraphEdNode* EdNode = Cast<UJavascriptGraphEdNode>(GraphPin->GetOwningNode());
		if (EdNode)
		{
			return EdNode->PinRefMap.FindChecked(GraphPin->PinName);
		}
	}

	return nullptr;
}

#undef LOCTEXT_NAMESPACE
