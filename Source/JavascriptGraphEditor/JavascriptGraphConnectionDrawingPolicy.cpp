#include "JavascriptGraphConnectionDrawingPolicy.h"
#include "JavascriptGraphAssetGraphSchema.h"
#include "EdGraph/EdGraph.h"
#include "Rendering/DrawElements.h"

FJavascriptGraphConnectionDrawingPolicy::FJavascriptGraphConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj)
	: FConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements)
	, GraphObj(InGraphObj)
{
}

FJavascriptGraphConnectionDrawingPolicy::~FJavascriptGraphConnectionDrawingPolicy()
{
	NodeWidgetMap.Empty();
	GraphObj = nullptr;
}

void FJavascriptGraphConnectionDrawingPolicy::DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin, /*inout*/ FConnectionParams& Params)
{
	auto Schema = Cast<UJavascriptGraphAssetGraphSchema>(GraphObj->GetSchema());
	if (Schema->OnDetermineWiringStyle.IsBound())
	{
		FJavascriptConnectionParams X = Params;
		Schema->OnDetermineWiringStyle.Execute(OutputPin, InputPin, X, FJavascriptGraphConnectionDrawingPolicyContainer{ this });
		Params = X;
	}
	else
	{
		FConnectionDrawingPolicy::DetermineWiringStyle(OutputPin, InputPin, Params);
	}
}

void FJavascriptGraphConnectionDrawingPolicy::DrawPreviewConnector(const FGeometry& PinGeometry, const FVector2D& StartPoint, const FVector2D& EndPoint, UEdGraphPin* Pin)
{
	auto Schema = Cast<UJavascriptGraphAssetGraphSchema>(GraphObj->GetSchema());
	if (Schema->OnDrawPreviewConnector.IsBound())
	{
		FConnectionParams Params;
		FJavascriptConnectionParams X = Params;
		if (Schema->OnDrawPreviewConnector.Execute(PinGeometry, StartPoint, EndPoint, FJavascriptEdGraphPin{ const_cast<UEdGraphPin*>(Pin) }, X, FJavascriptGraphConnectionDrawingPolicyContainer{ this }))
		{
			return;
		}
	}

	FConnectionDrawingPolicy::DrawPreviewConnector(PinGeometry, StartPoint, EndPoint, Pin);	
}

void FJavascriptGraphConnectionDrawingPolicy::DrawSplineWithArrow(const FVector2D& StartAnchorPoint, const FVector2D& EndAnchorPoint, const FConnectionParams& Params)
{
	auto Schema = Cast<UJavascriptGraphAssetGraphSchema>(GraphObj->GetSchema());
	if (Schema->OnDrawSplineWithArrow.IsBound())
	{
		if (Schema->OnDrawSplineWithArrow.Execute(StartAnchorPoint, EndAnchorPoint, Params, FJavascriptGraphConnectionDrawingPolicyContainer{ this }, ArrowRadius))
		{
			return;
		}
	}

	FConnectionDrawingPolicy::DrawSplineWithArrow(StartAnchorPoint, EndAnchorPoint, Params);
}

void FJavascriptGraphConnectionDrawingPolicy::DrawSplineWithArrow(const FGeometry& StartGeom, const FGeometry& EndGeom, const FConnectionParams& Params)
{
	auto Schema = Cast<UJavascriptGraphAssetGraphSchema>(GraphObj->GetSchema());
	if (Schema->OnDrawSplineWithArrow_Geom.IsBound())
	{
		if (Schema->OnDrawSplineWithArrow_Geom.Execute(StartGeom, EndGeom, Params, FJavascriptGraphConnectionDrawingPolicyContainer{ this }))
		{
			return;
		}
	}
	
	FConnectionDrawingPolicy::DrawSplineWithArrow(StartGeom, EndGeom, Params);
}

FVector2D FJavascriptGraphConnectionDrawingPolicy::ComputeSplineTangent(const FVector2D& Start, const FVector2D& End) const
{
	auto Schema = Cast<UJavascriptGraphAssetGraphSchema>(GraphObj->GetSchema());
	if (Schema->OnComputeSplineTangent.IsBound())
	{
		return Schema->OnComputeSplineTangent.Execute(Start, End);
	}
	else
	{
		return FConnectionDrawingPolicy::ComputeSplineTangent(Start, End);
	}
}

void FJavascriptGraphConnectionDrawingPolicy::Draw(TMap<TSharedRef<SWidget>, FArrangedWidget>& InPinGeometries, FArrangedChildren& ArrangedNodes)
{
	auto Schema = Cast<UJavascriptGraphAssetGraphSchema>(GraphObj->GetSchema());
	if (Schema->OnUsingNodeWidgetMap.IsBound())
	{
		bool bUsing = Schema->OnUsingNodeWidgetMap.Execute();
		if (bUsing)
		{
			NodeWidgetMap.Empty();
			for (int32 NodeIndex = 0; NodeIndex < ArrangedNodes.Num(); ++NodeIndex)
			{
				FArrangedWidget& CurWidget = ArrangedNodes[NodeIndex];
				TSharedRef<SGraphNode> ChildNode = StaticCastSharedRef<SGraphNode>(CurWidget.Widget);
				NodeWidgetMap.Add(ChildNode->GetNodeObj(), NodeIndex);
			}
		}
	}

	FConnectionDrawingPolicy::Draw(InPinGeometries, ArrangedNodes);
}

void FJavascriptGraphConnectionDrawingPolicy::DetermineLinkGeometry(
	FArrangedChildren& ArrangedNodes,
	TSharedRef<SWidget>& OutputPinWidget,
	UEdGraphPin* OutputPin,
	UEdGraphPin* InputPin,
	/*out*/ FArrangedWidget*& StartWidgetGeometry,
	/*out*/ FArrangedWidget*& EndWidgetGeometry
	)
{
	auto Schema = Cast<UJavascriptGraphAssetGraphSchema>(GraphObj->GetSchema());
	
	if (Schema->OnDetermineLinkGeometry.IsBound())
	{
		FJavascriptArrangedWidget Start = FJavascriptArrangedWidget{ StartWidgetGeometry };
		FJavascriptArrangedWidget End = FJavascriptArrangedWidget{ EndWidgetGeometry };

		if (Schema->OnDetermineLinkGeometry.Execute(
			FJavascriptEdGraphPin{ const_cast<UEdGraphPin*>(OutputPin) },
			FJavascriptEdGraphPin{ const_cast<UEdGraphPin*>(InputPin) },
			Start,
			End,
			FJavascriptDetermineLinkGeometryContainer{ &ArrangedNodes, &OutputPinWidget, &NodeWidgetMap, PinGeometries, &PinToPinWidgetMap }
			))
		{
			StartWidgetGeometry = Start.Handle;
			EndWidgetGeometry = End.Handle;
			return;
		}			
	}
	
	FConnectionDrawingPolicy::DetermineLinkGeometry(ArrangedNodes, OutputPinWidget, OutputPin, InputPin, StartWidgetGeometry, EndWidgetGeometry);
}

void FJavascriptGraphConnectionDrawingPolicy::MakeRotatedBox(FVector2D ArrowDrawPos, float AngleInRadians, FLinearColor WireColor) {
	FSlateDrawElement::MakeRotatedBox(
		DrawElementsList,
		ArrowLayerID,
		FPaintGeometry(ArrowDrawPos, ArrowImage->ImageSize * ZoomFactor, ZoomFactor),
		ArrowImage,
		ESlateDrawEffect::None,
		AngleInRadians,
		TOptional<FVector2D>(),
		FSlateDrawElement::RelativeToElement,
		WireColor
		);
};