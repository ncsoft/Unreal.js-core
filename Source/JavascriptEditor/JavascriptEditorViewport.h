#pragma once

#include "JavascriptEditorLibrary.h"
#include "JavascriptEditorViewport.generated.h"

class SAutoRefreshEditorViewport;

/**
 * 
 */
UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptEditorViewport : public UPanelWidget
{
	GENERATED_BODY()

public:	
#if WITH_EDITOR
	virtual TSharedRef<SWidget> RebuildWidget();

	TSharedPtr<class SAutoRefreshEditorViewport> ViewportWidget;
#endif

	/** Delegate for constructing a UWidget based on a UObject */
	DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnViewportClick, const FJavascriptViewportClick&, ViewportClick, UJavascriptEditorViewport*, Instance);
	DECLARE_DYNAMIC_DELEGATE_FourParams(FOnViewportTrackingStarted, const FJavascriptInputEventState&, InputState, bool, bIsDraggingWidget, bool, bNudge, UJavascriptEditorViewport*, Instance);
	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnViewportTrackingStopped, UJavascriptEditorViewport*, Instance);
	DECLARE_DYNAMIC_DELEGATE_RetVal_FourParams(bool, FOnInputWidgetDelta, FVector&, Drag, FRotator&, Rot, FVector&, Scale, UJavascriptEditorViewport*, Instance);
	DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnViewportDraw, const FJavascriptPDI&, PDI, UJavascriptEditorViewport*, Instance);

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnViewportClick OnClick;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnViewportTrackingStarted OnTrackingStarted;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnViewportTrackingStopped OnTrackingStopped;	

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnInputWidgetDelta OnInputWidgetDelta;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnViewportDraw OnDraw;

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	UWorld* GetViewportWorld() const;

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	void Redraw();

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	void OverridePostProcessSettings(const FPostProcessSettings& PostProcessSettings, float Weight);

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	void SetBackgroundColor(const FLinearColor& BackgroundColor);

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	void SetRealtime(bool bInRealtime, bool bStoreCurrentValue);

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	void SetViewLocation(const FVector& ViewLocation);

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	void SetViewRotation(const FRotator& ViewRotation);

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	void RestoreRealtime(bool bAllowDisable);

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	void SetLightDirection(const FRotator& InLightDir);

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	void SetLightBrightness(float LightBrightness);

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	void SetLightColor(const FColor& LightColor);

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	void SetSkyBrightness(float SkyBrightness);

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	void SetSimulatePhysics(bool bShouldSimulatePhysics);

	// UPanelWidget
	virtual UClass* GetSlotClass() const override;
	virtual void OnSlotAdded(UPanelSlot* Slot) override;
	virtual void OnSlotRemoved(UPanelSlot* Slot) override;
	// End UPanelWidget
};
