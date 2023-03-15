#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/ScriptMacros.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "../../Launch/Resources/Version.h"
#include "JavascriptGeneratedClass_Native.generated.h"

struct FJavascriptContext;

UCLASS()
class V8_API UJavascriptGeneratedClass_Native : public UBlueprintGeneratedClass
{
	GENERATED_BODY()

public:
	// UObject interface
	virtual void Serialize(FArchive& Ar) override { UClass::Serialize(Ar);  }
	virtual void PostLoad() override { UClass::PostLoad(); }
	virtual void PostInitProperties() override { UClass::PostInitProperties(); }
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
	virtual void PostInitInstance(UObject* InObj, FObjectInstancingGraph* InstanceGraph) override;
#else
	virtual void PostInitInstance(UObject* InObj) override;
#endif
	// End UObject interface

	// UClass interface
#if WITH_EDITOR
	virtual UClass* GetAuthoritativeClass() override { return UClass::GetAuthoritativeClass();  }
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 22
	virtual void ConditionalRecompileClass(TArray<UObject*>* ObjLoaded) override { UClass::ConditionalRecompileClass(ObjLoaded);  }
#else
	virtual void ConditionalRecompileClass(FUObjectSerializeContext* InLoadContext) override { UClass::ConditionalRecompileClass(InLoadContext); }
#endif
	virtual UObject* GetArchetypeForCDO() const override { return UClass::GetArchetypeForCDO();  }
#endif //WITH_EDITOR

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 23
	virtual bool IsFunctionImplementedInBlueprint(FName InFunctionName) const override { return false;  }
#else
	virtual bool IsFunctionImplementedInScript(FName InFunctionName) const override { return false; }
#endif

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 21
	virtual uint8* GetPersistentUberGraphFrame(UObject* Obj, UFunction* FuncToCheck) const override { return nullptr;  }
	virtual void GetRequiredPreloadDependencies(TArray<UObject*>& DependenciesOut) override { UClass::GetRequiredPreloadDependencies(DependenciesOut);  }
#endif
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 12
	virtual void CreatePersistentUberGraphFrame(UObject* Obj, bool bCreateOnlyIfEmpty = false, bool bSkipSuperClass = false) const override {}
#else
	virtual void CreatePersistentUberGraphFrame(UObject* Obj, bool bCreateOnlyIfEmpty = false, bool bSkipSuperClass = false, UClass* OldClass = nullptr) const override {}
#endif
	virtual void DestroyPersistentUberGraphFrame(UObject* Obj, bool bSkipSuperClass = false) const override {}
	virtual void Link(FArchive& Ar, bool bRelinkExistingProperties) override { UClass::Link(Ar, bRelinkExistingProperties); }
	virtual void PurgeClass(bool bRecompilingOnLoad) override { UClass::PurgeClass(bRecompilingOnLoad);  }
	virtual void Bind() override { UClass::Bind(); }
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 24
	virtual UObject* FindArchetype(UClass* ArchetypeClass, const FName ArchetypeName) const override { return UClass::FindArchetype(ArchetypeClass, ArchetypeName);  }
#else
	virtual UObject* FindArchetype(const UClass* ArchetypeClass, const FName ArchetypeName) const override { return UClass::FindArchetype(ArchetypeClass, ArchetypeName); }
#endif
	// End UClass interface

public:
	TWeakPtr<FJavascriptContext> JavascriptContext;
};
