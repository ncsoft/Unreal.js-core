
#include "JavascriptInGameScene.h"
#include "SoundDefinitions.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/LineBatchComponent.h"
#include "Components/MeshComponent.h"
#include "AudioDevice.h"
#include "Misc/ConfigCacheIni.h"
#include "../../Launch/Resources/Version.h"

FJavascriptInGameScene::FJavascriptInGameScene(FJavascriptInGameScene::ConstructionValues CVS)
	: PreviewWorld(NULL)
	, bForceAllUsedMipsResident(CVS.bForceMipsResident)
{
	PreviewWorld = NewObject<UWorld>();
	PreviewWorld->WorldType = EWorldType::Game;
	if (CVS.bTransactional)
	{
		PreviewWorld->SetFlags(RF_Transactional);
	}

	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(PreviewWorld);

	PreviewWorld->InitializeNewWorld(UWorld::InitializationValues()
		.AllowAudioPlayback(CVS.bAllowAudioPlayback)
		.CreatePhysicsScene(CVS.bCreatePhysicsScene)
		.RequiresHitProxies(false)
		.CreateNavigation(false)
		.CreateAISystem(false)
		.ShouldSimulatePhysics(CVS.bShouldSimulatePhysics)
		.SetTransactional(CVS.bTransactional));
	PreviewWorld->InitializeActorsForPlay(FURL());

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 13
	GetScene()->UpdateDynamicSkyLight(FLinearColor::White * CVS.SkyBrightness, FLinearColor::Black);
#endif

	DirectionalLight = NewObject<UDirectionalLightComponent>((UObject*)GetTransientPackage());
	DirectionalLight->Intensity = CVS.LightBrightness;
	DirectionalLight->LightColor = FColor::White;
	AddComponent(DirectionalLight, FTransform(CVS.LightRotation));
}

FJavascriptInGameScene::~FJavascriptInGameScene()
{
	Destroy();
}

void FJavascriptInGameScene::Destroy()
{
	if (bDestroyed) return;

	// Stop any audio components playing in this scene
	if (GEngine)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			if (FAudioDeviceHandle AudioDevice = World->GetAudioDevice())
			{
				AudioDevice->Flush(GetWorld(), false);
			}
		}
	}

	// Remove all the attached components
	for (int32 ComponentIndex = 0; ComponentIndex < Components.Num(); ComponentIndex++)
	{
		UActorComponent* Component = Components[ComponentIndex];

		if (bForceAllUsedMipsResident)
		{
			// Remove the mip streaming override on the mesh to be removed
			UMeshComponent* pMesh = Cast<UMeshComponent>(Component);
			if (pMesh != NULL)
			{
				pMesh->SetTextureForceResidentFlag(false);
			}
		}

		Component->UnregisterComponent();
	}

	if (GEngine) 
	{
		PreviewWorld->CleanupWorld();
		GEngine->DestroyWorldContext(GetWorld());
	}
	bDestroyed = true;
}

void FJavascriptInGameScene::AddComponent(UActorComponent* Component, const FTransform& LocalToWorld)
{
	Components.AddUnique(Component);

	USceneComponent* SceneComp = Cast<USceneComponent>(Component);
	if (SceneComp && SceneComp->GetAttachParent() == NULL)
	{
		SceneComp->SetRelativeTransform(LocalToWorld);
	}

	Component->RegisterComponentWithWorld(GetWorld());

	if (bForceAllUsedMipsResident)
	{
		// Add a mip streaming override to the new mesh
		UMeshComponent* pMesh = Cast<UMeshComponent>(Component);
		if (pMesh != NULL)
		{
			pMesh->SetTextureForceResidentFlag(true);
		}
	}

	GetScene()->UpdateSpeedTreeWind(0.0);
}

void FJavascriptInGameScene::RemoveComponent(UActorComponent* Component)
{
	Component->UnregisterComponent();
	Components.Remove(Component);

	if (bForceAllUsedMipsResident)
	{
		// Remove the mip streaming override on the old mesh
		UMeshComponent* pMesh = Cast<UMeshComponent>(Component);
		if (pMesh != NULL)
		{
			pMesh->SetTextureForceResidentFlag(false);
		}
	}
}

void FJavascriptInGameScene::AddReferencedObjects(FReferenceCollector& Collector)
{
	for (int32 Index = 0; Index < Components.Num(); Index++)
	{
		Collector.AddReferencedObject(Components[Index]);
	}
	Collector.AddReferencedObject(DirectionalLight);
	Collector.AddReferencedObject(PreviewWorld);
}

/** Accessor for finding the current direction of the preview scene's DirectionalLight. */
FRotator FJavascriptInGameScene::GetLightDirection()
{
	return DirectionalLight->GetComponentTransform().GetUnitAxis(EAxis::X).Rotation();
}

/** Function for modifying the current direction of the preview scene's DirectionalLight. */
void FJavascriptInGameScene::SetLightDirection(const FRotator& InLightDir)
{
	DirectionalLight->SetAbsolute(true, true, true);
	DirectionalLight->SetRelativeRotation(InLightDir);
}

void FJavascriptInGameScene::SetLightBrightness(float LightBrightness)
{
	DirectionalLight->Intensity = LightBrightness;
}

void FJavascriptInGameScene::SetLightColor(const FColor& LightColor)
{
	DirectionalLight->LightColor = LightColor;
}

void FJavascriptInGameScene::SetSkyBrightness(float SkyBrightness)
{
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 13
	GetScene()->UpdateDynamicSkyLight(FLinearColor::White * SkyBrightness, FLinearColor::Black);
#endif
}

void FJavascriptInGameScene::LoadSettings(const TCHAR* Section)
{
	FRotator LightDir;
	if (GConfig->GetRotator(Section, TEXT("LightDir"), LightDir, GEditorPerProjectIni))
	{
		SetLightDirection(LightDir);
	}
}

void FJavascriptInGameScene::SaveSettings(const TCHAR* Section)
{
	GConfig->SetRotator(Section, TEXT("LightDir"), GetLightDirection(), GEditorPerProjectIni);
}
