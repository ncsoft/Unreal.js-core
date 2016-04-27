#pragma once

#include "JavascriptEdModeLibrary.generated.h"

USTRUCT()
struct FJavascriptEditorModeTools
{
	GENERATED_BODY()

	FJavascriptEditorModeTools()
	{}

	FJavascriptEditorModeTools(FEditorModeTools* InTools)
		: Tools(InTools)
	{}

	FEditorModeTools* Tools;
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
#endif
};
