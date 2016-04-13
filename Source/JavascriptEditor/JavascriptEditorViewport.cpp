#include "JavascriptEditor.h"
#include "JavascriptEditorViewport.h"
#include "SEditorViewport.h"
#include "PreviewScene.h"
#include "Runtime/Engine/Public/Slate/SceneViewport.h"

#define LOCTEXT_NAMESPACE "JavascriptEditor"


#if WITH_EDITOR
class FJavascriptEditorViewportClient : public FEditorViewportClient
{
public:
	/** Constructor */
	explicit FJavascriptEditorViewportClient(FPreviewScene& InPreviewScene, const TWeakPtr<class SEditorViewport>& InEditorViewportWidget = nullptr)
		: FEditorViewportClient(nullptr,&InPreviewScene,InEditorViewportWidget)
	{		
	}
	~FJavascriptEditorViewportClient()
	{}

	virtual void ProcessClick(class FSceneView& View, class HHitProxy* HitProxy, FKey Key, EInputEvent Event, uint32 HitX, uint32 HitY) override
	{
		
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
					It->BeginPlay();
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
};

class SAutoRefreshEditorViewport : public SEditorViewport
{
	SLATE_BEGIN_ARGS(SAutoRefreshEditorViewport)
	{
	}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		SEditorViewport::Construct(
			SEditorViewport::FArguments()
				.IsEnabled(FSlateApplication::Get().GetNormalExecutionAttribute())
				.AddMetaData<FTagMetaData>(TEXT("JavascriptEditor.Viewport"))
			);
	}

	virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override
	{
		EditorViewportClient = MakeShareable(new FJavascriptEditorViewportClient(PreviewScene,SharedThis(this)));

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

public:
	TSharedPtr<FJavascriptEditorViewportClient> EditorViewportClient;
	
	/** preview scene */
	FPreviewScene PreviewScene;
};


TSharedRef<SWidget> UJavascriptEditorViewport::RebuildWidget()
{
	if (IsDesignTime())
	{
		return BuildDesignTimeWidget(SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("EditorViewport", "EditorViewport"))
			]);
	}
	else
	{
		ViewportWidget = SNew(SAutoRefreshEditorViewport);

		for (UPanelSlot* Slot : Slots)
		{
			if (UOverlaySlot* TypedSlot = Cast<UOverlaySlot>(Slot))
			{
				TypedSlot->Parent = this;
				TypedSlot->BuildSlot(ViewportWidget->GetOverlay().ToSharedRef());
			}
		}
		
		return BuildDesignTimeWidget(ViewportWidget.ToSharedRef());
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