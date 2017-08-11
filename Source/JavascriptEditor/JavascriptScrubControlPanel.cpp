// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "JavascriptScrubControlPanel.h"
#include "SScrubControlPanel.h"

#define LOCTEXT_NAMESPACE "JavascriptScrubControlPanel"

/////////////////////////////////////////////////////
// UJavascriptScrubControlPanel

UJavascriptScrubControlPanel::UJavascriptScrubControlPanel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AccumulatedTime = 0.0f;
	FramesPerSecond = 1.0f;
	SumFrames = 1;

	bLooping = true;

	ViewInputMin = 0.0f;
	ViewInputMax = 1.0f;
	LastObservedSequenceLength = ViewInputMax;
}

void UJavascriptScrubControlPanel::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	ScrubControlPanelWidget.Reset();
}

TSharedRef<SWidget> UJavascriptScrubControlPanel::RebuildWidget()
{
	if (IsDesignTime())
	{
		return RebuildDesignWidget(SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("JavascriptScrubControlPanel", "JavascriptScrubControlPanel"))
			]);
	}
	else
	{
		ScrubControlPanelWidget = SNew(SScrubControlPanel)
			.IsEnabled(true)
			.Value(BIND_UOBJECT_ATTRIBUTE(float, GetPlaybackPosition))
			.NumOfKeys(BIND_UOBJECT_ATTRIBUTE(uint32, GetTotalFrameCount))
			.OnValueChanged(BIND_UOBJECT_DELEGATE(FOnFloatValueChanged, SetPlaybackPosition))
			.SequenceLength(BIND_UOBJECT_ATTRIBUTE(float, GetTotalSequenceLength))
				// .OnBeginSliderMovement(this, &SAnimationScrubPanel::OnBeginSliderMovement)
				// .OnEndSliderMovement(this, &SAnimationScrubPanel::OnEndSliderMovement)
			.OnClickedForwardPlay(BIND_UOBJECT_DELEGATE(FOnClicked, OnClick_Forward))
			.OnClickedForwardStep(BIND_UOBJECT_DELEGATE(FOnClicked, OnClick_Forward_Step))
			.OnClickedForwardEnd(BIND_UOBJECT_DELEGATE(FOnClicked, OnClick_Forward_End))
			.OnClickedBackwardPlay(BIND_UOBJECT_DELEGATE(FOnClicked, OnClick_Backward))
			.OnClickedBackwardStep(BIND_UOBJECT_DELEGATE(FOnClicked, OnClick_Backward_Step))
			.OnClickedBackwardEnd(BIND_UOBJECT_DELEGATE(FOnClicked, OnClick_Backward_End))
			.OnClickedToggleLoop(BIND_UOBJECT_DELEGATE(FOnClicked, OnClick_ToggleLoop))
			.OnGetLooping(BIND_UOBJECT_DELEGATE(FOnGetLooping, IsLooping))
				// .OnGetPlaybackMode(BIND_UOBJECT_DELEGATE(FOnGetPlaybackMode, GetPlaybackMode))
			.ViewInputMin(BIND_UOBJECT_ATTRIBUTE(float, GetViewRangeMin))
			.ViewInputMax(BIND_UOBJECT_ATTRIBUTE(float, GetViewRangeMax))
			.OnSetInputViewRange(BIND_UOBJECT_DELEGATE(FOnSetInputViewRange, SetViewRange))
			.bAllowZoom(true)
			.IsRealtimeStreamingMode(false);
		
		return ScrubControlPanelWidget.ToSharedRef();
	}
}

void UJavascriptScrubControlPanel::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (ScrubControlPanelWidget.IsValid())
	{

	}
}

void UJavascriptScrubControlPanel::SetPlaybackPosition(float NewTime)
{
	NewTime = FMath::Clamp<float>(NewTime, 0.0f, GetTotalSequenceLength());

	AccumulatedTime = NewTime;

	SetPlaybackPosition_Delegate.Broadcast();
}

uint32 UJavascriptScrubControlPanel::GetTotalFrameCount() const
{
	return SumFrames + 1;
}

float UJavascriptScrubControlPanel::GetTotalSequenceLength() const
{
	if (FramesPerSecond != 0)
	{
		return SumFrames / FramesPerSecond;
	}

	return 0.0f;
}

float UJavascriptScrubControlPanel::GetPlaybackPosition() const
{
	return AccumulatedTime;
}

bool UJavascriptScrubControlPanel::IsLooping() const
{
	return bLooping;
}

float UJavascriptScrubControlPanel::GetViewRangeMin() const
{
	 return ViewInputMin;
}

float UJavascriptScrubControlPanel::GetViewRangeMax() const
{
	const float SequenceLength = GetTotalSequenceLength();
	if (SequenceLength != LastObservedSequenceLength)
	{
		LastObservedSequenceLength = SequenceLength;
		ViewInputMin = 0.0f;
		ViewInputMax = SequenceLength;
	}

	return ViewInputMax;
}

void UJavascriptScrubControlPanel::SetViewRange(float NewMin, float NewMax)
{
	ViewInputMin = FMath::Max<float>(NewMin, 0.0f);
	ViewInputMax = FMath::Min<float>(NewMax, GetTotalSequenceLength());
}

FReply UJavascriptScrubControlPanel::OnClick_Forward()
{
	OnClick_Forward_Delegate.Broadcast();

	return FReply::Handled();
}
FReply UJavascriptScrubControlPanel::OnClick_Forward_Step()
{
	OnClick_Forward_Step_Delegate.Broadcast();

	return FReply::Handled();
}
FReply UJavascriptScrubControlPanel::OnClick_Forward_End()
{
	OnClick_Forward_End_Delegate.Broadcast();

	return FReply::Handled();
}
FReply UJavascriptScrubControlPanel::OnClick_Backward()
{
	OnClick_Backward_Delegate.Broadcast();

	return FReply::Handled();
}
FReply UJavascriptScrubControlPanel::OnClick_Backward_Step()
{
	OnClick_Backward_Step_Delegate.Broadcast();

	return FReply::Handled();
}
FReply UJavascriptScrubControlPanel::OnClick_Backward_End()
{
	OnClick_Backward_End_Delegate.Broadcast();

	return FReply::Handled();
}
FReply UJavascriptScrubControlPanel::OnClick_ToggleLoop()
{
	OnClick_ToggleLoop_Delegate.Broadcast();

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE