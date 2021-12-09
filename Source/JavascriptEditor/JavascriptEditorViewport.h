#pragma once

#include "JavascriptEditorLibrary.h"
#include "Components/PanelWidget.h"
#include "JavascriptEditorViewport.generated.h"

class SAutoRefreshEditorViewport;
class UAssetViewerSettings;

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
	DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnViewportClick, const FJavascriptViewportClick&, ViewportClick, const FJavascriptHitProxy&, HitProxy, UJavascriptEditorViewport*, Instance);
	DECLARE_DYNAMIC_DELEGATE_FourParams(FOnViewportTrackingStarted, const FJavascriptInputEventState&, InputState, bool, bIsDraggingWidget, bool, bNudge, UJavascriptEditorViewport*, Instance);
	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnViewportTrackingStopped, UJavascriptEditorViewport*, Instance);
	DECLARE_DYNAMIC_DELEGATE_RetVal_FourParams(bool, FOnInputWidgetDelta, FVector&, Drag, FRotator&, Rot, FVector&, Scale, UJavascriptEditorViewport*, Instance);
	DECLARE_DYNAMIC_DELEGATE_RetVal_FourParams(bool, FOnInputKey, int32, ControllerId, FKey, Key, EInputEvent, Event, UJavascriptEditorViewport*, Instance);
	DECLARE_DYNAMIC_DELEGATE_RetVal_FiveParams(bool, FOnInputAxis, int32, ControllerId, FKey, Key, float, Delta, float, DeltaTime, UJavascriptEditorViewport*, Instance);
	DECLARE_DYNAMIC_DELEGATE_RetVal_ThreeParams(bool, FOnMouseEnter, int32, x, int32, y, UJavascriptEditorViewport*, Instance);
	DECLARE_DYNAMIC_DELEGATE_RetVal_ThreeParams(bool, FOnMouseMove, int32, x, int32, y, UJavascriptEditorViewport*, Instance);
	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FOnMouseLeave, UJavascriptEditorViewport*, Instance);
	DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnViewportDraw, const FJavascriptPDI&, PDI, UJavascriptEditorViewport*, Instance);
    DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnViewportDrawCanvas, UCanvas*, Canvas, UJavascriptEditorViewport*, Instance);
	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(FVector, FOnGetWidgetLocation, UJavascriptEditorViewport*, Instance);
	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(FRotator, FOnGetWidgetRotation, UJavascriptEditorViewport*, Instance);
	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(EJavascriptWidgetMode, FOnGetWidgetMode, UJavascriptEditorViewport*, Instance);

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnViewportClick OnClick;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnViewportTrackingStarted OnTrackingStarted;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnViewportTrackingStopped OnTrackingStopped;	

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnInputWidgetDelta OnInputWidgetDelta;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnInputKey OnInputKey;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnInputAxis OnInputAxis;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnMouseEnter OnMouseEnter;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnMouseMove OnMouseMove;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnMouseLeave OnMouseLeave;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnViewportDraw OnDraw;

    UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
    FOnViewportDrawCanvas OnDrawCanvas;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGetWidgetLocation OnGetWidgetLocation;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGetWidgetRotation OnGetWidgetRotation;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGetWidgetMode OnGetWidgetMode;
    
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
	FVector GetViewLocation();

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	FRotator GetViewRotation();

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	void SetViewFOV(float InViewFOV);

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	float GetViewFOV();

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	void SetCameraSpeedSetting(int32 SpeedSetting);

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	int32 GetCameraSpeedSetting();

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	void SetLightLocation(const FVector& InLightPos);

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

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	void SetWidgetMode(EJavascriptWidgetMode WidgetMode);
    
    UFUNCTION(BlueprintCallable, Category = "Viewport")
    EJavascriptWidgetMode GetWidgetMode();
    
	UFUNCTION(BlueprintCallable, Category = "Viewport")
	FString GetEngineShowFlags();

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	bool SetEngineShowFlags(const FString& In);

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	void SetProfileIndex(const int32 InProfileIndex);

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	int32 GetCurrentProfileIndex();

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	UAssetViewerSettings* GetDefaultAssetViewerSettings();

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	void SetFloorOffset(const float InFloorOffset);

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	UStaticMeshComponent* GetFloorMeshComponent();

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	UStaticMeshComponent* GetSkyComponent();

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	void SetViewportType(ELevelViewportType InViewportType);

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	void SetViewMode(EViewModeIndex InViewModeIndex);

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	void DeprojectScreenToWorld(const FVector2D &ScreenPosition, FVector &OutRayOrigin, FVector& OutRayDirection);

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	void ProjectWorldToScreen(const FVector &WorldPosition, FVector2D &OutScreenPosition);

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	UDirectionalLightComponent* GetDefaultDirectionalLightComponent();

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	class USkyLightComponent* GetDefaultSkyLightComponent();

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	class UStaticMeshComponent* GetDefaultSkySphereComponent();

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	class USphereReflectionCaptureComponent* GetDefaultSphereReflectionComponent();

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	class UMaterialInstanceConstant* GetDefaultInstancedSkyMaterial();

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	class UPostProcessComponent* GetDefaultPostProcessComponent();

	// UPanelWidget
	virtual UClass* GetSlotClass() const override;
	virtual void OnSlotAdded(UPanelSlot* Slot) override;
	virtual void OnSlotRemoved(UPanelSlot* Slot) override;
	// End UPanelWidget

	//UVisual interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ End UVisual Interface
};
