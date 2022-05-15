#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/ScriptMacros.h"
#include "UObject/UnrealType.h"
#include "UObject/FieldPath.h"
#include "UObject/ObjectSaveContext.h"
#include "Engine/World.h"
#include "JavascriptGlobalDelegates.generated.h"

UCLASS()
class V8_API UJavascriptGlobalDelegates : public UObject
{
	GENERATED_BODY()

public:
	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
	void OnPreObjectPropertyChanged_Friendly(UObject* InObject, const TFieldPath<FProperty>& Property, const TFieldPath<FProperty>& MemberProperty);

	void OnPreObjectPropertyChanged(UObject* InObject, const class FEditPropertyChain& Chain)
	{
		OnPreObjectPropertyChanged_Friendly(InObject, Chain.GetActiveNode()->GetValue(), Chain.GetActiveMemberNode()->GetValue());
	}

	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
	void OnObjectPropertyChanged_Friendly(UObject* InObject, const TFieldPath<FProperty>& Property, const TFieldPath<FProperty>& MemberProperty, int32 ChangeType);

	void OnObjectPropertyChanged(UObject* InObject, struct FPropertyChangedEvent& Event)
	{
		if (Event.Property != nullptr)
		{
			OnObjectPropertyChanged_Friendly(InObject, Event.Property, Event.MemberProperty, (int32)Event.ChangeType);
		}
	}

	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
	void RedirectorFollowed(const FString& PackageName, UObject* Redirector);

	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
	void PreGarbageCollectDelegate();

	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
	void PostGarbageCollect();

	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
	void PreLoadMap(const FString& MapName);
	
	void PreLoadMap_Old()
	{
		PreLoadMap(TEXT(""));
	}

	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
	void PostLoadMapWithWorld(UWorld* World);

	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
	void PostDemoPlay();
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
	void PackageCreatedForLoad(class UPackage* InPackage);

	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
	void OnActorLabelChanged(AActor* Actor);

	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
	void OnObjectModified(UObject* Object);

	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
	void OnAssetLoaded(UObject* Object);	

	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
	void OnObjectPreSave_Friendly(UObject* ObjectSaved);

	void OnObjectPreSave(UObject* ObjectSaved, FObjectPreSaveContext SaveContext)
	{
		OnObjectPreSave_Friendly(ObjectSaved);
	}

	/// WorldDelegates
	///@todo : ELevelTick -> BT Type
// 	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
// 	void OnWorldTickStart(ELevelTick LevelTickType, float Delta);

	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
	void OnPostWorldCreation(UWorld* World);

	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
	void OnPreWorldInitialization_Friendly(UWorld* World);

	void OnPreWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS)
	{
		OnPreWorldInitialization_Friendly(World);
	}

	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
	void OnPostWorldInitialization_Friendly(UWorld* World);

	void OnPostWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS)
	{
		OnPostWorldInitialization_Friendly(World);
	}

#if WITH_EDITOR
	// Callback for world rename event (pre)
// 	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
// 	void OnPreWorldRename(UWorld* World, const TCHAR* InName, UObject* NewOuter, ERenameFlags Flags, bool& bShouldFailRename);
#endif // WITH_EDITOR

 	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
	void OnPostDuplicate_Friendly(UWorld* World, bool bDuplicateForPIE);

	void OnPostDuplicate(UWorld* World, bool bDuplicateForPIE, FWorldDelegates::FReplacementMap& ReplacementMap, TArray<UObject*>& ObjectsToFixReferences)
	{
		OnPostDuplicate_Friendly(World, bDuplicateForPIE);
	}

	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
	void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources);

	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
	void OnPreWorldFinishDestroy(UWorld* World);

	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
	void LevelAddedToWorld(ULevel* Level, UWorld* World);

	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
	void LevelRemovedFromWorld(ULevel* Level, UWorld* World);

	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
	void PostApplyLevelOffset(ULevel* Level, UWorld* World, const FVector& Offset, bool Flag);

// 	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
// 	void GetAssetTags(const UWorld* World, TArray<UObject::FAssetRegistryTag>& Tags);

#if WITH_EDITOR
// 	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
// 	void RefreshLevelScriptActions(UWorld* World);
#endif
	/// end of WorldDelegates

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	void Bind(FString Key);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	void Unbind(FString Key);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	void UnbindAll();

	TMap<FString,FDelegateHandle> Handles;
};