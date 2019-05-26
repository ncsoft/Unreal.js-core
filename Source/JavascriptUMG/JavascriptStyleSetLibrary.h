#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "JavascriptStyleSetLibrary.generated.h"

USTRUCT(BlueprintType)
struct FJavascriptStyleSet
{
	GENERATED_BODY()

	UPROPERTY()
	FName StyleSetName;
};
/**
 * 
 */
UCLASS()
class JAVASCRIPTUMG_API UJavascriptStyleSetLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:		

	UFUNCTION(BlueprintCallable, Category = "Javascript")
	static FButtonStyle GetButtonStyle(const FJavascriptStyleSet& Handle, const FName& StyleName);
	UFUNCTION(BlueprintCallable, Category = "Javascript")
	static FTextBlockStyle GetTextBlockStyle(const FJavascriptStyleSet& Handle, const FName& StyleName);
	UFUNCTION(BlueprintCallable, Category = "Javascript")
	static FEditableTextStyle GetEditableTextStyle(const FJavascriptStyleSet& Handle, const FName& StyleName);
	UFUNCTION(BlueprintCallable, Category = "Javascript")
	static FEditableTextBoxStyle GetEditableTextBoxStyle(const FJavascriptStyleSet& Handle, const FName& StyleName);
	UFUNCTION(BlueprintCallable, Category = "Javascript")
	static FCheckBoxStyle GetCheckBoxStyle(const FJavascriptStyleSet& Handle, const FName& StyleName);
	UFUNCTION(BlueprintCallable, Category = "Javascript")
	static FComboBoxStyle GetComboBoxStyle(const FJavascriptStyleSet& Handle, const FName& StyleName);
	UFUNCTION(BlueprintCallable, Category = "Javascript")
	static FComboButtonStyle GetComboButtonStyle(const FJavascriptStyleSet& Handle, const FName& StyleName);
	UFUNCTION(BlueprintCallable, Category = "Javascript")
	static FProgressBarStyle GetProgressBarStyle(const FJavascriptStyleSet& Handle, const FName& StyleName);
	UFUNCTION(BlueprintCallable, Category = "Javascript")
	static FMargin GetMargin(const FJavascriptStyleSet& Handle, const FName& StyleName);
	UFUNCTION(BlueprintCallable, Category = "Javascript")
	static FSlateColor GetSlateColor(const FJavascriptStyleSet& Handle, const FName& StyleName);
	UFUNCTION(BlueprintCallable, Category = "Javascript")
	static FSlateBrush GetBrush(const FJavascriptStyleSet& Handle, const FName& StyleName);
	UFUNCTION(BlueprintCallable, Category = "Javascript")
	static FLinearColor GetColor(const FJavascriptStyleSet& Handle, const FName& StyleName);
	UFUNCTION(BlueprintCallable, Category = "Javascript")
	static FVector2D GetVector(const FJavascriptStyleSet& Handle, const FName& StyleName);
	UFUNCTION(BlueprintCallable, Category = "Javascript")
	static float GetFloat(const FJavascriptStyleSet& Handle, const FName& StyleName);
	UFUNCTION(BlueprintCallable, Category = "Javascript")
	static FSlateFontInfo GetFontStyle(const FJavascriptStyleSet& Handle, const FName& StyleName);
	UFUNCTION(BlueprintCallable, Category = "Javascript")
	static FSlateSound GetSound(const FJavascriptStyleSet& Handle, const FName& StyleName);

	UPROPERTY()
	FSlateColor SlateColor;	

	UPROPERTY()
	FSlateBrush SlateBrush;

	UPROPERTY()
	FButtonStyle ButtonStyle;
	
	UPROPERTY()
	FTextBlockStyle TextBlockStyle;

	UPROPERTY()
	FEditableTextStyle EditableTextStyle;

	UPROPERTY()
	FEditableTextBoxStyle EditableTextBoxStyle;

	UPROPERTY()
	FCheckBoxStyle CheckBoxStyle;

	UPROPERTY()
	FComboBoxStyle ComboBoxStyle;

	UPROPERTY()	
	FComboButtonStyle ComboButtonStyle;

	UPROPERTY()
	FProgressBarStyle ProgressBarStyle;
};
