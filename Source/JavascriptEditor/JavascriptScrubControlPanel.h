// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/Widget.h"
#include "JavascriptScrubControlPanel.generated.h"

/**
*
*/
UCLASS(Experimental)
class JAVASCRIPTEDITOR_API UJavascriptScrubControlPanel : public UWidget
{
	GENERATED_UCLASS_BODY()
	
public:

	//~ Begin UWidget interface
	virtual void SynchronizeProperties() override;
	// End UWidget interface

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
protected:

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClick_Forward);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClick_Forward_Step);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClick_Forward_End);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClick_Backward);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClick_Backward_Step);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClick_Backward_End);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnClick_ToggleLoop);

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSetPlaybackPosition);

public:

	UPROPERTY(EditAnywhere, Category = "ScrubControlPanel")
	FOnClick_Forward OnClick_Forward_Delegate;
	UPROPERTY(EditAnywhere, Category = "ScrubControlPanel")
	FOnClick_Forward_Step OnClick_Forward_Step_Delegate;
	UPROPERTY(EditAnywhere, Category = "ScrubControlPanel")
	FOnClick_Forward_End OnClick_Forward_End_Delegate;
	UPROPERTY(EditAnywhere, Category = "ScrubControlPanel")
	FOnClick_Backward OnClick_Backward_Delegate;
	UPROPERTY(EditAnywhere, Category = "ScrubControlPanel")
	FOnClick_Backward_Step OnClick_Backward_Step_Delegate;
	UPROPERTY(EditAnywhere, Category = "ScrubControlPanel")
	FOnClick_Backward_End OnClick_Backward_End_Delegate;
	UPROPERTY(EditAnywhere, Category = "ScrubControlPanel")
	FOnClick_ToggleLoop OnClick_ToggleLoop_Delegate;

	UPROPERTY(EditAnywhere, Category = "ScrubControlPanel")
	FSetPlaybackPosition SetPlaybackPosition_Delegate;

protected:
	float AccumulatedTime;
	float FramesPerSecond;
	int32 SumFrames;
	uint32 bLooping : 1;

protected:
	mutable float ViewInputMin;
	mutable float ViewInputMax;
	mutable float LastObservedSequenceLength;

protected:
	TSharedPtr<class SScrubControlPanel> ScrubControlPanelWidget;

public:
	UFUNCTION(BlueprintCallable, Category = "ScrubControlPanel")
	float GetPlaybackPosition() const;
	UFUNCTION(BlueprintCallable, Category = "ScrubControlPanel")
	float GetFramesPerSecond() const
	{
		return FramesPerSecond;
	}
	UFUNCTION(BlueprintCallable, Category = "ScrubControlPanel")
	float GetTotalSequenceLength() const;
	UFUNCTION(BlueprintCallable, Category = "ScrubControlPanel")
	bool IsLooping() const;

	UFUNCTION(BlueprintCallable, Category = "ScrubControlPanel")
	float GetViewRangeMin() const;
	UFUNCTION(BlueprintCallable, Category = "ScrubControlPanel")
	float GetViewRangeMax() const;

	UFUNCTION(BlueprintCallable, Category = "ScrubControlPanel")
	void SetPlaybackPosition(float NewTime);
	UFUNCTION(BlueprintCallable, Category = "ScrubControlPanel")
	void SetFramesPerSecond(float NewFramesPerSecond)
	{
		FramesPerSecond = NewFramesPerSecond;
	}
	UFUNCTION(BlueprintCallable, Category = "ScrubControlPanel")
	void SetSumFrames(float NewSumFrames)
	{
		SumFrames = NewSumFrames;
	}
	UFUNCTION(BlueprintCallable, Category = "ScrubControlPanel")
	void SetLooping(bool NewbLooping)
	{
		bLooping = NewbLooping;
	}

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface

	FReply OnClick_Forward();
	FReply OnClick_Forward_Step();
	FReply OnClick_Forward_End();
	FReply OnClick_Backward();
	FReply OnClick_Backward_Step();
	FReply OnClick_Backward_End();
	FReply OnClick_ToggleLoop();

protected:
	UFUNCTION()
	uint32 GetTotalFrameCount() const;
	UFUNCTION()
	void SetViewRange(float NewMin, float NewMax);
};
