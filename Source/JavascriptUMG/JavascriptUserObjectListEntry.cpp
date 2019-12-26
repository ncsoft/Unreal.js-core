#include "JavascriptUserObjectListEntry.h"
#if WITH_EDITOR
#include "Engine/Blueprint.h"
#endif

UJavascriptUserObjectListEntry::UJavascriptUserObjectListEntry(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
#if WITH_EDITOR
	// Create dummy UBlueprint instance.. see a UListViewBase::RebuildWidget()
	UClass* ClassObj = GetClass();

	if (ClassObj->ClassGeneratedBy == nullptr)
	{
		auto TransientBlueprint = NewObject<UBlueprint>(GetTransientPackage(), NAME_None, RF_Transient);
		ClassObj->ClassGeneratedBy = TransientBlueprint;
		TransientBlueprint->GeneratedClass = ClassObj;
	}
#endif
}
