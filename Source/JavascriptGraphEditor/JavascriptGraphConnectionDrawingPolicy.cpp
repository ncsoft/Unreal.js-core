#include "JavascriptGraphEditorPrivatePCH.h"
#include "JavascriptGraphConnectionDrawingPolicy.h"
#include "JavascriptGraphAssetGraphSchema.h"

FJavascriptGraphConnectionDrawingPolicy::FJavascriptGraphConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float ZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj)
	: FConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, ZoomFactor, InClippingRect, InDrawElements)
	, GraphObj(InGraphObj)
{
}

void FJavascriptGraphConnectionDrawingPolicy::DetermineWiringStyle(UEdGraphPin* OutputPin, UEdGraphPin* InputPin, /*inout*/ FConnectionParams& Params)
{
	auto Schema = Cast<UJavascriptGraphAssetGraphSchema>(GraphObj->GetSchema());
	if (Schema->OnDetermineWiringStyle.IsBound())
	{
		FJavascriptConnectionParams X = Params;
		Schema->OnDetermineWiringStyle.Execute(OutputPin, InputPin, X);
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
		Schema->OnDrawPreviewConnector.Execute(PinGeometry, StartPoint, EndPoint, Pin);
	}
	else
	{
		FConnectionDrawingPolicy::DrawPreviewConnector(PinGeometry, StartPoint, EndPoint, Pin);
	}
}

void FJavascriptGraphConnectionDrawingPolicy::DrawSplineWithArrow(const FVector2D& StartAnchorPoint, const FVector2D& EndAnchorPoint, const FConnectionParams& Params)
{
	auto Schema = Cast<UJavascriptGraphAssetGraphSchema>(GraphObj->GetSchema());
	if (Schema->OnDrawSplineWithArrow.IsBound())
	{
		Schema->OnDrawSplineWithArrow.Execute(StartAnchorPoint, EndAnchorPoint, Params);
	}
	else
	{
		FConnectionDrawingPolicy::DrawSplineWithArrow(StartAnchorPoint, EndAnchorPoint, Params);
	}
}

void FJavascriptGraphConnectionDrawingPolicy::DrawSplineWithArrow(const FGeometry& StartGeom, const FGeometry& EndGeom, const FConnectionParams& Params)
{
	auto Schema = Cast<UJavascriptGraphAssetGraphSchema>(GraphObj->GetSchema());
	if (Schema->OnDrawSplineWithArrow_Geom.IsBound())
	{
		Schema->OnDrawSplineWithArrow_Geom.Execute(StartGeom, EndGeom, Params);
	}
	else
	{
		FConnectionDrawingPolicy::DrawSplineWithArrow(StartGeom, EndGeom, Params);
	}
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