#pragma once

#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "IDetailPropertyRow.h"
#include "JavascriptUMGLibrary.h"
#include "JavascriptPropertyCustomizationLibrary.generated.h"

USTRUCT()
struct FJavascriptDetailWidgetDecl
{
	GENERATED_BODY()

#if WITH_EDITOR
	FDetailWidgetDecl* WidgetDecl;

	FDetailWidgetDecl* operator -> ()
	{
		return WidgetDecl;
	}

	FDetailWidgetDecl& Get()
	{
		return *WidgetDecl;
	}
#endif
};

USTRUCT()
struct FJavascriptPropertyHandle
{
	GENERATED_BODY()

#if WITH_EDITOR
	TSharedPtr<IPropertyHandle> PropertyHandle;

	TSharedPtr<IPropertyHandle> operator -> ()
	{
		return PropertyHandle;
	}
#endif
};

USTRUCT()
struct FJavascriptDetailWidgetRow
{
	GENERATED_BODY()

#if WITH_EDITOR	
	FDetailWidgetRow* operator -> ()
	{
		return HeaderRow;
	}

	class FDetailWidgetRow* HeaderRow;
#endif
};

USTRUCT()
struct FJavascriptDetailPropertyRow
{
	GENERATED_BODY()

#if WITH_EDITOR	
	IDetailPropertyRow* operator -> ()
	{
		return PropertyRow;
	}

	IDetailPropertyRow* PropertyRow;
#endif
};

USTRUCT()
struct FJavascriptDetailChildrenBuilder
{
	GENERATED_BODY()

#if WITH_EDITOR	
	IDetailChildrenBuilder* operator -> ()
	{
		return ChildBuilder;
	}

	IDetailChildrenBuilder* ChildBuilder;
#endif
};

USTRUCT()
struct FJavascriptPropertyTypeCustomizationUtils
{
	GENERATED_BODY()

#if WITH_EDITOR	
	IPropertyTypeCustomizationUtils* operator -> ()
	{
		return CustomizationUtils;
	}

	IPropertyTypeCustomizationUtils* CustomizationUtils;
#endif
};

/**
 * 
 */
UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptPropertyCustomizationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
		static FJavascriptPropertyHandle GetChildHandle(FJavascriptPropertyHandle Parent, FName Name);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
		static FJavascriptSlateWidget CreatePropertyNameWidget(FJavascriptPropertyHandle Handle, const FText& NameOverride, const FText& ToolTipOverride, bool bDisplayResetToDefault, bool bHideText, bool bHideThumbnail);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
		static FJavascriptSlateWidget CreatePropertyValueWidget(FJavascriptPropertyHandle Handle, bool bHideDefaultPropertyButtons);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
		static FJavascriptDetailWidgetDecl WholeRowContent(FJavascriptDetailWidgetRow Row);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
		static FJavascriptDetailWidgetDecl NameContent(FJavascriptDetailWidgetRow Row);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
		static FJavascriptDetailWidgetDecl ValueContent(FJavascriptDetailWidgetRow Row);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
		static void SetFilterString(FJavascriptDetailWidgetRow Row, const FText& InFilterString);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
		static FJavascriptDetailWidgetRow AddChildContent(FJavascriptDetailChildrenBuilder ChildBuilder, const FText& SearchString);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
		static FJavascriptDetailPropertyRow AddChildProperty(FJavascriptDetailChildrenBuilder ChildBuilder, FJavascriptPropertyHandle PropertyHandle);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
		static FJavascriptSlateWidget GenerateStructValueWidget(FJavascriptDetailChildrenBuilder ChildBuilder, FJavascriptPropertyHandle StructPropertyHandle);

		
	//void DisplayName(FJavascriptDetailPropertyRow Row, const FText& InDisplayName);
	//void ToolTip(FJavascriptDetailPropertyRow Row, const FText& InToolTip);
	//void ShowPropertyButtons(bool bShowPropertyButtons);
	/*void EditCondition(TAttribute<bool> EditConditionValue, FOnBooleanValueChanged OnEditConditionValueChanged);
	void IsEnabled(TAttribute<bool> InIsEnabled);*/
	//void ShouldAutoExpand(FJavascriptDetailPropertyRow Row, bool bForceExpansion);
	//void Visibility(TAttribute<EVisibility> Visibility);
	//void OverrideResetToDefault(const FResetToDefaultOverride& ResetToDefault);
	//void GetDefaultWidgets(TSharedPtr<SWidget>& OutNameWidget, TSharedPtr<SWidget>& OutValueWidget);
	//void GetDefaultWidgets(TSharedPtr<SWidget>& OutNameWidget, TSharedPtr<SWidget>& OutValueWidget, FDetailWidgetRow& Row);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
		static FJavascriptDetailWidgetRow CustomWidget(FJavascriptDetailPropertyRow Row, bool bShowChildren);


	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
		static void SetContent(FJavascriptDetailWidgetDecl Decl, FJavascriptSlateWidget Widget);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
		static void SetVAlign(FJavascriptDetailWidgetDecl Decl, EVerticalAlignment InAlignment);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
		static void SetHAlign(FJavascriptDetailWidgetDecl Decl, EHorizontalAlignment InAlignment);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
		static void SetMinDesiredWidth(FJavascriptDetailWidgetDecl Decl, float MinWidth);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
		static void SetMaxDesiredWidth(FJavascriptDetailWidgetDecl Decl, float MaxWidth);
#endif
};
