#pragma once

#include "JavascriptMenuLibrary.h"
#include "JavascriptUIExtender.generated.h"

USTRUCT(BlueprintType)
struct FJavascriptMenuExtension
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	FName ExtensionHook;

	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	TEnumAsByte<EJavascriptExtensionHook::Type> HookPosition;
};

class UJavascriptUICommands;


/**
*
*/
UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptUIExtender : public UObject
{
	GENERATED_UCLASS_BODY()

public:
#if WITH_EDITOR
	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	TArray<FJavascriptMenuExtension> MenuExtensions;

	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	TArray<FJavascriptMenuExtension> ToolbarExtensions;

	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnHook,FName,Hook);

	UPROPERTY()
	FOnHook OnHook;

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void BeginSection(FName Name, FText Text);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void EndSection();

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FJavascriptUICommandList GetTopCommandList();

	// Menu
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void AddMenuEntry(UJavascriptUICommands* Commands, FString Id);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void AddMenuSeparator();

	// Toolbar
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void AddToolBarButton(UJavascriptUICommands* Commands, FString Id);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void Bind(UJavascriptUICommands* Commands);

	TSharedPtr<FExtender> TakeMenuExtender( const TSharedPtr<FUICommandList>& CommandList );
	TSharedPtr<FExtender> TakeToolbarExtender(const TSharedPtr<FUICommandList>& CommandList);

	static void PushMenuBuilder(FMenuBuilder& MenuBuilder);
	static void Reset();

	void BuildMenu(FMenuBuilder& MenuBuilder);
#endif
};
