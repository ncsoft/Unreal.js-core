#pragma once

#include "JavascriptMenuLibrary.generated.h"

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
struct FJavascriptMenuBuilder
{
	GENERATED_BODY()

#if WITH_EDITOR
	TSharedPtr<FMultiBoxBuilder> MultiBox;
	TSharedPtr<FMenuBuilder> Menu;
	TSharedPtr<FMenuBarBuilder> MenuBar;
	TSharedPtr<FToolBarBuilder> ToolBar;
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
class JAVASCRIPTUMG_API UJavascriptMenuLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptMenuBuilder CreateToolbarBuilder(FJavascriptUICommandList CommandList);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptMenuBuilder CreateMenuBuilder(FJavascriptUICommandList CommandList, bool bInShouldCloseWindowAfterMenuSelection);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptMenuBuilder CreateMenuBarBuilder(FJavascriptUICommandList CommandList);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void BeginSection(FJavascriptMenuBuilder& Builder, FName InExtensionHook);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void EndSection(FJavascriptMenuBuilder& Builder);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void AddSeparator(FJavascriptMenuBuilder& Builder);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void AddToolBarButton(FJavascriptMenuBuilder& Builder, FJavascriptUICommandInfo CommandInfo);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void PushCommandList(FJavascriptMenuBuilder& Builder, FJavascriptUICommandList List);
	
	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void PopCommandList(FJavascriptMenuBuilder& Builder);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptBindingContext NewBindingContext(const FName InContextName, const FText& InContextDesc, const FName InContextParent, const FName InStyleSetName);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void Destroy(FJavascriptBindingContext Context);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptUICommandInfo UI_COMMAND_Function(FJavascriptBindingContext This, FJavascriptUICommand Command);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptUICommandList CreateUICommandList();

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool ProcessCommandBindings_KeyEvent(FJavascriptUICommandList CommandList, const FKeyEvent& InKeyEvent);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool ProcessCommandBindings_PointerEvent(FJavascriptUICommandList CommandList, const FPointerEvent& InMouseEvent);
};
