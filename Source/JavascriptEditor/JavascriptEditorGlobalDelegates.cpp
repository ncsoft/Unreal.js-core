#include "JavascriptEditorGlobalDelegates.h"
#include "AssetRegistryModule.h"
#include "EditorSupportDelegates.h"
#include "Editor/EditorEngine.h"
#include "Editor.h"
#include "GameDelegates.h"

#if WITH_EDITOR
void UJavascriptEditorGlobalDelegates::BeginDestroy()
{
	Super::BeginDestroy();

	UnbindAll();
}

#define DO_REFLECT() \
OP_REFLECT(NewCurrentLevel)\
OP_REFLECT(MapChange)\
OP_REFLECT(LayerChange)\
OP_REFLECT(ChangeEditorMode)\
OP_REFLECT(SurfProps)\
OP_REFLECT(SelectedProps)\
OP_REFLECT(FitTextureToSurface)\
OP_REFLECT(ActorPropertiesChange)\
OP_REFLECT(RefreshEditor)\
OP_REFLECT(RefreshAllBrowsers)\
OP_REFLECT(RefreshLayerBrowser)\
OP_REFLECT(RefreshPrimitiveStatsBrowser)\
OP_REFLECT(LoadSelectedAssetsIfNeeded)\
OP_REFLECT(DisplayLoadErrors)\
OP_REFLECT(PreBeginPIE)\
OP_REFLECT(BeginPIE)\
OP_REFLECT(PostPIEStarted)\
OP_REFLECT(EndPIE)\
OP_REFLECT(ResumePIE)\
OP_REFLECT(SingleStepPIE)\
OP_REFLECT(PropertySelectionChange)\
OP_REFLECT(PostLandscapeLayerUpdated)\
OP_REFLECT(PreSaveWorld)\
OP_REFLECT(PostSaveWorld)\
OP_REFLECT(OnFinishPickingBlueprintClass)\
OP_REFLECT(OnConfigureNewAssetProperties)\
OP_REFLECT(OnNewAssetCreated)\
OP_REFLECT(OnNewActorsDropped)\
OP_REFLECT(OnGridSnappingChanged)\
OP_REFLECT(OnLightingBuildStarted)\
OP_REFLECT(OnLightingBuildKept)\
OP_REFLECT(OnApplyObjectToActor)\
OP_REFLECT(OnFocusViewportOnActors)\
OP_REFLECT(OnMapOpened)\
OP_REFLECT(OnEditorCameraMoved)\
OP_REFLECT(OnDollyPerspectiveCamera)\
OP_REFLECT(OnShutdownPostPackagesSaved)\
OP_REFLECT(OnAssetsPreDelete)\
OP_REFLECT(OnAssetsDeleted)\
OP_REFLECT(OnActionAxisMappingsChanged)\
OP_REFLECT(OnAddLevelToWorld)

// UEditorEngine::OnObjectReimported is integrated with UImportSubsystem::OnAssetReimport
#define DO_REFLECT_IMPORT_SUBSYS() \
OP_REFLECT_IMPORT_SUBSYS(OnAssetPreImport)\
OP_REFLECT_IMPORT_SUBSYS(OnAssetPostImport)\
OP_REFLECT_IMPORT_SUBSYS(OnAssetReimport)\
OP_REFLECT_IMPORT_SUBSYS_FORWARD(OnAssetReimport, OnObjectReimported)

#define DO_REFLECT_ASSETREGISTRY() \
OP_REFLECT_ASSETREGISTRY(OnPathAdded)\
OP_REFLECT_ASSETREGISTRY(OnPathRemoved)\
OP_REFLECT_ASSETREGISTRY(OnAssetAdded)\
OP_REFLECT_ASSETREGISTRY(OnAssetRemoved)\
OP_REFLECT_ASSETREGISTRY(OnAssetRenamed)\
OP_REFLECT_ASSETREGISTRY(OnInMemoryAssetCreated)\
OP_REFLECT_ASSETREGISTRY(OnInMemoryAssetDeleted)\
OP_REFLECT_ASSETREGISTRY(OnFilesLoaded)\
OP_REFLECT_ASSETREGISTRY(OnFileLoadProgressUpdated)

#define DO_REFLECT_EDITORENGINE() \
OP_REFLECT_EDITORENGINE(OnBlueprintPreCompile)\
OP_REFLECT_EDITORENGINE(OnBlueprintCompiled)\
OP_REFLECT_EDITORENGINE(OnBlueprintReinstanced)\
OP_REFLECT_EDITORENGINE(OnClassPackageLoadedOrUnloaded)

#define DO_REFLECT_SUPPORT() \
OP_REFLECT_SUPPORT(RedrawAllViewports)\
OP_REFLECT_SUPPORT(CleanseEditor)\
OP_REFLECT_SUPPORT(WorldChange)

#define DO_REFLECT_GAME() \
OP_REFLECT_GAME(EndPlayMapDelegate)

FJavascriptAssetData::FJavascriptAssetData(const FAssetData& Source)
	: ObjectPath(Source.ObjectPath), PackageName(Source.PackageName), PackagePath(Source.PackagePath), AssetName(Source.AssetName), AssetClass(Source.AssetClass), ChunkIDs(Source.ChunkIDs), PackageFlags((int32)Source.PackageFlags), SourceAssetData(Source)
{
}

void UJavascriptEditorGlobalDelegates::Bind(FString Key)
{
	FDelegateHandle Handle;

	auto& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
#define OP_REFLECT(x) else if (Key == #x) { Handle = FEditorDelegates::x.AddUObject(this, &UJavascriptEditorGlobalDelegates::x); }
#define OP_REFLECT_IMPORT_SUBSYS(x) else if (Key == #x) { Handle = GEditor->GetEditorSubsystem<UImportSubsystem>()->x.AddUObject(this, &UJavascriptEditorGlobalDelegates::x); }
#define OP_REFLECT_IMPORT_SUBSYS_FORWARD(DelegateName, HandlerFunc) else if (Key == #HandlerFunc) { Handle = GEditor->GetEditorSubsystem<UImportSubsystem>()->DelegateName.AddUObject(this, &UJavascriptEditorGlobalDelegates::HandlerFunc); }
#define OP_REFLECT_ASSETREGISTRY(x) else if (Key == #x) { Handle = AssetRegistry.x().AddUObject(this, &UJavascriptEditorGlobalDelegates::x); }
#define OP_REFLECT_EDITORENGINE(x) else if (Key == #x) { Handle = Cast<UEditorEngine>(GEngine)->x().AddUObject(this, &UJavascriptEditorGlobalDelegates::x); }
#define OP_REFLECT_SUPPORT(x) else if (Key == #x) { Handle = FEditorSupportDelegates::x.AddUObject(this, &UJavascriptEditorGlobalDelegates::x); }
#define OP_REFLECT_GAME(x) else if (Key == #x) { Handle = FGameDelegates::Get().Get##x().AddUObject(this, &UJavascriptEditorGlobalDelegates::x); }
	if (false) {}
		DO_REFLECT()
		DO_REFLECT_IMPORT_SUBSYS()
		DO_REFLECT_ASSETREGISTRY()
		DO_REFLECT_EDITORENGINE()
		DO_REFLECT_SUPPORT()
		DO_REFLECT_GAME()
		;

	if (Handle.IsValid())
	{
		Handles.Add(Key, Handle);
	}
#undef OP_REFLECT
#undef OP_REFLECT_IMPORT_SUBSYS
#undef OP_REFLECT_IMPORT_SUBSYS_FORWARD
#undef OP_REFLECT_ASSETREGISTRY
#undef OP_REFLECT_EDITORENGINE
#undef OP_REFLECT_SUPPORT
#undef OP_REFLECT_GAME
}

void UJavascriptEditorGlobalDelegates::UnbindAll()
{
	for (auto Iter = Handles.CreateIterator(); Iter; ++Iter)
	{
		Unbind(Iter->Key);
	}
}

void UJavascriptEditorGlobalDelegates::Unbind(FString Key)
{
	auto Handle = Handles[Key];
	
#define OP_REFLECT(x) else if (Key == #x) { FEditorDelegates::x.Remove(Handle); }
#define OP_REFLECT_IMPORT_SUBSYS(x) else if (Key == #x) { if (GEditor != nullptr) { GEditor->GetEditorSubsystem<UImportSubsystem>()->x.Remove(Handle); } }
#define OP_REFLECT_IMPORT_SUBSYS_FORWARD(DelegateName, HandlerFunc) else if (Key == #HandlerFunc) { if (GEditor != nullptr) { GEditor->GetEditorSubsystem<UImportSubsystem>()->DelegateName.Remove(Handle); } }
#define OP_REFLECT_ASSETREGISTRY(x) else if (Key == #x) { if (FModuleManager::Get().IsModuleLoaded(TEXT("AssetRegistry"))) { FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get().x().Remove(Handle); } }
#define OP_REFLECT_EDITORENGINE(x) else if (Key == #x) { Cast<UEditorEngine>(GEngine)->x().Remove(Handle); }
#define OP_REFLECT_SUPPORT(x) else if (Key == #x) { FEditorSupportDelegates::x.Remove(Handle); }
#define OP_REFLECT_GAME(x) else if (Key == #x) { FGameDelegates::Get().Get##x().Remove(Handle); }
	if (false) {}
		DO_REFLECT()
		DO_REFLECT_IMPORT_SUBSYS()
		DO_REFLECT_ASSETREGISTRY()
		DO_REFLECT_EDITORENGINE()
		DO_REFLECT_SUPPORT()
		DO_REFLECT_GAME()
		;

	Handles.Remove(Key);
#undef OP_REFLECT
#undef OP_REFLECT_IMPORT_SUBSYS
#undef OP_REFLECT_IMPORT_SUBSYS_FORWARD
#undef OP_REFLECT_ASSETREGISTRY
#undef OP_REFLECT_EDITORENGINE
#undef OP_REFLECT_SUPPORT
#undef OP_REFLECT_GAME
}
#endif
