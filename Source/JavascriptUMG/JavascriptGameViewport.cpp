﻿
#include "JavascriptGameViewport.h"
#include "Slate/SceneViewport.h"
#include "Framework/Application/SlateApplication.h"

#include "EngineUtils.h"
#include "EngineModule.h"
#include "SceneView.h"
#include "CanvasTypes.h"
#include "Widgets/SViewport.h"
#include "Runtime/Launch/Resources/Version.h"

#define LOCTEXT_NAMESPACE "UMG"

namespace FocusConstants
{
	const float TransitionTime = 0.25f;
}

FJavascriptUMGViewportCameraTransform::FJavascriptUMGViewportCameraTransform()
	: TransitionStartTime(0)
	, ViewLocation(FVector::ZeroVector)
	, ViewRotation(FRotator::ZeroRotator)
	, DesiredLocation(FVector::ZeroVector)
	, LookAt(FVector::ZeroVector)
	, StartLocation(FVector::ZeroVector)
	, OrthoZoom(DEFAULT_ORTHOZOOM)
{}

void FJavascriptUMGViewportCameraTransform::SetLocation(const FVector& Position)
{
	ViewLocation = Position;
	DesiredLocation = ViewLocation;
}

void FJavascriptUMGViewportCameraTransform::TransitionToLocation(const FVector& InDesiredLocation, bool bInstant)
{
	if (bInstant)
	{
		SetLocation(InDesiredLocation);
		TransitionStartTime = FSlateApplication::Get().GetCurrentTime() - FocusConstants::TransitionTime;
	}
	else
	{
		DesiredLocation = InDesiredLocation;
		StartLocation = ViewLocation;

		TransitionStartTime = FSlateApplication::Get().GetCurrentTime();
	}
}

bool FJavascriptUMGViewportCameraTransform::UpdateTransition()
{
	bool bIsAnimating = false;
	double TransitionProgress = FMath::Clamp((FSlateApplication::Get().GetCurrentTime() - TransitionStartTime) / FocusConstants::TransitionTime, 0.0, 1.0);
	if (TransitionProgress < 1.0 || ViewLocation != DesiredLocation)
	{
		const float Offset = (float)TransitionProgress - 1.0f;
		float LerpWeight = Offset * Offset * Offset + 1.0f;

		if (LerpWeight == 1.0f)
		{
			// Failsafe for the value not being exact on lerps
			ViewLocation = DesiredLocation;
		}
		else
		{
			ViewLocation = FMath::Lerp(StartLocation, DesiredLocation, LerpWeight);
		}


		bIsAnimating = true;
	}

	return bIsAnimating;
}

FMatrix FJavascriptUMGViewportCameraTransform::ComputeOrbitMatrix() const
{
	FTransform Transform =
		FTransform(-LookAt) *
		FTransform(FRotator(0, ViewRotation.Yaw, 0)) *
		FTransform(FRotator(0, 0, ViewRotation.Pitch)) *
		FTransform(FVector(0, (ViewLocation - LookAt).Size(), 0));

	return Transform.ToMatrixNoScale() * FInverseRotationMatrix(FRotator(0, 90.f, 0));
}

FJavascriptUMGViewportClient::FJavascriptUMGViewportClient(FJavascriptInGameScene* InGameScene)
	: GameScene(InGameScene)
	, Viewport(nullptr)
	, EngineShowFlags(ESFIM_Game)
{
	ViewState.Allocate(GMaxRHIFeatureLevel);

	BackgroundColor = FColor(55, 55, 55);
}

FJavascriptUMGViewportClient::~FJavascriptUMGViewportClient()
{
	ViewState.Destroy();
}

void FJavascriptUMGViewportClient::Tick(float InDeltaTime)
{
	if (!GIntraFrameDebuggingGameThread)
	{
		// Begin Play
		if (!GameScene->GetWorld()->bBegunPlay)
		{
			for (FActorIterator It(GameScene->GetWorld()); It; ++It)
			{
#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION > 14) || ENGINE_MAJOR_VERSION > 4
				It->DispatchBeginPlay();
#else
				if (It->HasActorBegunPlay() == false)
				{
					It->BeginPlay();
				}
#endif
			}
			GameScene->GetWorld()->bBegunPlay = true;
		}

		// Tick
		GameScene->GetWorld()->Tick(LEVELTICK_All, InDeltaTime);
	}
}

void FJavascriptUMGViewportClient::Draw(FViewport* InViewport, FCanvas* Canvas)
{
	FViewport* ViewportBackup = Viewport;
	Viewport = InViewport ? InViewport : Viewport;

	// Determine whether we should use world time or real time based on the scene.
	float TimeSeconds;
	float RealTimeSeconds;
	float DeltaTimeSeconds;

	const bool bIsRealTime = true;

	UWorld* World = GWorld;
	if ((GetScene() != World->Scene) || (bIsRealTime == true))
	{
		// Use time relative to start time to avoid issues with float vs double
		TimeSeconds = FApp::GetCurrentTime() - GStartTime;
		RealTimeSeconds = FApp::GetCurrentTime() - GStartTime;
		DeltaTimeSeconds = FApp::GetDeltaTime();
	}
	else
	{
		TimeSeconds = World->GetTimeSeconds();
		RealTimeSeconds = World->GetRealTimeSeconds();
		DeltaTimeSeconds = World->GetDeltaSeconds();
	}

	// Setup a FSceneViewFamily/FSceneView for the viewport.
	FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(
		Canvas->GetRenderTarget(),
		GetScene(),
		EngineShowFlags)
		.SetTime(FGameTime::CreateDilated(RealTimeSeconds, DeltaTimeSeconds, TimeSeconds, DeltaTimeSeconds))
		.SetRealtimeUpdate(bIsRealTime));

	ViewFamily.EngineShowFlags = EngineShowFlags;

	//UpdateLightingShowFlags(ViewFamily.EngineShowFlags);

	//ViewFamily.ExposureSettings = ExposureSettings;

	//ViewFamily.LandscapeLODOverride = LandscapeLODOverride;

	FSceneView* View = CalcSceneView(&ViewFamily);

	//SetupViewForRendering(ViewFamily, *View);

	FSlateRect SafeFrame;
	View->CameraConstrainedViewRect = View->UnscaledViewRect;
	//if ( CalculateEditorConstrainedViewRect(SafeFrame, Viewport) )
	//{
	//	View->CameraConstrainedViewRect = FIntRect(SafeFrame.Left, SafeFrame.Top, SafeFrame.Right, SafeFrame.Bottom);
	//}

	if (IsAspectRatioConstrained())
	{
		// Clear the background to black if the aspect ratio is constrained, as the scene view won't write to all pixels.
		Canvas->Clear(FLinearColor::Black);
	}

	Canvas->Clear(BackgroundColor);

	// workaround for hacky renderer code that uses GFrameNumber to decide whether to resize render targets
	--GFrameNumber;
	GetRendererModule().BeginRenderingViewFamily(Canvas, &ViewFamily);
	
	Viewport = ViewportBackup;
}

FSceneInterface* FJavascriptUMGViewportClient::GetScene() const
{
	UWorld* World = GetWorld();
	if (World)
	{
		return World->Scene;
	}

	return nullptr;
}

UWorld* FJavascriptUMGViewportClient::GetWorld() const
{
	UWorld* OutWorldPtr = nullptr;
	// If we have a valid scene get its world
	if (GameScene)
	{
		OutWorldPtr = GameScene->GetWorld();
	}
	if (OutWorldPtr == nullptr)
	{
		OutWorldPtr = GWorld;
	}
	return OutWorldPtr;
}

bool FJavascriptUMGViewportClient::IsAspectRatioConstrained() const
{
	return ViewInfo.bConstrainAspectRatio;
}

void FJavascriptUMGViewportClient::SetBackgroundColor(FLinearColor InBackgroundColor)
{
	BackgroundColor = InBackgroundColor;
}

FLinearColor FJavascriptUMGViewportClient::GetBackgroundColor() const
{
	return BackgroundColor;
}

float FJavascriptUMGViewportClient::GetOrthoUnitsPerPixel(const FViewport* InViewport) const
{
	const float SizeX = InViewport->GetSizeXY().X;

	// 15.0f was coming from the CAMERA_ZOOM_DIV marco, seems it was chosen arbitrarily
	return (GetOrthoZoom() / (SizeX * 15.f))/* * ComputeOrthoZoomFactor(SizeX)*/;
}


FSceneView* FJavascriptUMGViewportClient::CalcSceneView(FSceneViewFamily* ViewFamily)
{
	FSceneViewInitOptions ViewInitOptions;

	const FVector& ViewLocation = GetViewLocation();
	const FRotator& ViewRotation = GetViewRotation();

	const FIntPoint ViewportSizeXY = Viewport->GetSizeXY();

	FIntRect ViewRect = FIntRect(0, 0, ViewportSizeXY.X, ViewportSizeXY.Y);
	ViewInitOptions.SetViewRectangle(ViewRect);

	ViewInitOptions.ViewOrigin = ViewLocation;

	ViewInitOptions.ViewRotationMatrix = FInverseRotationMatrix(ViewRotation);
	ViewInitOptions.ViewRotationMatrix = ViewInitOptions.ViewRotationMatrix * FMatrix(
		FPlane(0, 0, 1, 0),
		FPlane(1, 0, 0, 0),
		FPlane(0, 1, 0, 0),
		FPlane(0, 0, 0, 1));

	//@TODO: Should probably be locally configurable (or just made into a FMinimalViewInfo property)
	const EAspectRatioAxisConstraint AspectRatioAxisConstraint = GetDefault<ULocalPlayer>()->AspectRatioAxisConstraint;

	FMinimalViewInfo::CalculateProjectionMatrixGivenView(ViewInfo, AspectRatioAxisConstraint, Viewport, /*inout*/ ViewInitOptions);

	ViewInitOptions.ViewFamily = ViewFamily;
	ViewInitOptions.SceneViewStateInterface = ViewState.GetReference();
	ViewInitOptions.ViewElementDrawer = this;

	ViewInitOptions.BackgroundColor = GetBackgroundColor();

	//ViewInitOptions.EditorViewBitflag = 0, // send the bit for this view - each actor will check it's visibility bits against this

	// for ortho views to steal perspective view origin
	//ViewInitOptions.OverrideLODViewOrigin = FVector::ZeroVector;
	//ViewInitOptions.bUseFauxOrthoViewPos = true;

	//ViewInitOptions.CursorPos = CurrentMousePos;

	FSceneView* View = new FSceneView(ViewInitOptions);

	ViewFamily->Views.Add(View);

	View->StartFinalPostprocessSettings(ViewLocation);

	//OverridePostProcessSettings(*View);

	View->EndFinalPostprocessSettings(ViewInitOptions);

	return View;
}

/////////////////////////////////////////////////////
// SAutoRefreshViewport

class SJavascriptAutoRefreshViewport : public SViewport
{
	SLATE_BEGIN_ARGS(SJavascriptAutoRefreshViewport)
	{}

	SLATE_ATTRIBUTE(FVector2D, ViewportSize);
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		SViewport::FArguments ParentArgs;
		ParentArgs.IgnoreTextureAlpha(false);
		ParentArgs.ShowEffectWhenDisabled(false);
		ParentArgs.EnableBlending(false);
		ParentArgs.EnableGammaCorrection(false);
		ParentArgs.ViewportSize(InArgs._ViewportSize);
 		// ParentArgs.RenderDirectlyToWindow(true);
		SViewport::Construct(ParentArgs);

		FJavascriptInGameScene::ConstructionValues CVS;
		CVS.ShouldSimulatePhysics(true);

		GameScene = MakeShareable(new FJavascriptInGameScene(CVS));

		ViewportClient = MakeShareable(new FJavascriptUMGViewportClient(GameScene.Get()));
		Viewport = MakeShareable(new FSceneViewport(ViewportClient.Get(), SharedThis(this)));

		// The viewport widget needs an interface so it knows what should render
		SetViewportInterface(Viewport.ToSharedRef());
	}

	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override
	{
		Viewport->Invalidate();
		//Viewport->InvalidateDisplay();

		Viewport->Tick(AllottedGeometry, InCurrentTime, InDeltaTime);
		ViewportClient->Tick(InDeltaTime);
	}

public:
	TSharedPtr<FJavascriptUMGViewportClient> ViewportClient;

	TSharedPtr<FSceneViewport> Viewport;

	TSharedPtr<FJavascriptInGameScene> GameScene;
};

/////////////////////////////////////////////////////
// UJavascriptGameViewport

UJavascriptGameViewport::UJavascriptGameViewport(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, ShowFlags(ESFIM_Game)
{
	bIsVariable = true;

	BackgroundColor = FLinearColor::Black;
	ShowFlags.DisableAdvancedFeatures();
	//ParentArgs.IgnoreTextureAlpha(false);
	//ParentArgs.EnableBlending(true);
	////ParentArgs.RenderDirectlyToWindow(true);
}

void UJavascriptGameViewport::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyViewportWidget.Reset();
}

TSharedRef<SWidget> UJavascriptGameViewport::RebuildWidget()
{
	auto ViewportWidget = SNew(SJavascriptAutoRefreshViewport);

	if (GetChildrenCount() > 0)
	{
		ViewportWidget->SetContent(GetContentSlot()->Content ? GetContentSlot()->Content->TakeWidget() : SNullWidget::NullWidget);
	}

	MyViewportWidget = ViewportWidget;
	return ViewportWidget;
}

void UJavascriptGameViewport::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (MyViewportWidget.IsValid())
	{
		MyViewportWidget->ViewportClient->SetBackgroundColor(BackgroundColor);
		MyViewportWidget->ViewportClient->SetEngineShowFlags(ShowFlags);
	}
}

void UJavascriptGameViewport::OnSlotAdded(UPanelSlot* InSlot)
{
	// Add the child to the live canvas if it already exists
	if (MyViewportWidget.IsValid())
	{
		MyViewportWidget->SetContent(InSlot->Content ? InSlot->Content->TakeWidget() : SNullWidget::NullWidget);
	}
}

void UJavascriptGameViewport::OnSlotRemoved(UPanelSlot* InSlot)
{
	// Remove the widget from the live slot if it exists.
	if (MyViewportWidget.IsValid())
	{
		MyViewportWidget->SetContent(SNullWidget::NullWidget);
	}
}

UWorld* UJavascriptGameViewport::GetViewportWorld() const
{
	if (MyViewportWidget.IsValid())
	{
		return MyViewportWidget->GameScene->GetWorld();
	}

	return nullptr;
}

FVector UJavascriptGameViewport::GetViewLocation() const
{
	if (MyViewportWidget.IsValid())
	{
		return MyViewportWidget->ViewportClient->GetViewLocation();
	}

	return FVector();
}

void UJavascriptGameViewport::SetViewLocation(FVector Vector)
{
	if (MyViewportWidget.IsValid())
	{
		MyViewportWidget->ViewportClient->SetViewLocation(Vector);
	}
}

FRotator UJavascriptGameViewport::GetViewRotation() const
{
	if (MyViewportWidget.IsValid())
	{
		return MyViewportWidget->ViewportClient->GetViewRotation();
	}

	return FRotator();
}

void UJavascriptGameViewport::SetViewRotation(FRotator Rotator)
{
	if (MyViewportWidget.IsValid())
	{
		MyViewportWidget->ViewportClient->SetViewRotation(Rotator);
	}
}

AActor* UJavascriptGameViewport::Spawn(TSubclassOf<AActor> ActorClass)
{
	if (MyViewportWidget.IsValid())
	{
		UWorld* World = GetViewportWorld();
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		return World->SpawnActor<AActor>(ActorClass, FVector(0, 0, 0), FRotator(), SpawnParameters);
	}

	// TODO UMG Report spawning actor error before the world is ready.

	return nullptr;
}

#if WITH_EDITOR

const FText UJavascriptGameViewport::GetPaletteCategory()
{
	return LOCTEXT("Primitive", "Primitive");
}

#endif

/////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE
