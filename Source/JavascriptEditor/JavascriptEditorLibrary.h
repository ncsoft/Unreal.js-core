#pragma once

#include "JavascriptEditorGlobalDelegates.h"
#include "LandscapeProxy.h"
#include "JavascriptMenuLibrary.h"
#include "JavascriptInputEventStateLibrary.h"
#include "Editor/Transactor.h"
#include "Engine/Brush.h"
#include "Framework/Docking/WorkspaceItem.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Engine/CurveTable.h"
#include "JavascriptEditorLibrary.generated.h"

UENUM()
enum class EJavascriptWidgetMode : uint8
{
	WM_Translate,
	WM_TranslateRotateZ,
	WM_2D,
	WM_Rotate,
	WM_Scale,
	WM_Max,
	WM_None = 255,
};

USTRUCT(BlueprintType)
struct FJavascriptTransaction
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	const FTransaction* Transaction;

	const FTransaction* operator -> () const
	{
		return Transaction;
	}
#endif
};

USTRUCT(BlueprintType)
struct FJavascriptWorkspaceItem
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	TSharedPtr<FWorkspaceItem> Handle;
#endif
};

USTRUCT(BlueprintType)
struct FJavascriptHitProxy
{
	GENERATED_BODY()

	class HHitProxy* HitProxy;
};

USTRUCT(BlueprintType)
struct FJavascriptViewportClick
{
	GENERATED_BODY()

	FJavascriptViewportClick()
	{}

	FJavascriptViewportClick(const FViewportClick* Other)
		: Click(Other)
	{}

	const FViewportClick* Click;
};

USTRUCT(BlueprintType)
struct FJavascriptPDI
{
	GENERATED_BODY()

	FJavascriptPDI()
	{}

	FJavascriptPDI(FPrimitiveDrawInterface* InPDI)
		: PDI(InPDI)
	{}

	FPrimitiveDrawInterface* PDI;
};

// forward decl
class FExtensibilityManager;

USTRUCT(BlueprintType)
struct FJavascriptExtensibilityManager
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	FExtensibilityManager* operator -> () const
	{
		return Handle.Get();
	}

	TSharedPtr<FExtensibilityManager> Handle;
	TArray<class UJavascriptLazyExtenderDelegates*> LazyExtenders;
#endif
};

USTRUCT(BlueprintType)
struct FJavascriptExtenderParameter
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Scripting | Javascript")
	FJavascriptUICommandList CommandList;

	UPROPERTY(BlueprintReadOnly, Category = "Scripting | Javascript")
	TArray<UObject*> EditingObjects;

	UPROPERTY(BlueprintReadWrite, Category = "Scripting | Javascript")
	FJavascriptExtender Extender;
};

/** The severity of the message type */
UENUM()
namespace EJavascriptMessageSeverity
{
	/** Ordered according to their severity */
	enum Type
	{
		CriticalError = 0,
		Error = 1,
		PerformanceWarning = 2,
		Warning = 3,
		Info = 4,	// Should be last
	};
}

/**
* The RHI's feature level indicates what level of support can be relied upon.
* Note: these are named after graphics API's like ES2 but a feature level can be used with a different API (eg ERHIFeatureLevel::ES2 on D3D11)
* As long as the graphics API supports all the features of the feature level (eg no ERHIFeatureLevel::SM5 on OpenGL ES2)
*/

UENUM()
namespace EJavascriptRHIFeatureLevel
{
	enum Type
	{
		/** Feature level defined by the core capabilities of OpenGL ES2. */
		ES2,
		/** Feature level defined by the core capabilities of OpenGL ES3.1 & Metal/Vulkan. */
		ES3_1,
		/** Feature level defined by the capabilities of DX10 Shader Model 4. */
		SM4,
		/** Feature level defined by the capabilities of DX11 Shader Model 5. */
		SM5,
		Num
	};
}

UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptLazyExtenderDelegates : public UObject
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(FJavascriptExtender, FJavascriptGetExtender, FJavascriptUICommandList, List, const TArray<UObject*>, EditingObjects);

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FJavascriptGetExtender GetExtender;
};

/**
 *
 */
UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptEditorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FIntPoint GetClickPos(const FJavascriptViewportClick& Click);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FKey GetKey(const FJavascriptViewportClick& Click);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static EInputEvent GetEvent(const FJavascriptViewportClick& Click);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static bool IsControlDown(const FJavascriptViewportClick& Click);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static bool IsShiftDown(const FJavascriptViewportClick& Click);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static bool IsAltDown(const FJavascriptViewportClick& Click);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FVector GetOrigin(const FJavascriptViewportClick& Click);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FVector GetDirection(const FJavascriptViewportClick& Click);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static bool GetWorldPositionFromViewportClick(const AActor* Actor, const FJavascriptViewportClick& Click, FHitResult& OutHitResult);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static ULandscapeInfo* GetLandscapeInfo(ALandscape* Landscape, bool bSpawnNewActor);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void SetHeightmapDataFromMemory(ULandscapeInfo* LandscapeInfo, int32 MinX, int32 MinY, int32 MaxX, int32 MaxY);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void GetHeightmapDataToMemory(ULandscapeInfo* LandscapeInfo, int32 MinX, int32 MinY, int32 MaxX, int32 MaxY);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void SetAlphamapDataFromMemory(ULandscapeInfo* LandscapeInfo, ULandscapeLayerInfoObject* LayerInfo, int32 MinX, int32 MinY, int32 MaxX, int32 MaxY, ELandscapeLayerPaintingRestriction PaintingRestriction  = ELandscapeLayerPaintingRestriction::None);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void GetAlphamapDataToMemory(ULandscapeInfo* LandscapeInfo, ULandscapeLayerInfoObject* LayerInfo, int32 MinX, int32 MinY, int32 MaxX, int32 MaxY);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static ULandscapeLayerInfoObject* GetLayerInfoByName(ULandscapeInfo* LandscapeInfo, FName LayerName, ALandscapeProxy* Owner = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static bool GetLandscapeExtent(ULandscapeInfo* LandscapeInfo, int32& MinX, int32& MinY, int32& MaxX, int32& MaxY);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void GetAllTagsByAssetData(const FAssetData& AssetData, TArray<FName>& OutArray);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool GetTagValueByAssetData(const FAssetData& AssetData, const FName& Name, FString& OutValue);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void GetAllTags(const FJavascriptAssetData& AssetData, TArray<FName>& OutArray);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool GetTagValue(const FJavascriptAssetData& AssetData, const FName& Name, FString& OutValue);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static UClass* GetClass(const FJavascriptAssetData& AssetData);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static UPackage* GetPackage(const FJavascriptAssetData& AssetData);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static UObject* GetAsset(const FJavascriptAssetData& AssetData);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool IsAssetLoaded(const FJavascriptAssetData& AssetData);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool EditorDestroyActor(UWorld* World, AActor* Actor, bool bShouldModifyLevel);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool ConditionalBeginDestroybyUObject(UObject* TargetObject);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void SetIsTemporarilyHiddenInEditor(AActor* Actor, bool bIsHidden);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static ABrush* GetDefaultBrush(UWorld* World);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool Build(UBrushBuilder* Builder, UWorld* InWorld, ABrush* InBrush = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void Select(USelection* Selection, UObject* InObject);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void Deselect(USelection* Selection, UObject* InObject);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void ToggleSelect(USelection* Selection, UObject* InObject);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void DeselectAll(USelection* Selection, UClass* InClass = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static int32 GetSelectedObjects(USelection* Selection, TArray<UObject*>& Out);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static ABrush* csgAdd(ABrush* DefaultBrush, int32 PolyFlags, EBrushType BrushType);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void ModifyObject(UObject* Object, bool bAlwaysMarkDirty = false);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void InvalidateModelGeometry(UWorld* World, ULevel* InLevel);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void UpdateModelComponents(ULevel* Level);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptWorkspaceItem AddGroup(FJavascriptWorkspaceItem Parent, const FText& DisplayName);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptWorkspaceItem GetGroup(const FString& Name);

	UFUNCTION(BlueprintCallable, Category = "Javascript | PDI")
	static void DrawWireBox(const FJavascriptPDI& PDI, const FBox& Box, const FLinearColor& Color, ESceneDepthPriorityGroup DepthPriority, float Thickness, float DepthBias, bool bScreenSpace);

	UFUNCTION(BlueprintCallable, Category = "Javascript | PDI")
	static void DrawWireBox2(const FJavascriptPDI& PDI, const FTransform& Matrix, const FBox& Box, const FLinearColor& Color, ESceneDepthPriorityGroup DepthPriority, float Thickness, float DepthBias, bool bScreenSpace);

	UFUNCTION(BlueprintCallable, Category = "Javascript | PDI")
	static void DrawCircle(const FJavascriptPDI& PDI, const FVector& Base, const FVector& X, const FVector& Y, const FLinearColor& Color, float Radius, int32 NumSides, ESceneDepthPriorityGroup DepthPriority, float Thickness, float DepthBias, bool bScreenSpace);

	UFUNCTION(BlueprintCallable, Category = "Javascript | PDI")
	static void DrawArc(const FJavascriptPDI& PDI, const FVector Base, const FVector X, const FVector Y, const float MinAngle, const float MaxAngle, const float Radius, const int32 Sections, const FLinearColor& Color, ESceneDepthPriorityGroup DepthPriority);

	UFUNCTION(BlueprintCallable, Category = "Javascript | PDI")
	static void DrawWireSphere(const FJavascriptPDI& PDI, const FVector& Base, const FLinearColor& Color, float Radius, int32 NumSides, ESceneDepthPriorityGroup DepthPriority, float Thickness, float DepthBias, bool bScreenSpace);

	UFUNCTION(BlueprintCallable, Category = "Javascript | PDI")
	static void DrawWireSphere2(const FJavascriptPDI& PDI, const FTransform& Transform, const FLinearColor& Color, float Radius, int32 NumSides, ESceneDepthPriorityGroup DepthPriority, float Thickness, float DepthBias, bool bScreenSpace);

	UFUNCTION(BlueprintCallable, Category = "Javascript | PDI")
	static void DrawWireSphereAutoSides(const FJavascriptPDI& PDI, const FVector& Base, const FLinearColor& Color, float Radius, ESceneDepthPriorityGroup DepthPriority, float Thickness, float DepthBias, bool bScreenSpace);

	UFUNCTION(BlueprintCallable, Category = "Javascript | PDI")
	static void DrawWireSphereAutoSides2(const FJavascriptPDI& PDI, const FTransform& Transform, const FLinearColor& Color, float Radius, ESceneDepthPriorityGroup DepthPriority, float Thickness, float DepthBias, bool bScreenSpace);

	UFUNCTION(BlueprintCallable, Category = "Javascript | PDI")
	static void DrawWireCylinder(const FJavascriptPDI& PDI, const FVector& Base, const FVector& X, const FVector& Y, const FVector& Z, const FLinearColor& Color, float Radius, float HalfHeight, int32 NumSides, ESceneDepthPriorityGroup DepthPriority, float Thickness, float DepthBias, bool bScreenSpace);

	UFUNCTION(BlueprintCallable, Category = "Javascript | PDI")
	static void DrawWireCapsule(const FJavascriptPDI& PDI, const FVector& Base, const FVector& X, const FVector& Y, const FVector& Z, const FLinearColor& Color, float Radius, float HalfHeight, int32 NumSides, ESceneDepthPriorityGroup DepthPriority, float Thickness, float DepthBias, bool bScreenSpace);

	UFUNCTION(BlueprintCallable, Category = "Javascript | PDI")
	static void DrawWireChoppedCone(const FJavascriptPDI& PDI, const FVector& Base, const FVector& X, const FVector& Y, const FVector& Z, const FLinearColor& Color, float Radius, float TopRadius, float HalfHeight, int32 NumSides, ESceneDepthPriorityGroup DepthPriority);

	UFUNCTION(BlueprintCallable, Category = "Javascript | PDI")
	static void DrawWireCone(const FJavascriptPDI& PDI, TArray<FVector>& Verts, const FTransform& Transform, float ConeRadius, float ConeAngle, int32 ConeSides, const FLinearColor& Color, ESceneDepthPriorityGroup DepthPriority, float Thickness, float DepthBias, bool bScreenSpace);

	UFUNCTION(BlueprintCallable, Category = "Javascript | PDI")
	static void DrawWireSphereCappedCone(const FJavascriptPDI& PDI, const FTransform& Transform, float ConeRadius, float ConeAngle, int32 ConeSides, int32 ArcFrequency, int32 CapSegments, const FLinearColor& Color, ESceneDepthPriorityGroup DepthPriority);

	UFUNCTION(BlueprintCallable, Category = "Javascript | PDI")
	static void DrawOrientedWireBox(const FJavascriptPDI& PDI, const FVector& Base, const FVector& X, const FVector& Y, const FVector& Z, FVector Extent, const FLinearColor& Color, ESceneDepthPriorityGroup DepthPriority, float Thickness, float DepthBias, bool bScreenSpace);

	UFUNCTION(BlueprintCallable, Category = "Javascript | PDI")
	static void DrawDirectionalArrow(const FJavascriptPDI& PDI, const FTransform& ArrowToWorld, const FLinearColor& InColor, float Length, float ArrowSize, ESceneDepthPriorityGroup DepthPriority, float Thickness);

	UFUNCTION(BlueprintCallable, Category = "Javascript | PDI")
	static void DrawConnectedArrow(const FJavascriptPDI& PDI, const FTransform& ArrowToWorld, const FLinearColor& Color, float ArrowHeight, float ArrowWidth, ESceneDepthPriorityGroup DepthPriority, float Thickness, int32 NumSpokes);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void DrawWireStar(const FJavascriptPDI& PDI, const FVector& Position, float Size, const FLinearColor& Color, ESceneDepthPriorityGroup DepthPriority);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void DrawDashedLine(const FJavascriptPDI& PDI, const FVector& Start, const FVector& End, const FLinearColor& Color, float DashSize, ESceneDepthPriorityGroup DepthPriority, float DepthBias);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void DrawWireDiamond(const FJavascriptPDI& PDI, const FTransform& Transform, float Size, const FLinearColor& InColor, ESceneDepthPriorityGroup DepthPriority);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void DrawPolygon(const FJavascriptPDI& PDI, const TArray<FVector>& Verts, const FLinearColor& InColor, ESceneDepthPriorityGroup DepthPriority, EJavascriptRHIFeatureLevel::Type RHIFeatureLevel);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void SetHitProxy(const FJavascriptPDI& PDI, const FName& Name);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static AActor* GetActor(const FJavascriptHitProxy& Proxy);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FName GetName(const FJavascriptHitProxy& Proxy);

	/**
	* Returns this actor's current label.  Actor labels are only available in development builds.
	* @return	The label text
	*/
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FString GetActorLabel(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void SetActorLabel(AActor* Actor, const FString& NewActorLabel, bool bMarkDirty);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void ClearActorLabel(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static bool SetActorLocation(AActor* Actor, FVector NewLocation, bool bSweep, FHitResult& SweepHitResult, bool bTeleport);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FVector GetActorLocation(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FRotator GetActorRotation(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static bool IsActorLabelEditable(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void SetFolderPath(AActor* Actor, const FName& NewFolderPath);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void SetFolderPath_Recursively(AActor* Actor, const FName& NewFolderPath);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FName GetFolderPath(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void BroadcastHotReload();

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void BroadcastAssetCreated(UObject* NewAsset);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static bool IsActive(UTransactor* Transactor);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static int32 GetQueueLength(UTransactor* Transactor);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FJavascriptTransaction GetTransaction(UTransactor* Transactor, int32 QueueIndex);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FText GetTitle(const FJavascriptTransaction& Transaction);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FString GetContext(const FJavascriptTransaction& Transaction);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static UObject* GetPrimaryObject(const FJavascriptTransaction& Transaction);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void EditorAddModalWindow(FJavascriptSlateWidget Widget);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FJavascriptSlateWidget GetRootWindow();

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void CreatePropertyEditorToolkit(TArray<UObject*> ObjectsForPropertiesMenu);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FJavascriptExtensibilityManager GetMenuExtensibilityManager(FName What);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FJavascriptExtensibilityManager GetToolBarExtensibilityManager(FName What);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FJavascriptUICommandList GetLevelEditorActions();

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void AddExtender(FJavascriptExtensibilityManager Manager, FJavascriptExtender Extender);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void RemoveExtender(FJavascriptExtensibilityManager Manager, FJavascriptExtender Extender);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void AddLazyExtender(FJavascriptExtensibilityManager Manager, UJavascriptLazyExtenderDelegates* Delegates);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void RemoveAllLazyExtender(FJavascriptExtensibilityManager Manager);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static bool SavePackage(UPackage* Package, FString FileName);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static bool DeletePackage(UPackage* Package);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void CreateBrushForVolumeActor(AVolume* NewActor, UBrushBuilder* BrushBuilder);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static UWorld* FindWorldInPackage(UPackage* Package);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FString ExportNavigation(UWorld* InWorld, FString Path);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void RequestEndPlayMapInPIE();

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void RemoveLevelInstance(UWorld* World);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void AddWhitelistedObject(UObject* InObject);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void PostEditChange(UObject* InObject);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool MarkPackageDirty(UObject* InObject);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void CreateLogListing(const FName& InLogName, const FText& InLabel);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptSlateWidget CreateLogListingWidget(const FName& InLogName);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void AddLogListingMessage(const FName& InLogName, EJavascriptMessageSeverity::Type InSeverity, const FString& LogText);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static UEditorEngine* GetEngine();

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static UClass* GetParentClassOfBlueprint(UBlueprint* Blueprint);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void AddComponentsToBlueprint(UBlueprint* Blueprint, const TArray<UActorComponent*>& Components, bool bHarvesting = false, class UActorComponent* OptionalNewRootComponent = nullptr, bool bKeepMobility = false);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void RemoveComponentFromBlueprint(UBlueprint* Blueprint, UActorComponent* RemoveComponent, bool bPromoteChildren = false);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void CompileBlueprint(UBlueprint* Blueprint);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool OpenEditorForAsset(UObject* Asset);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void OpenEditorForAssetByPath(const FString& AssetPathName, const FString& ObjectName);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static TArray<FAssetData> GetAssetsByType(const TArray<FString>& Types, bool bRecursiveClasses = true);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static int32 ReplaceAnimNotifyClass(class UAnimSequenceBase* Sequence, FString NotifyName, FString NewNotifyName, UObject* NewNotifyClass);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void DownloadImageFromUrl(const FString& ImageUrl, class UAsyncTaskDownloadImage* Callback);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool LoadImageFromDiskAsync(const FString& ImagePath, class UAsyncTaskDownloadImage* Callback);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool OpenFileDialog(const class UJavascriptWindow* WindowHandle, const FString& DialogTitle, const FString& DefaultPath, const FString& DefaultFile, const FString& FileTypes, int32 Flags, TArray<FString>& OutFilenames);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool OpenDirectoryDialog(const class UJavascriptWindow* WindowHandle, const FString& DialogTitle, const FString& DefaultPath, FString& OutFolderName);
	
	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void OpenCreateBlueprintFromActorDialog(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool SaveFileDialog(const class UJavascriptWindow* WindowHandle, const FString& DialogTitle, const FString& DefaultPath, const FString& DefaultFile, const FString& FileTypes, int32 Flags, TArray<FString>& OutFilenames);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool LoadFileToIntArray(FString Path, TArray<uint8>& FileData);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool LoadFileToString(FString Path, FString& Data);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FString GetKeyNameByKeyEvent(const FKeyEvent& Event);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool GetIsControlDownByKeyEvent(const FKeyEvent& Event);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool GetIsShiftDownByKeyEvent(const FKeyEvent& Event);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool GetIsAltDownByKeyEvent(const FKeyEvent& Event);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FString GetDataTableAsJSON(class UDataTable* InDataTable, uint8 InDTExportFlags = 0);

	UFUNCTION()
	static void AddRichCurve(UCurveTable* InCurveTable, const FName& Key, const FRichCurve& InCurve);

	UFUNCTION()
	static bool FindRichCurve(UCurveTable* InCurveTable, const FName& Key, FRichCurve& OutCurve);

	UFUNCTION()
	static void NotifyUpdateCurveTable(UCurveTable* InCurveTable);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static bool HasMetaData(UField* Field, const FString& Key);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static UWorld* GetEditorPlayWorld();

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static bool ToggleIsExecuteTestModePIE();

	UFUNCTION(BlueprintPure, Category = "Scripting|Javascript")
	static bool GetIsExecuteTestModePIE();

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static int32 GetUniqueID(UObject* InObject);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void SetActorLabelUnique(AActor* Actor, const FString& NewActorLabel, const TArray<FString>& InExistingActorLabels);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool EditorExec(UWorld* World, FString Cmd);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptTextProperty FromStringTable(const FName InTableId, const FString& InKey);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static float GetAverageFPS();

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static float GetAverageMS();

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool CheckActivatedStatGroup(FName GroupName);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FString GetSourceControlStatusText();

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static class UJavascriptEditorObjectManager* GetEditorObjectManager();

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FString GetHostName();

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FString GetIPAddress();
#endif
};
