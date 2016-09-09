#pragma once

#include "JavascriptInGameScene.h"
#include "Components/SceneCaptureComponent.h"
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
		if(DelegateHandle.IsValid())
			GEngine->GameViewport->OnTick().Remove(DelegateHandle);
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
			// Begin Play
			if (!GameScene->GetWorld()->bBegunPlay)
			{
				for (FActorIterator It(GameScene->GetWorld()); It; ++It)
				{
					It->BeginPlay();
				}
				GameScene->GetWorld()->bBegunPlay = true;
			}

			// Tick
			GameScene->GetWorld()->Tick(LEVELTICK_All, DeltaTime);
			GameScene->GetWorld()->SendAllEndOfFrameUpdates();
			for (const auto& Scene : SceneComponents) {
				GameScene->GetScene()->UpdateSceneCaptureContents(Scene);
			}
		}
	}
	
	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	bool isBegunPlay()
	{
		return GameScene->GetWorld()->bBegunPlay;
	}

	UPROPERTY()
	TArray<USceneCaptureComponent2D*> SceneComponents;

private:
	TSharedPtr<FJavascriptInGameScene> GameScene;
 	FDelegateHandle DelegateHandle;
};
