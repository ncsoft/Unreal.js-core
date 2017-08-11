#pragma once

#include "CoreMinimal.h"
#include "EngineUtils.h"

#include "JavascriptInGameScene.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SceneCaptureComponentCube.h"
#include "Modules/ModuleVersion.h"
#include "EngineUtils.h"
#include "UnrealEngine.h"
#include "InGameSceneAccessor.generated.h"

UCLASS()
class UInGameSceneAccessor : public UObject
{
	GENERATED_BODY()

public:
	UInGameSceneAccessor(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
	{
	}
	
	~UInGameSceneAccessor()
	{
	}

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	UInGameSceneAccessor* Initialize()
	{
		FJavascriptInGameScene::ConstructionValues CVS;
		CVS.ShouldSimulatePhysics(true);

		GameScene = MakeShareable(new FJavascriptInGameScene(CVS));
		DelegateHandle = GEngine->GameViewport->OnTick().AddUObject(this, &UInGameSceneAccessor::Tick);
		return this;
	}

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	void Destroy()
	{
		if (GameScene.IsValid()) 
			GameScene->Destroy();
		GameScene.Reset();
		if (DelegateHandle.IsValid())
		{
			GEngine->GameViewport->OnTick().Remove(DelegateHandle);
			DelegateHandle.Reset();
		}
	}

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	UWorld* GetSceneWorld()
	{
		return GameScene->GetWorld();
	}
	
	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	void Tick(float DeltaTime)
	{
		if (!GIntraFrameDebuggingGameThread)
		{
			if(!GameScene.IsValid()) return;
			// Begin Play
			if (!GameScene->GetWorld()->bBegunPlay)
			{
				for (FActorIterator It(GameScene->GetWorld()); It; ++It)
				{
#if ENGINE_MINOR_VERSION > 14
					It->DispatchBeginPlay();
#else
					It->BeginPlay();
#endif
				}
				GameScene->GetWorld()->bBegunPlay = true;
			}

			// Tick
			GameScene->GetWorld()->Tick(LEVELTICK_All, DeltaTime);
			GameScene->GetWorld()->SendAllEndOfFrameUpdates();

			auto Scene = GameScene->GetScene();
			USceneCaptureComponent2D::UpdateDeferredCaptures(Scene);
			USceneCaptureComponentCube::UpdateDeferredCaptures(Scene);
		}
	}
	
	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	bool isBegunPlay()
	{
		return GameScene->GetWorld()->bBegunPlay;
	}

private:
	TSharedPtr<FJavascriptInGameScene> GameScene;
 	FDelegateHandle DelegateHandle;
};
