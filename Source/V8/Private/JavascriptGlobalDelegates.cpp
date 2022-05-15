#include "JavascriptGlobalDelegates.h"
#include "Misc/CoreDelegates.h"
#include "../../Launch/Resources/Version.h"

void UJavascriptGlobalDelegates::BeginDestroy()
{
	Super::BeginDestroy();

	UnbindAll();
}

#define DO_REFLECT() \
OP_REFLECT(PostLoadMapWithWorld)\
OP_REFLECT(PostDemoPlay)\
OP_REFLECT(PackageCreatedForLoad)

#define DO_REFLECT_CORE() \
OP_REFLECT_CORE(OnActorLabelChanged)

#define DO_REFLECT_WORLD() \
OP_REFLECT_WORLD(OnPostWorldCreation)\
OP_REFLECT_WORLD(OnWorldCleanup)\
OP_REFLECT_WORLD(OnPreWorldFinishDestroy)\
OP_REFLECT_WORLD(LevelAddedToWorld)\
OP_REFLECT_WORLD(LevelRemovedFromWorld)\
OP_REFLECT_WORLD(PostApplyLevelOffset)\
OP_REFLECT_WORLD(OnPreWorldInitialization)\
OP_REFLECT_WORLD(OnPostWorldInitialization)\
OP_REFLECT_WORLD(OnPostDuplicate)

#define DO_REFLECT_FUNCTION() \
OP_REFLECT_FUNCTION(PreGarbageCollectDelegate)\
OP_REFLECT_FUNCTION(PostGarbageCollect)

#if WITH_EDITOR
#define DO_REFLECT_EDITOR_ONLY() \
OP_REFLECT(OnPreObjectPropertyChanged)\
OP_REFLECT(OnObjectPropertyChanged)\
OP_REFLECT(OnObjectModified)\
OP_REFLECT(OnAssetLoaded)\
OP_REFLECT(OnObjectPreSave)
#else
#define DO_REFLECT_EDITOR_ONLY()
#endif

void UJavascriptGlobalDelegates::Bind(FString Key)
{
	FDelegateHandle Handle;
	
#define OP_REFLECT(x) else if (Key == #x) { Handle = FCoreUObjectDelegates::x.AddUObject(this, &UJavascriptGlobalDelegates::x); }
#define OP_REFLECT_CORE(x) else if (Key == #x) { Handle = FCoreDelegates::x.AddUObject(this, &UJavascriptGlobalDelegates::x); }
#define OP_REFLECT_WORLD(x) else if (Key == #x) { Handle = FWorldDelegates::x.AddUObject(this, &UJavascriptGlobalDelegates::x); }
#define OP_REFLECT_OLD(x) else if (Key == #x) { Handle = FCoreUObjectDelegates::x.AddUObject(this, &UJavascriptGlobalDelegates::x##_Old); }
#define OP_REFLECT_FUNCTION(x) else if (Key == #x) { Handle = FCoreUObjectDelegates::Get##x().AddUObject(this, &UJavascriptGlobalDelegates::x); }

	if (false) {}
	DO_REFLECT()
	DO_REFLECT_CORE()
	DO_REFLECT_WORLD()
	DO_REFLECT_EDITOR_ONLY()
	DO_REFLECT_FUNCTION()

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 13
	OP_REFLECT_OLD(PreLoadMap)
#else
	OP_REFLECT(PreLoadMap)
#endif	

	if (Handle.IsValid())
	{
		Handles.Add(Key, Handle);
	}
#undef OP_REFLECT
#undef OP_REFLECT_CORE
#undef OP_REFLECT_WORLD
#undef OP_REFLECT_OLD
#undef OP_REFLECT_FUNCTION
}

void UJavascriptGlobalDelegates::UnbindAll()
{
	for (auto Iter = Handles.CreateIterator(); Iter; ++Iter)
	{
		Unbind(Iter->Key);		
	}
}

void UJavascriptGlobalDelegates::Unbind(FString Key)
{
	auto Handle = Handles[Key];

#define OP_REFLECT(x) else if (Key == #x) { FCoreUObjectDelegates::x.Remove(Handle); }
#define OP_REFLECT_CORE(x) else if (Key == #x) { FCoreDelegates::x.Remove(Handle); }
#define OP_REFLECT_WORLD(x) else if (Key == #x) { FWorldDelegates::x.Remove(Handle); }
#define OP_REFLECT_FUNCTION(x) else if (Key == #x) { FCoreUObjectDelegates::Get##x().Remove(Handle); }
	if (false) {}
	DO_REFLECT()
	DO_REFLECT_CORE()
	DO_REFLECT_WORLD()
	DO_REFLECT_FUNCTION()
	DO_REFLECT_EDITOR_ONLY()
	OP_REFLECT(PreLoadMap)

	Handles.Remove(Key);
#undef OP_REFLECT
#undef OP_REFLECT_CORE
#undef OP_REFLECT_WORLD
#undef OP_REFLECT_FUNCTION
}
