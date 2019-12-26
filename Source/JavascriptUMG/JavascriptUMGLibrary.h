#pragma once
#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"
#include "Widgets/SWidget.h"
#include "UObject/ScriptMacros.h"
#include "Textures/SlateIcon.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "JavascriptUMGLibrary.generated.h"

USTRUCT(BlueprintType)
struct FJavascriptSlateStyle
{
	GENERATED_BODY()

	TSharedPtr<FSlateStyleSet> Handle;
};

USTRUCT(BlueprintType)
struct FJavascriptSlateWidget
{
	GENERATED_BODY()

	TSharedPtr<SWidget> Widget;
};

USTRUCT()
struct FJavascriptSlateIcon
{
	GENERATED_BODY()

	UPROPERTY()
	FName StyleSetName;

	UPROPERTY()
	FName StyleName;

	UPROPERTY()
	FName SmallStyleName;

	FSlateIcon GetSlateIcon() const
	{
		if (StyleSetName.IsNone() || StyleName.IsNone()) return FSlateIcon();
		if (SmallStyleName.IsNone()) return FSlateIcon(StyleSetName, StyleName);
		return FSlateIcon(StyleSetName, StyleName, SmallStyleName);
	}

	operator FSlateIcon () const
	{
		return GetSlateIcon();
	}
};
/**
 * 
 */
UCLASS()
class JAVASCRIPTUMG_API UJavascriptUMGLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptSlateStyle CreateSlateStyle(FName InStyleSetName);
		
	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void Register(FJavascriptSlateStyle StyleSet);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void Unregister(FJavascriptSlateStyle StyleSet);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void SetContentRoot(FJavascriptSlateStyle StyleSet, const FString& InContentRootDir);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void SetCoreContentRoot(FJavascriptSlateStyle StyleSet, const FString& InCoreContentRootDir);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FString RootToContentDir(FJavascriptSlateStyle StyleSet, const FString& RelativePath);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FString RootToCoreContentDir(FJavascriptSlateStyle StyleSet, const FString& RelativePath);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void AddImageBrush(FJavascriptSlateStyle StyleSet, FName PropertyName, const FString& InImageName, const FVector2D& InImageSize, const FLinearColor& InTint, ESlateBrushTileType::Type InTiling, ESlateBrushImageType::Type InImageType);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void AddBorderBrush(FJavascriptSlateStyle StyleSet, FName PropertyName, const FString& InImageName, const FMargin& InMargin, const FLinearColor& InColorAndOpacity, ESlateBrushImageType::Type InImageType);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void AddBoxBrush(FJavascriptSlateStyle StyleSet, FName PropertyName, const FString& InImageName, const FMargin& InMargin, const FLinearColor& InColorAndOpacity, ESlateBrushImageType::Type InImageType);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void AddSound(FJavascriptSlateStyle StyleSet, FName PropertyName, const FSlateSound& Sound);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void AddFontInfo(FJavascriptSlateStyle StyleSet, FName PropertyName, const FSlateFontInfo& FontInfo);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptSlateWidget TakeWidget(class UWidget* Widget);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static class UWidget* SetContent(class UNativeWidgetHost* TargetWidget, FJavascriptSlateWidget SlateWidget);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FJavascriptSlateWidget GetRootWindow();

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void AddWindowAsNativeChild(FJavascriptSlateWidget NewWindow, FJavascriptSlateWidget RootWindow);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void AddWindow(FJavascriptSlateWidget NewWindow, const bool bShowImmediately = true);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void ShowWindow(FJavascriptSlateWidget NewWindow);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FVector2D GenerateDynamicImageResource(const FName InDynamicBrushName);
};
