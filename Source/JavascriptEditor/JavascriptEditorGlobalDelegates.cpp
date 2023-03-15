#include "JavascriptEditorGlobalDelegates.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Interfaces/IMainFrameModule.h"
#include "EditorSupportDelegates.h"
#include "Editor/EditorEngine.h"
#include "Editor.h"
#include "GameDelegates.h"
#include "Engine/Selection.h"

#if WITH_EDITOR
struct FJavascriptSupportDelegates
{
	/**  */
	DECLARE_MULTICAST_DELEGATE_TwoParams(FOnConsoleCommandJSDelegate, const TArray<FString>&, UWorld*);

	static FOnConsoleCommandJSDelegate OnConsoleCommandJS;
};

FJavascriptSupportDelegates::FOnConsoleCommandJSDelegate FJavascriptSupportDelegates::OnConsoleCommandJS;

void JSConsoleCmd(const TArray<FString>& Args, UWorld* InWorld)
{
	if (FJavascriptSupportDelegates::OnConsoleCommandJS.IsBound())
	{
		FJavascriptSupportDelegates::OnConsoleCommandJS.Broadcast(Args, InWorld);
	}	
}

static FAutoConsoleCommandWithWorldAndArgs AddJSConsoleCmd(
	TEXT("jsc"),
	TEXT("javascript console command. ex)jsc command arg1.."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&JSConsoleCmd)
);

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
OP_REFLECT(PrePIEEnded)\
OP_REFLECT(EndPIE)\
OP_REFLECT(ResumePIE)\
OP_REFLECT(SingleStepPIE)\
OP_REFLECT(PropertySelectionChange)\
OP_REFLECT(PostLandscapeLayerUpdated)\
OP_REFLECT(PreSaveWorldWithContext)\
OP_REFLECT(PostSaveWorldWithContext)\
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
OP_REFLECT(OnAddLevelToWorld)\
OP_REFLECT(OnEditCutActorsBegin)\
OP_REFLECT(OnEditCutActorsEnd)\
OP_REFLECT(OnEditCopyActorsBegin)\
OP_REFLECT(OnEditCopyActorsEnd)\
OP_REFLECT(OnEditPasteActorsBegin)\
OP_REFLECT(OnEditPasteActorsEnd)\
OP_REFLECT(OnDeleteActorsBegin)\
OP_REFLECT(OnDeleteActorsEnd)\
OP_REFLECT(OnDuplicateActorsBegin)\
OP_REFLECT(OnDuplicateActorsEnd)

#define DO_REFLECT_MAINFRAME() \
OP_REFLECT_MAINFRAME(OnMainFrameCreationFinished)

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
OP_REFLECT_EDITORENGINE(OnClassPackageLoadedOrUnloaded)\
OP_REFLECT_EDITORENGINE(OnLevelActorAdded)\
OP_REFLECT_EDITORENGINE(OnLevelActorDeleted)

#define DO_REFLECT_SUPPORT() \
OP_REFLECT_SUPPORT(RedrawAllViewports)\
OP_REFLECT_SUPPORT(CleanseEditor)\
OP_REFLECT_SUPPORT(WorldChange)

#define DO_REFLECT_GAME() \
OP_REFLECT_GAME(EndPlayMapDelegate)

#define DO_REFLECT_SELECTION() \
OP_REFLECT_SELECTION(SelectionChangedEvent)\
OP_REFLECT_SELECTION(SelectObjectEvent)\
OP_REFLECT_SELECTION(SelectNoneEvent)\

#define DO_REFLECT_JAVASCRIPT() \
OP_REFLECT_JAVASCRIPT(OnConsoleCommandJS) \

#define DO_REFLECT_SLATEAPPLICATION() \
OP_REFLECT_SLATEAPPLICATION(OnApplicationPreInputKeyDownListener)\

FJavascriptAssetData::FJavascriptAssetData(const FAssetData& Source)
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
	: ObjectPath(*Source.GetSoftObjectPath().ToString())
#else
	: ObjectPath(Source.ObjectPath)
#endif
	, PackageName(Source.PackageName), PackagePath(Source.PackagePath), AssetName(Source.AssetName)
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
	, AssetClass(*Source.AssetClassPath.ToString())
	, ChunkIDs(Source.GetChunkIDs())
#else
	, AssetClass(Source.AssetClass)
	, ChunkIDs(Source.ChunkIDs)
#endif
	, PackageFlags((int32)Source.PackageFlags), SourceAssetData(Source)
{
}

void UJavascriptEditorGlobalDelegates::Bind(FString Key)
{
	FDelegateHandle Handle;

	auto& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry")).Get();
	auto& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame")).Get();
#define OP_REFLECT(x) else if (Key == #x) { Handle = FEditorDelegates::x.AddUObject(this, &UJavascriptEditorGlobalDelegates::x); }
#define OP_REFLECT_IMPORT_SUBSYS(x) else if (Key == #x) { Handle = GEditor->GetEditorSubsystem<UImportSubsystem>()->x.AddUObject(this, &UJavascriptEditorGlobalDelegates::x); }
#define OP_REFLECT_IMPORT_SUBSYS_FORWARD(DelegateName, HandlerFunc) else if (Key == #HandlerFunc) { Handle = GEditor->GetEditorSubsystem<UImportSubsystem>()->DelegateName.AddUObject(this, &UJavascriptEditorGlobalDelegates::HandlerFunc); }
#define OP_REFLECT_ASSETREGISTRY(x) else if (Key == #x) { Handle = AssetRegistry.x().AddUObject(this, &UJavascriptEditorGlobalDelegates::x); }
#define OP_REFLECT_MAINFRAME(x) else if (Key == #x) { Handle = MainFrame.x().AddUObject(this, &UJavascriptEditorGlobalDelegates::x); }
#define OP_REFLECT_EDITORENGINE(x) else if (Key == #x) { Handle = Cast<UEditorEngine>(GEngine)->x().AddUObject(this, &UJavascriptEditorGlobalDelegates::x); }
#define OP_REFLECT_SUPPORT(x) else if (Key == #x) { Handle = FEditorSupportDelegates::x.AddUObject(this, &UJavascriptEditorGlobalDelegates::x); }
#define OP_REFLECT_GAME(x) else if (Key == #x) { Handle = FGameDelegates::Get().Get##x().AddUObject(this, &UJavascriptEditorGlobalDelegates::x); }
#define OP_REFLECT_SELECTION(x) else if (Key == #x) { Handle = USelection::x.AddUObject(this, &UJavascriptEditorGlobalDelegates::x); }
#define OP_REFLECT_JAVASCRIPT(x) else if (Key == #x) { Handle = FJavascriptSupportDelegates::x.AddUObject(this, &UJavascriptEditorGlobalDelegates::x); }
#define OP_REFLECT_SLATEAPPLICATION(x) else if (Key == #x) { Handle = FSlateApplication::Get().x().AddUObject(this, &UJavascriptEditorGlobalDelegates::x); }
	if (false) {}
		DO_REFLECT()
		DO_REFLECT_IMPORT_SUBSYS()
		DO_REFLECT_ASSETREGISTRY()
		DO_REFLECT_MAINFRAME()
		DO_REFLECT_EDITORENGINE()
		DO_REFLECT_SUPPORT()
		DO_REFLECT_GAME()
		DO_REFLECT_SELECTION()
		DO_REFLECT_JAVASCRIPT()
		DO_REFLECT_SLATEAPPLICATION()
		;

	if (Handle.IsValid())
	{
		Handles.Add(Key, Handle);
	}
#undef OP_REFLECT
#undef OP_REFLECT_IMPORT_SUBSYS
#undef OP_REFLECT_IMPORT_SUBSYS_FORWARD
#undef OP_REFLECT_ASSETREGISTRY
#undef OP_REFLECT_MAINFRAME
#undef OP_REFLECT_EDITORENGINE
#undef OP_REFLECT_SUPPORT
#undef OP_REFLECT_GAME
#undef OP_REFLECT_SELECTION
#undef OP_REFLECT_JAVASCRIPT
#undef OP_REFLECT_SLATEAPPLICATION
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
#define OP_REFLECT_MAINFRAME(x) else if (Key == #x) { if (FModuleManager::Get().IsModuleLoaded(TEXT("MainFrame"))) { FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame")).Get().x().Remove(Handle); } }
#define OP_REFLECT_EDITORENGINE(x) else if (Key == #x) { Cast<UEditorEngine>(GEngine)->x().Remove(Handle); }
#define OP_REFLECT_SUPPORT(x) else if (Key == #x) { FEditorSupportDelegates::x.Remove(Handle); }
#define OP_REFLECT_GAME(x) else if (Key == #x) { FGameDelegates::Get().Get##x().Remove(Handle); }
#define OP_REFLECT_SELECTION(x) else if (Key == #x) { USelection::x.Remove(Handle); }
#define OP_REFLECT_JAVASCRIPT(x) else if (Key == #x) { FJavascriptSupportDelegates::x.Remove(Handle); }
#define OP_REFLECT_SLATEAPPLICATION(x) else if (Key == #x) { if (FSlateApplication::IsInitialized()) { FSlateApplication::Get().x().Remove(Handle); } }
	if (false) {}
		DO_REFLECT()
		DO_REFLECT_IMPORT_SUBSYS()
		DO_REFLECT_ASSETREGISTRY()
		DO_REFLECT_MAINFRAME()
		DO_REFLECT_EDITORENGINE()
		DO_REFLECT_SUPPORT()
		DO_REFLECT_GAME()
		DO_REFLECT_SELECTION()
		DO_REFLECT_JAVASCRIPT()
		DO_REFLECT_SLATEAPPLICATION()
		;

	Handles.Remove(Key);
#undef OP_REFLECT
#undef OP_REFLECT_IMPORT_SUBSYS
#undef OP_REFLECT_IMPORT_SUBSYS_FORWARD
#undef OP_REFLECT_ASSETREGISTRY
#undef OP_REFLECT_MAINFRAME
#undef OP_REFLECT_EDITORENGINE
#undef OP_REFLECT_SUPPORT
#undef OP_REFLECT_GAME
#undef OP_REFLECT_SELECTION
#undef OP_REFLECT_JAVASCRIPT
#undef OP_REFLECT_SLATEAPPLICATION
}
#endif
