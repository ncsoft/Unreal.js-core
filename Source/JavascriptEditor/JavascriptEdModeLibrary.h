#pragma once

#include "JavascriptEditorLibrary.h"
#include "JavascriptEdModeLibrary.generated.h"

class FEdMode;

// Extended axis enum for more specialized usage
USTRUCT(BlueprintType)
struct FJavascriptEditorModeTools
{
	GENERATED_BODY()

	FJavascriptEditorModeTools()
	{}

	FJavascriptEditorModeTools(FEditorModeTools* InTools)
		: Tools(InTools)
	{}

	FEditorModeTools* Tools;

	FEditorModeTools* operator -> ()
	{
		return Tools;
	}
};

USTRUCT(BlueprintType)
struct FJavascriptEditorMode
{
	GENERATED_BODY()

	FJavascriptEditorMode() {}
	FJavascriptEditorMode(const FEdMode* InEdMode)
		: EdMode(const_cast<FEdMode*>(InEdMode)), bIsConst(true)
	{}
	FJavascriptEditorMode(FEdMode* InEdMode)
		: EdMode(InEdMode), bIsConst(false)
	{}
	FEdMode* operator -> ()
	{
		return EdMode;
	}
		
	FEdMode* EdMode;
	bool bIsConst;
};


USTRUCT(BlueprintType)
struct FJavascriptEdViewport
{
	GENERATED_BODY()

	FJavascriptEdViewport() {}
	FJavascriptEdViewport(FEditorViewportClient* InViewportClient, FViewport* InViewport)
		: ViewportClient(InViewportClient), Viewport(InViewport)
	{}

	FEditorViewportClient* ViewportClient;
	FViewport* Viewport;
};
/**
 * 
 */
UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptEdModeLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

#if WITH_EDITOR	
	UFUNCTION(BlueprintCallable, Category = "Javascript | EdMode")
	static void SetDefaultMode(FJavascriptEditorModeTools& Tools, FName DefaultID);
	UFUNCTION(BlueprintCallable, Category = "Javascript | EdMode")
	static void ActivateDefaultMode(FJavascriptEditorModeTools& Tools);
	UFUNCTION(BlueprintCallable, Category = "Javascript | EdMode")
	static bool IsDefaultModeActive(FJavascriptEditorModeTools& Tools);
	UFUNCTION(BlueprintCallable, Category = "Javascript | EdMode")
	static void ActivateMode(FJavascriptEditorModeTools& Tools, FName InID, bool bToggle);
	UFUNCTION(BlueprintCallable, Category = "Javascript | EdMode")
	static void DeactivateMode(FJavascriptEditorModeTools& Tools, FName InID);
	UFUNCTION(BlueprintCallable, Category = "Javascript | EdMode")
	static void DestroyMode(FJavascriptEditorModeTools& Tools, FName InID);
	UFUNCTION(BlueprintCallable, Category = "Javascript | EdMode")
	static void DeactivateAllModes(FJavascriptEditorModeTools& Tools);
	UFUNCTION(BlueprintCallable, Category = "Javascript | EdMode")
	static bool EnsureNotInMode(FJavascriptEditorModeTools& Tools, FName ModeID, const FText& ErrorMsg, bool bNotifyUser);
	UFUNCTION(BlueprintCallable, Category = "Javascript | EdMode")
	static bool IsModeActive(FJavascriptEditorModeTools& Tools, FName InID);

	UFUNCTION(BlueprintCallable, Category = "Javascript | EdMode")
	static bool StartTracking(FJavascriptEditorModeTools Tools, FJavascriptEdViewport Viewport);
	UFUNCTION(BlueprintCallable, Category = "Javascript | EdMode")
	static bool EndTracking(FJavascriptEditorModeTools Tools, FJavascriptEdViewport Viewport);
	UFUNCTION(BlueprintCallable, Category = "Javascript | EdMode")
	static bool IsTracking(FJavascriptEditorModeTools Tools);


	UFUNCTION(BlueprintCallable, Category = "Javascript | EdMode")
	static FJavascriptHitProxy GetHitProxy(FJavascriptEdViewport Viewport);

	UFUNCTION(BlueprintCallable, Category = "Javascript | EdMode")
	static FJavascriptEditorModeTools GetLevelEditorModeTools();

	UFUNCTION(BlueprintCallable, Category = "Javascript | EdMode")
	static FJavascriptEditorModeTools GetModeManager(FJavascriptEditorMode Mode);

	UFUNCTION(BlueprintCallable, Category = "Javascript | EdMode")
	static int32 GetCurrentWidgetAxis(FJavascriptEditorMode Mode);
	UFUNCTION(BlueprintCallable, Category = "Javascript | EdMode")
	static void SetCurrentWidgetAxis(FJavascriptEditorMode Mode, int32 InAxis);
	UFUNCTION(BlueprintCallable, Category = "Javascript | EdMode")
	static void SelectNone(FJavascriptEditorMode Mode);
#endif
};
