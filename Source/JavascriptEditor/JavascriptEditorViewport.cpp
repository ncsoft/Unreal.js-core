#include "JavascriptEditorViewport.h"
#include "SEditorViewport.h"
#include "AdvancedPreviewScene.h"
#include "Runtime/Engine/Public/Slate/SceneViewport.h"
#include "Engine/Canvas.h"
#include "OverlaySlot.h"
#include "AssetViewerSettings.h"
#include "Modules/ModuleVersion.h"

#define LOCTEXT_NAMESPACE "JavascriptEditor"

PRAGMA_DISABLE_SHADOW_VARIABLE_WARNINGS


class FCanvasOwner : public FGCObject
{
    
public:
    FCanvasOwner(): FGCObject()
    {
        Canvas = nullptr;
    }
    
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override
    {
        Collector.AddReferencedObject(Canvas);
    }
    
public:
    UCanvas* Canvas;
};

#if WITH_EDITOR
class FJavascriptEditorViewportClient : public FEditorViewportClient
{
public:
	TWeakObjectPtr<UJavascriptEditorViewport> Widget;
	
	/** Constructor */
	explicit FJavascriptEditorViewportClient(FAdvancedPreviewScene& InPreviewScene, const TWeakPtr<class SEditorViewport>& InEditorViewportWidget = nullptr, TWeakObjectPtr<UJavascriptEditorViewport> InWidget = nullptr)
		: FEditorViewportClient(nullptr,&InPreviewScene,InEditorViewportWidget), Widget(InWidget), BackgroundColor(FColor(55,55,55))
	{
	}
	
	virtual void ProcessClick(class FSceneView& View, class HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY) override
	{
		if (Widget.IsValid() && Widget->OnClick.IsBound())
		{
			FJavascriptHitProxy Proxy;
			Proxy.HitProxy = HitProxy;
			FViewportClick Click(&View, this, Key, Event, HitX, HitY);
			Widget->OnClick.Execute(FJavascriptViewportClick(&Click), Proxy, Widget.Get());
		}
	}

	virtual void TrackingStarted(const struct FInputEventState& InInputState, bool bIsDraggingWidget, bool bNudge) override
	{
		if (Widget.IsValid() && Widget->OnTrackingStarted.IsBound())
		{
			Widget->OnTrackingStarted.Execute(FJavascriptInputEventState(InInputState), bIsDraggingWidget, bNudge, Widget.Get());
		}
	}

	virtual void TrackingStopped() override 
	{
		if (Widget.IsValid() && Widget->OnTrackingStopped.IsBound())
		{
			Widget->OnTrackingStopped.Execute(Widget.Get());
		}
	}

	virtual bool InputKey(FViewport* Viewport, int32 ControllerId, FKey Key, EInputEvent Event, float AmountDepressed = 1.f, bool bGamepad = false)
	{
		FEditorViewportClient::InputKey(Viewport, ControllerId, Key, Event, AmountDepressed, bGamepad);
		if (Widget.IsValid() && Widget->OnInputKey.IsBound())
		{
			return Widget->OnInputKey.Execute(ControllerId, Key, Event, Widget.Get());
		}
		else
		{
			return false;
		}
	}

	virtual bool InputAxis(FViewport* Viewport, int32 ControllerId, FKey Key, float Delta, float DeltaTime, int32 NumSamples = 1, bool bGamepad = false) override
	{
		FEditorViewportClient::InputAxis(Viewport, ControllerId, Key, Delta, DeltaTime, NumSamples, bGamepad);
		if (Widget.IsValid() && Widget->OnInputAxis.IsBound())
		{
			return Widget->OnInputAxis.Execute(ControllerId, Key, Delta, DeltaTime, Widget.Get());
		}
		else
		{
			return false;
		}
	}

	virtual void MouseEnter(FViewport* Viewport, int32 x, int32 y) override
	{

		FEditorViewportClient::MouseEnter(Viewport, x, y);
		if (Widget.IsValid() && Widget->OnMouseEnter.IsBound())
		{
			Widget->OnMouseEnter.Execute(x, y, Widget.Get());
		}
	}

	virtual void MouseMove(FViewport* Viewport, int32 x, int32 y) override
	{
		FEditorViewportClient::MouseMove(Viewport, x, y);
		if (Widget.IsValid() && Widget->OnMouseMove.IsBound())
		{
			Widget->OnMouseMove.Execute(x, y, Widget.Get());
		}
	}

	virtual void MouseLeave(FViewport* Viewport) override
	{
		FEditorViewportClient::MouseLeave(Viewport);
		if (Widget.IsValid() && Widget->OnMouseLeave.IsBound())
		{
			Widget->OnMouseLeave.Execute(Widget.Get());
		}
	}

	virtual bool InputWidgetDelta(FViewport* InViewport, EAxisList::Type CurrentAxis, FVector& Drag, FRotator& Rot, FVector& Scale) override
	{
		if (Widget.IsValid() && Widget->OnInputWidgetDelta.IsBound())
		{
			return Widget->OnInputWidgetDelta.Execute(Drag,Rot,Scale,Widget.Get());
		}
		else
		{
			return false;
		}
	}

	virtual void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override
	{
		FEditorViewportClient::Draw(View, PDI);

		if (Widget.IsValid() && Widget->OnDraw.IsBound())
		{
			Widget->OnDraw.Execute(FJavascriptPDI(PDI),Widget.Get());
		}
	}
    
    virtual void DrawCanvas(FViewport& InViewport, FSceneView& View, FCanvas& Canvas) override
    {
        FEditorViewportClient::DrawCanvas(InViewport, View, Canvas);
        
        if (Widget.IsValid() && Widget->OnDrawCanvas.IsBound())
        {
            if(CanvasOwner.Canvas == nullptr){
                CanvasOwner.Canvas = NewObject<UCanvas>(Widget.Get());
            }
            
            CanvasOwner.Canvas->Canvas = &Canvas;
            CanvasOwner.Canvas->Init(View.UnscaledViewRect.Width(), View.UnscaledViewRect.Height(), const_cast<FSceneView*>(&View), &Canvas);
            CanvasOwner.Canvas->ApplySafeZoneTransform();
            
            Widget->OnDrawCanvas.Execute(CanvasOwner.Canvas, Widget.Get());
            
            CanvasOwner.Canvas->PopSafeZoneTransform();
        }
    }

	virtual FWidget::EWidgetMode GetWidgetMode() const override
	{
		if (Widget.IsValid() && Widget->OnGetWidgetMode.IsBound())
		{
			return (FWidget::EWidgetMode)Widget->OnGetWidgetMode.Execute(Widget.Get());
		}
		else
		{
			return FEditorViewportClient::GetWidgetMode();
		}		
	}	

	virtual FVector GetWidgetLocation() const override
	{
		if (Widget.IsValid() && Widget->OnGetWidgetLocation.IsBound())
		{
			return Widget->OnGetWidgetLocation.Execute(Widget.Get());
		}
		else
		{
			return FEditorViewportClient::GetWidgetLocation();
		}
	}
	
	virtual FMatrix GetWidgetCoordSystem() const override
	{
		if (Widget.IsValid() && Widget->OnGetWidgetRotation.IsBound())
		{
			return FRotationMatrix(Widget->OnGetWidgetRotation.Execute(Widget.Get()));
		}
		else
		{
			return FEditorViewportClient::GetWidgetCoordSystem();
		}
	}

	virtual FLinearColor GetBackgroundColor() const
	{
		return BackgroundColor;
	}

	virtual void OverridePostProcessSettings(FSceneView& View) override 
	{
		View.OverridePostProcessSettings(PostProcessSettings, PostProcessSettingsWeight);
	}

	virtual void Tick(float InDeltaTime) override
	{
		FEditorViewportClient::Tick(InDeltaTime);

		if (!GIntraFrameDebuggingGameThread)
		{
			// Begin Play
			if (!PreviewScene->GetWorld()->bBegunPlay)
			{
				for (FActorIterator It(PreviewScene->GetWorld()); It; ++It)
				{
#if ENGINE_MINOR_VERSION > 14
					It->DispatchBeginPlay();
#else
					if (It->HasActorBegunPlay() == false)
					{
						It->BeginPlay();
					}
#endif
				}
				PreviewScene->GetWorld()->bBegunPlay = true;
			}

			// Tick
			PreviewScene->GetWorld()->Tick(LEVELTICK_All, InDeltaTime);
		}
	}

	FPostProcessSettings PostProcessSettings;
	float PostProcessSettingsWeight;
	FLinearColor BackgroundColor;

private:
    FCanvasOwner CanvasOwner;
};

class SAutoRefreshEditorViewport : public SEditorViewport
{
	SLATE_BEGIN_ARGS(SAutoRefreshEditorViewport)
	{}
		SLATE_ARGUMENT(TWeakObjectPtr<UJavascriptEditorViewport>, Widget)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		Widget = InArgs._Widget;

		SEditorViewport::Construct(
			SEditorViewport::FArguments()
				.IsEnabled(FSlateApplication::Get().GetNormalExecutionAttribute())
				.AddMetaData<FTagMetaData>(TEXT("JavascriptEditor.Viewport"))
			);
	}
	SAutoRefreshEditorViewport()
		: PreviewScene(FPreviewScene::ConstructionValues().SetEditor(false).AllowAudioPlayback(true))
	{

	}

	~SAutoRefreshEditorViewport()
	{
		EditorViewportClient.Reset();
	}

	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override
	{
		EditorViewportClient = MakeShareable(new FJavascriptEditorViewportClient(PreviewScene,SharedThis(this),Widget));

		return EditorViewportClient.ToSharedRef();
	}

	TSharedPtr<SOverlay> GetOverlay()
	{
		return ViewportOverlay;
	}

	void Redraw()
	{
		SceneViewport->InvalidateDisplay();
	}

	void SetRealtime(bool bInRealtime, bool bStoreCurrentValue)
	{
		EditorViewportClient->SetRealtime(bInRealtime, bStoreCurrentValue);
	}

	void RestoreRealtime(bool bAllowDisable)
	{
		EditorViewportClient->RestoreRealtime(bAllowDisable);
	}	

	void SetBackgroundColor(const FLinearColor& BackgroundColor)
	{
		EditorViewportClient->BackgroundColor = BackgroundColor;
	}

	void SetViewLocation(const FVector& ViewLocation)
	{
		EditorViewportClient->SetViewLocation(ViewLocation);
	}
	
	void SetViewRotation(const FRotator& ViewRotation)
	{
		EditorViewportClient->SetViewRotation(ViewRotation);
	}

	void SetViewFOV(float InViewFOV)
	{
		EditorViewportClient->ViewFOV = InViewFOV;
	}

	float GetViewFOV()
	{
		return EditorViewportClient->ViewFOV;
	}

	void SetCameraSpeedSetting(int32 SpeedSetting)
	{
		EditorViewportClient->SetCameraSpeedSetting(SpeedSetting);
	}

	int32 GetCameraSpeedSetting()
	{
		return EditorViewportClient->GetCameraSpeedSetting();
	}

	void SetViewportType(ELevelViewportType InViewportType)
	{
		EditorViewportClient->SetViewportType(InViewportType);
	}

	void SetViewMode(EViewModeIndex InViewModeIndex)
	{
		EditorViewportClient->SetViewMode(InViewModeIndex);
	}

	void OverridePostProcessSettings(const FPostProcessSettings& PostProcessSettings, float Weight)
	{
		EditorViewportClient->PostProcessSettings = PostProcessSettings;
		EditorViewportClient->PostProcessSettingsWeight = Weight;
	}

	void SetLightDirection(const FRotator& InLightDir)
	{
		PreviewScene.SetLightDirection(InLightDir);
	}

	void SetLightBrightness(float LightBrightness)
	{
		PreviewScene.SetLightBrightness(LightBrightness);
	}

	void SetLightColor(const FColor& LightColor)
	{
		PreviewScene.SetLightColor(LightColor);
	}

	void SetSkyBrightness(float SkyBrightness)
	{
		PreviewScene.SetSkyBrightness(SkyBrightness);
	}

	void SetSimulatePhysics(bool bShouldSimulatePhysics)
	{
		auto World = PreviewScene.GetWorld();
		if (::IsValid(World) == true)
			World->bShouldSimulatePhysics = bShouldSimulatePhysics;
	}

	void SetWidgetMode(EJavascriptWidgetMode WidgetMode)
	{
		EditorViewportClient->SetWidgetMode(WidgetMode == EJavascriptWidgetMode::WM_None ? FWidget::WM_None : (FWidget::EWidgetMode)WidgetMode);
    }
    
    EJavascriptWidgetMode GetWidgetMode()
    {
        FWidget::EWidgetMode WidgetMode = EditorViewportClient->GetWidgetMode();
        return FWidget::WM_None ? EJavascriptWidgetMode::WM_None : (EJavascriptWidgetMode)WidgetMode;
    }

	FString GetEngineShowFlags()
	{
		return EditorViewportClient->EngineShowFlags.ToString();
	}

	bool SetEngineShowFlags(const FString& In)
	{
		if (EditorViewportClient->EngineShowFlags.SetFromString(*In))
		{
			EditorViewportClient->Invalidate();
			return true;
		}
		else
		{
			return false;
		}
	}

	void SetProfileIndex(const int32 InProfileIndex)
	{
		PreviewScene.SetProfileIndex(InProfileIndex);
	}

	int32 GetCurrentProfileIndex()
	{
		return PreviewScene.GetCurrentProfileIndex();
	}

	void SetFloorOffset(const float InFloorOffset)
	{
		PreviewScene.SetFloorOffset(InFloorOffset);
	}

	UStaticMeshComponent* GetFloorMeshComponent()
	{
		return const_cast<UStaticMeshComponent*>(PreviewScene.GetFloorMeshComponent());
	}

	UStaticMeshComponent* GetSkyComponent()
	{
		for (TObjectIterator<UStaticMeshComponent> Itr; Itr; ++Itr)
		{
			if (Itr->GetWorld() != PreviewScene.GetWorld())
				continue;

			UStaticMeshComponent* Component = *Itr;
			if (Component && Component->GetOwner() == nullptr) 
			{
#if ENGINE_MINOR_VERSION >= 14
				auto StaticMesh = Component->GetStaticMesh();
#else
				auto StaticMesh = Component->StaticMesh;
#endif
				if (StaticMesh && StaticMesh->GetName().Equals(FString(TEXT("Sphere_inversenormals"))))
				{
					return Component;
				}
			}
		}

		return nullptr;
	}

public:
	TSharedPtr<FJavascriptEditorViewportClient> EditorViewportClient;
	
	/** preview scene */
	FAdvancedPreviewScene PreviewScene;

private:
	TWeakObjectPtr<UJavascriptEditorViewport> Widget;
};


TSharedRef<SWidget> UJavascriptEditorViewport::RebuildWidget()
{
	if (IsDesignTime())
	{
		return RebuildDesignWidget(SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("EditorViewport", "EditorViewport"))
			]);
	}
	else
	{
		ViewportWidget = SNew(SAutoRefreshEditorViewport).Widget(this);

		for (UPanelSlot* Slot : Slots)
		{
			if (UOverlaySlot* TypedSlot = Cast<UOverlaySlot>(Slot))
			{
				TypedSlot->Parent = this;
				TypedSlot->BuildSlot(ViewportWidget->GetOverlay().ToSharedRef());
			}
		}
		
		return ViewportWidget.ToSharedRef();
	}
}
#endif

UWorld* UJavascriptEditorViewport::GetViewportWorld() const
{
#if WITH_EDITOR
	if (ViewportWidget.IsValid())
	{
		return ViewportWidget->PreviewScene.GetWorld();
	}
#endif
	return nullptr;
}

void UJavascriptEditorViewport::Redraw()
{
	if (ViewportWidget.IsValid())
	{
		ViewportWidget->Redraw();
	}
}

UClass* UJavascriptEditorViewport::GetSlotClass() const
{
	return UOverlaySlot::StaticClass();
}

void UJavascriptEditorViewport::OnSlotAdded(UPanelSlot* Slot)
{
	// Add the child to the live canvas if it already exists
	if (ViewportWidget.IsValid())
	{
		auto MyOverlay = ViewportWidget->GetOverlay();
		Cast<UOverlaySlot>(Slot)->BuildSlot(MyOverlay.ToSharedRef());
	}
}

void UJavascriptEditorViewport::OnSlotRemoved(UPanelSlot* Slot)
{
	// Remove the widget from the live slot if it exists.
	if (ViewportWidget.IsValid())
	{
		TSharedPtr<SWidget> Widget = Slot->Content->GetCachedWidget();
		if (Widget.IsValid())
		{
			auto MyOverlay = ViewportWidget->GetOverlay();
			MyOverlay->RemoveSlot(Widget.ToSharedRef());
		}
	}
}

void UJavascriptEditorViewport::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);
	ViewportWidget.Reset();
}

void UJavascriptEditorViewport::SetRealtime(bool bInRealtime, bool bStoreCurrentValue)
{
	if (ViewportWidget.IsValid())
	{
		ViewportWidget->SetRealtime(bInRealtime,bStoreCurrentValue);
	}
}

void UJavascriptEditorViewport::RestoreRealtime(bool bAllowDisable)
{
	if (ViewportWidget.IsValid())
	{
		ViewportWidget->RestoreRealtime(bAllowDisable);
	}
}

void UJavascriptEditorViewport::OverridePostProcessSettings(const FPostProcessSettings& PostProcessSettings, float Weight)
{
	if (ViewportWidget.IsValid())
	{
		ViewportWidget->OverridePostProcessSettings(PostProcessSettings, Weight);
	}
}

void UJavascriptEditorViewport::SetBackgroundColor(const FLinearColor& BackgroundColor)
{
	if (ViewportWidget.IsValid())
	{
		ViewportWidget->SetBackgroundColor(BackgroundColor);
	}
}

void UJavascriptEditorViewport::SetViewLocation(const FVector& ViewLocation)
{
	if (ViewportWidget.IsValid())
	{
		ViewportWidget->SetViewLocation(ViewLocation);
	}
}

void UJavascriptEditorViewport::SetViewRotation(const FRotator& ViewRotation)
{
	if (ViewportWidget.IsValid())
	{
		ViewportWidget->SetViewRotation(ViewRotation);
	}
}

void UJavascriptEditorViewport::SetViewFOV(float InViewFOV)
{
	if (ViewportWidget.IsValid())
	{
		ViewportWidget->SetViewFOV(InViewFOV);
	}
}

float UJavascriptEditorViewport::GetViewFOV()
{
	if (ViewportWidget.IsValid())
	{
		return ViewportWidget->GetViewFOV();
	}

	return -1.0f;
}

void UJavascriptEditorViewport::SetCameraSpeedSetting(int32 SpeedSetting)
{
	if (ViewportWidget.IsValid())
	{
		return ViewportWidget->SetCameraSpeedSetting(SpeedSetting);
	}
}

int32 UJavascriptEditorViewport::GetCameraSpeedSetting()
{
	if (ViewportWidget.IsValid())
	{
		return ViewportWidget->GetCameraSpeedSetting();
	}

	return -1;
}

void UJavascriptEditorViewport::SetLightDirection(const FRotator& InLightDir)
{
	if (ViewportWidget.IsValid())
	{
		ViewportWidget->SetLightDirection(InLightDir);
	}
}

void UJavascriptEditorViewport::SetLightBrightness(float LightBrightness)
{
	if (ViewportWidget.IsValid())
	{
		ViewportWidget->SetLightBrightness(LightBrightness);
	}
}

void UJavascriptEditorViewport::SetLightColor(const FColor& LightColor)
{
	if (ViewportWidget.IsValid())
	{
		ViewportWidget->SetLightColor(LightColor);
	}
}

void UJavascriptEditorViewport::SetSkyBrightness(float SkyBrightness)
{
	if (ViewportWidget.IsValid())
	{
		ViewportWidget->SetSkyBrightness(SkyBrightness);
	}
}

void UJavascriptEditorViewport::SetSimulatePhysics(bool bShouldSimulatePhysics)
{
	if (ViewportWidget.IsValid())
	{
		ViewportWidget->SetSimulatePhysics(bShouldSimulatePhysics);
	}
}

void UJavascriptEditorViewport::SetWidgetMode(EJavascriptWidgetMode WidgetMode)
{
	if (ViewportWidget.IsValid())
	{
		ViewportWidget->SetWidgetMode(WidgetMode);
	}
}

EJavascriptWidgetMode UJavascriptEditorViewport::GetWidgetMode()
{
    if (ViewportWidget.IsValid())
    {
        return ViewportWidget->GetWidgetMode();
    }
    else {
        return EJavascriptWidgetMode::WM_None;
    }
}

void UJavascriptEditorViewport::SetViewportType(ELevelViewportType InViewportType)
{
	if (ViewportWidget.IsValid())
	{
		ViewportWidget->SetViewportType(InViewportType);
	}
}

void UJavascriptEditorViewport::SetViewMode(EViewModeIndex InViewModeIndex)
{
	if (ViewportWidget.IsValid())
	{
		ViewportWidget->SetViewMode(InViewModeIndex);
	}
}

void UJavascriptEditorViewport::DeprojectScreenToWorld(const FVector2D &ScreenPosition, FVector &OutRayOrigin, FVector& OutRayDirection)
{
    if (ViewportWidget.IsValid())
    {
        FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues( ViewportWidget->EditorViewportClient->Viewport, ViewportWidget->EditorViewportClient->GetScene(), ViewportWidget->EditorViewportClient->EngineShowFlags ));
        FSceneView* View = ViewportWidget->EditorViewportClient->CalcSceneView(&ViewFamily);
        
#if ENGINE_MINOR_VERSION >= 14
		const auto& InvViewProjMatrix = View->ViewMatrices.GetInvViewProjectionMatrix();
#else
		const auto& InvViewProjMatrix = View->ViewMatrices.GetInvViewProjMatrix();
#endif
        FSceneView::DeprojectScreenToWorld(ScreenPosition, View->ViewRect, InvViewProjMatrix, OutRayOrigin, OutRayDirection);
    }
}

void UJavascriptEditorViewport::ProjectWorldToScreen(const FVector &WorldPosition, FVector2D &OutScreenPosition)
{
    if (ViewportWidget.IsValid())
    {
        FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues( ViewportWidget->EditorViewportClient->Viewport, ViewportWidget->EditorViewportClient->GetScene(), ViewportWidget->EditorViewportClient->EngineShowFlags ));
        FSceneView* View = ViewportWidget->EditorViewportClient->CalcSceneView(&ViewFamily);

#if ENGINE_MINOR_VERSION >= 14
		const auto& ViewProjMatrix = View->ViewMatrices.GetViewProjectionMatrix();
#else
		const auto& ViewProjMatrix = View->ViewMatrices.GetViewProjMatrix();
#endif
        
        FSceneView::ProjectWorldToScreen(WorldPosition, View->ViewRect, ViewProjMatrix, OutScreenPosition);
    }
}

FString UJavascriptEditorViewport::GetEngineShowFlags()
{
	if (ViewportWidget.IsValid())
	{
		return ViewportWidget->GetEngineShowFlags();
	}
	else
	{
		return TEXT("");
	}
}

bool UJavascriptEditorViewport::SetEngineShowFlags(const FString& In)
{
	if (ViewportWidget.IsValid())
	{
		return ViewportWidget->SetEngineShowFlags(In);
	}
	else
	{
		return false;
	}
}

void UJavascriptEditorViewport::SetProfileIndex(const int32 InProfileIndex)
{
	if (ViewportWidget.IsValid())
	{
		ViewportWidget->SetProfileIndex(InProfileIndex);
	}
}

int32 UJavascriptEditorViewport::GetCurrentProfileIndex()
{
	if (ViewportWidget.IsValid())
	{
		return ViewportWidget->GetCurrentProfileIndex();
	}

	return 0;
}

UAssetViewerSettings* UJavascriptEditorViewport::GetDefaultAssetViewerSettings()
{
	return UAssetViewerSettings::Get();
}

void UJavascriptEditorViewport::SetFloorOffset(const float InFloorOffset)
{
	if (ViewportWidget.IsValid())
	{
		ViewportWidget->SetFloorOffset(InFloorOffset);
	}
}

UStaticMeshComponent* UJavascriptEditorViewport::GetFloorMeshComponent()
{
	if (ViewportWidget.IsValid())
	{
		return ViewportWidget->GetFloorMeshComponent();
	}
	return nullptr;
}

UStaticMeshComponent* UJavascriptEditorViewport::GetSkyComponent()
{
	if (ViewportWidget.IsValid())
	{
		return ViewportWidget->GetSkyComponent();
	}

	return nullptr;
}

PRAGMA_ENABLE_SHADOW_VARIABLE_WARNINGS

#undef LOCTEXT_NAMESPACE
