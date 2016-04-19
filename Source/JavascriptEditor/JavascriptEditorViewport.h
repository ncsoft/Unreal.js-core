#pragma once

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
