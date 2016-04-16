#pragma once

#include "JavascriptEditorGlobalDelegates.h"
#include "JavascriptEditorLibrary.generated.h"

UENUM()
namespace EJavasrciptUserInterfaceActionType
{
	enum Type
	{
		/** Momentary buttons or menu items.  These support enable state, and execute a delegate when clicked. */
		Button,

		/** Toggleable buttons or menu items that store on/off state.  These support enable state, and execute a delegate when toggled. */
		ToggleButton,

		/** Radio buttons are similar to toggle buttons in that they are for menu items that store on/off state.  However they should be used to indicate that menu items in a group can only be in one state */
		RadioButton,

		/** Similar to Button but will display a readonly checkbox next to the item. */
		Check
	};
}

USTRUCT(BlueprintType)
struct FJavascriptUICommand
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	FString Id;

	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	FString FriendlyName;

	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	FString Description;

	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	FInputChord DefaultChord;

	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	TEnumAsByte<EJavasrciptUserInterfaceActionType::Type> ActionType;
};


USTRUCT()
struct FJavascriptToolbarBuilder
{
	GENERATED_BODY()

#if WITH_EDITOR
	TSharedPtr<FToolBarBuilder> Handle;
#endif
};

USTRUCT()
struct FJavascriptUICommandList
{
	GENERATED_BODY()

#if WITH_EDITOR
	TSharedPtr<FUICommandList> Handle;
#endif
};

USTRUCT()
struct FJavascriptBindingContext
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	void Destroy()
	{
		if (Handle.IsValid())
		{
			FInputBindingManager::Get().RemoveContextByName(Handle->GetContextName());
			Handle.Reset();
		}
	}

	TSharedPtr<FBindingContext> Handle;
#endif
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
struct FJavascriptUICommandInfo
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	TSharedPtr<FUICommandInfo> Handle;
#endif
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
	static FJavascriptUICommandList CreateUICommandList();

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptToolbarBuilder CreateToolbarBuilder(FJavascriptUICommandList CommandList);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void BeginSection(FJavascriptToolbarBuilder& Builder, FName InExtensionHook);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void EndSection(FJavascriptToolbarBuilder& Builder);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void AddSeparator(FJavascriptToolbarBuilder& Builder);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptWorkspaceItem AddGroup(FJavascriptWorkspaceItem Parent, const FText& DisplayName);
	
	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptWorkspaceItem GetGroup(const FString& Name);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptBindingContext NewBindingContext(const FName InContextName, const FText& InContextDesc, const FName InContextParent, const FName InStyleSetName);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void Destroy(FJavascriptBindingContext Context);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptUICommandInfo UI_COMMAND_Function(FJavascriptBindingContext This, FJavascriptUICommand Command);
#endif
};
