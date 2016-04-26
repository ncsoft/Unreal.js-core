#pragma once

#include "JavascriptEditorGlobalDelegates.h"
#include "JavascriptMenuLibrary.h"
#include "JavascriptInputEventStateLibrary.h"
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

USTRUCT()
struct FJavascriptWorkspaceItem
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	TSharedPtr<FWorkspaceItem> Handle;
#endif
};

USTRUCT()
struct FJavascriptHitProxy
{
	GENERATED_BODY()

	class HHitProxy* HitProxy;
};

USTRUCT()
struct FJavascriptViewportClick
{
	GENERATED_BODY()

	FJavascriptViewportClick()
	{}

	FJavascriptViewportClick(const class FSceneView* View, class FEditorViewportClient* ViewportClient, FKey InKey, EInputEvent InEvent, int32 X, int32 Y)
		: Click(MakeShareable(new FViewportClick(View, ViewportClient, InKey, InEvent, X, Y)))
	{}

	TSharedPtr<FViewportClick> Click;	
};

USTRUCT()
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
	static ULandscapeLayerInfoObject* GetLayerInfoByName(ULandscapeInfo* LandscapeInfo, FName LayerName, ALandscapeProxy* Owner = NULL);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static bool GetLandscapeExtent(ULandscapeInfo* LandscapeInfo, int32& MinX, int32& MinY, int32& MaxX, int32& MaxY);
		
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void OpenPopupWindow(UWidget* Widget, const FVector2D& PopupDesiredSize, const FText& HeadingText);

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
	static void DeselectAll(USelection* Selection, UClass* InClass = NULL);

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

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void DrawWireDiamond(const FJavascriptPDI& PDI, const FTransform& Transform, float Size, const FLinearColor& InColor, ESceneDepthPriorityGroup DepthPriority);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void SetHitProxy(const FJavascriptPDI& PDI, const FName& Name);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static AActor* GetActor(const FJavascriptHitProxy& Proxy);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FName GetName(const FJavascriptHitProxy& Proxy);
#endif
};
