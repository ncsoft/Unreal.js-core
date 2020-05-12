#pragma once

#include "JavascriptUMG/JavascriptUMGLibrary.h"
#include "JavascriptIsolate.h"
#include "../JavascriptUMG/JavascriptComboButtonContext.h"
#include "JavascriptUMG/JavascriptMenuContext.h"
#include "Framework/Commands/UICommandInfo.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Framework/Commands/InputBindingManager.h"
#include "UObject/TextProperty.h"
#include "JavascriptMenuLibrary.generated.h"

class UJavascriptUICommands;
class UJavascriptToolbarButtonContext;
class UToolMenu;

USTRUCT(BlueprintType)
struct FJavascriptUICommandInfo
{
	GENERATED_BODY()

public:
	TSharedPtr<FUICommandInfo> Handle;
};

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

	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	FJavascriptUICommandInfo CommandInfo;

	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	FString IconStyleName;
};

USTRUCT(BlueprintType)
struct FJavascriptMenuBuilder
{
	GENERATED_BODY()

	UToolMenu* ToolMenu = nullptr;
	FMultiBoxBuilder* MultiBox = nullptr;
	FMenuBuilder* Menu = nullptr;
	FMenuBarBuilder* MenuBar = nullptr;
	FToolBarBuilder* ToolBar = nullptr;
};

USTRUCT(BlueprintType)
struct FJavascriptUICommandList
{
	GENERATED_BODY()

	TSharedPtr<FUICommandList> Handle;

	operator TSharedPtr<FUICommandList>()
	{
		return Handle;
	}
};

USTRUCT(BlueprintType)
struct FJavascriptBindingContext
{
	GENERATED_BODY()

public:
	void Destroy()
	{
		if (Handle.IsValid())
		{
			FInputBindingManager::Get().RemoveContextByName(Handle->GetContextName());
			Handle.Reset();
		}
	}

	TSharedPtr<FBindingContext> Handle;
};


USTRUCT(BlueprintType)
struct JAVASCRIPTEDITOR_API FJavascriptExtender
{
	GENERATED_BODY()

public:
	FJavascriptExtender();
	FJavascriptExtender(TSharedPtr<FExtender> Extender);

	FExtender* operator -> () const
	{
		return Handle.Get();
	}

	TSharedPtr<FExtender> Handle;
};

USTRUCT(BlueprintType)
struct FJavascriptExtensionBase
{
	GENERATED_BODY()

public:
	TSharedPtr<const FExtensionBase> Handle;
};

UENUM()
namespace EJavascriptExtensionHook
{
	enum Type
	{
		/** Inserts the extension before the element or section. */
		Before,
		/** Inserts the extension after the element or section. */
		After,
		/** Sections only. Inserts the extension at the beginning of the section. */
		First,
	};
}

/**
 *
 */
UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptMenuLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void CreateToolbarBuilder(FJavascriptUICommandList CommandList, EOrientation Orientation, FJavascriptFunction Function);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void CreateMenuBuilder(FJavascriptUICommandList CommandList, bool bInShouldCloseWindowAfterMenuSelection, FJavascriptFunction Function);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void CreateMenuBarBuilder(FJavascriptUICommandList CommandList, FJavascriptFunction Function);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptExtensionBase AddToolBarExtension(FJavascriptExtender Extender, FName ExtensionHook, EJavascriptExtensionHook::Type HookPosition, FJavascriptUICommandList CommandList, FJavascriptFunction Function);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptExtensionBase AddMenuExtension(FJavascriptExtender Extender, FName ExtensionHook, EJavascriptExtensionHook::Type HookPosition, FJavascriptUICommandList CommandList, FJavascriptFunction Function);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptExtensionBase AddMenubarExtension(FJavascriptExtender Extender, FName ExtensionHook, EJavascriptExtensionHook::Type HookPosition, FJavascriptUICommandList CommandList, FJavascriptFunction Function);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void RemoveExtension(FJavascriptExtender Extender, FJavascriptExtensionBase Extension);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void Apply(FJavascriptExtender Extender, FName ExtensionHook, EJavascriptExtensionHook::Type HookPosition, FJavascriptMenuBuilder& MenuBuilder);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptExtender Combine(const TArray<FJavascriptExtender>& Extenders);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void BeginSection(FJavascriptMenuBuilder& Builder, FName InExtensionHook, FText MenuHeadingText);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void EndSection(FJavascriptMenuBuilder& Builder);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void AddSeparator(FJavascriptMenuBuilder& Builder);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void AddToolBarButton(FJavascriptMenuBuilder& Builder, FJavascriptUICommandInfo CommandInfo);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void AddToolBarButtonByContext(FJavascriptMenuBuilder& Builder, UJavascriptToolbarButtonContext* Context, UObject* EditingObject);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void AddComboButton(FJavascriptMenuBuilder& Builder, UJavascriptComboButtonContext* Object, UObject* EditingObject);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void AddMenuEntry(FJavascriptMenuBuilder& Builder, UJavascriptMenuContext* Object);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void AddSubMenu(FJavascriptMenuBuilder& Builder, const FText& Label, const FText& ToolTip, const bool bInOpenSubMenuOnClick, FJavascriptFunction Function);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void AddMenuByCommands(FJavascriptMenuBuilder& Builder, UJavascriptUICommands* UICommands);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void AddWidget(FJavascriptMenuBuilder& Builder, UWidget* Widget, const FText& Label, bool bNoIndent, FName InTutorialHighlightName, bool bSearchable);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void PushCommandList(FJavascriptMenuBuilder& Builder, FJavascriptUICommandList List);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void PopCommandList(FJavascriptMenuBuilder& Builder);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void AddPullDownMenu(FJavascriptMenuBuilder& MenuBuilder, const FText& InMenuLabel, const FText& InToolTip, FJavascriptFunction InPullDownMenu, FName InExtensionHook, FName InTutorialHighlightName);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptBindingContext NewBindingContext(const FName InContextName, const FText& InContextDesc, const FName InContextParent, const FName InStyleSetName);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void Destroy(FJavascriptBindingContext Context);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptUICommandInfo UI_COMMAND_Function(FJavascriptBindingContext This, FJavascriptUICommand Command, const FString& InTextSubNamespace);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptUICommandList CreateUICommandList();

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool ProcessCommandBindings_KeyEvent(FJavascriptUICommandList CommandList, const FKeyEvent& InKeyEvent);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool ProcessCommandBindings_PointerEvent(FJavascriptUICommandList CommandList, const FPointerEvent& InMouseEvent);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptUICommandInfo GenericCommand(FString What);
};
