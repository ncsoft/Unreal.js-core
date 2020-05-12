#pragma once

#include "PropertyHandle.h"
#include "DetailWidgetRow.h"
#include "IDetailPropertyRow.h"
#include "JavascriptIsolate.h"
#include "JavascriptUMG/JavascriptUMGLibrary.h"
#include "JavascriptPropertyCustomizationLibrary.generated.h"

class UJavascriptPropertyCustomization;

USTRUCT(BlueprintType)
struct FJavascriptDetailWidgetDecl
{
	GENERATED_BODY()

#if WITH_EDITOR
	FDetailWidgetDecl* WidgetDecl = nullptr;

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

USTRUCT(BlueprintType)
struct FJavascriptPropertyHandle
{
	GENERATED_BODY()

#if WITH_EDITOR
	TSharedPtr<IPropertyHandle> PropertyHandle;

	TSharedPtr<IPropertyHandle> operator -> ()
	{
		return PropertyHandle;
	}

	bool IsValid() const { return PropertyHandle.IsValid(); }
#endif
};

USTRUCT(BlueprintType)
struct FJavascriptDetailWidgetRow
{
	GENERATED_BODY()

#if WITH_EDITOR	
	FDetailWidgetRow* operator -> ()
	{
		return HeaderRow;
	}

	class FDetailWidgetRow* HeaderRow = nullptr;
#endif
};

USTRUCT(BlueprintType)
struct FJavascriptDetailPropertyRow
{
	GENERATED_BODY()

#if WITH_EDITOR	
	IDetailPropertyRow* operator -> ()
	{
		return PropertyRow;
	}

	IDetailPropertyRow* PropertyRow = nullptr;
#endif
};

USTRUCT(BlueprintType)
struct FJavascriptDetailChildrenBuilder
{
	GENERATED_BODY()

#if WITH_EDITOR	
	IDetailChildrenBuilder* operator -> ()
	{
		return ChildBuilder;
	}

	IDetailChildrenBuilder* ChildBuilder = nullptr;
#endif
};

USTRUCT(BlueprintType)
struct FJavascriptPropertyTypeCustomizationUtils
{
	GENERATED_BODY()

#if WITH_EDITOR	
	IPropertyTypeCustomizationUtils* operator -> ()
	{
		return CustomizationUtils;
	}

	IPropertyTypeCustomizationUtils* CustomizationUtils = nullptr;
#endif
};

UENUM()
enum class EPropertyAccessResult : uint8
{
	/** Multiple values were found so the value could not be read */
	MultipleValues,
	/** Failed to set or get the value */
	Fail,
	/** Successfully set the got the value */
	Success,
};


DECLARE_DYNAMIC_DELEGATE_RetVal(bool, FDynamicSimpleGetBoolDelegate);

UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptSimpleGetBoolDelegateWrapper : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FDynamicSimpleGetBoolDelegate Delegate;
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
	static FJavascriptPropertyHandle GetParentHandle(FJavascriptPropertyHandle Handle);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FJavascriptPropertyHandle GetKeyHandle(FJavascriptPropertyHandle Handle);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static bool IsValidHandle(FJavascriptPropertyHandle Handle);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FJavascriptSlateWidget CreatePropertyNameWidget(FJavascriptPropertyHandle Handle, const FText& NameOverride, const FText& ToolTipOverride, bool bDisplayResetToDefault, bool bHideText, bool bHideThumbnail);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FJavascriptSlateWidget CreatePropertyValueWidget(FJavascriptPropertyHandle Handle, bool bHideDefaultPropertyButtons);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FString GetMetaData(FJavascriptPropertyHandle Handle, const FName& Key);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static EPropertyAccessResult GetValueAsFormattedString(FJavascriptPropertyHandle Handle, FString& OutValue);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static EPropertyAccessResult SetValueFromFormattedString(FJavascriptPropertyHandle Handle, const FString& InValue);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static EPropertyAccessResult GetObjectValue(FJavascriptPropertyHandle Handle, UObject*& OutValue);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static EPropertyAccessResult SetObjectValue(FJavascriptPropertyHandle Handle, const UObject* InValue);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static EPropertyAccessResult GetJavascriptRefValue(FJavascriptPropertyHandle Handle, FJavascriptRef& OutValue);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static EPropertyAccessResult SetJavascriptRefValue(FJavascriptPropertyHandle Handle, const FJavascriptRef& InValue);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static TFieldPath<FProperty> GetProperty(FJavascriptPropertyHandle Handle);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void SetOnPropertyValueChanged(FJavascriptPropertyHandle Handle, UJavascriptPropertyCustomization* Custom);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static bool IsEditConst(FJavascriptPropertyHandle Handle);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static bool IsArrayProperty(FJavascriptPropertyHandle Handle);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static bool IsArrayPropertyWithValueType(FJavascriptPropertyHandle Handle);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static int32 GetIndexInArray(FJavascriptPropertyHandle Handle);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static TArray<UObject*> GetOuterObjects(FJavascriptPropertyHandle Handle);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FString GeneratePathToProperty(FJavascriptPropertyHandle Handle);

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
	static FJavascriptDetailPropertyRow AddExternalObjects(FJavascriptDetailChildrenBuilder ChildBuilder, TArray<UObject*>& Objects, FName UniqueIdName = NAME_None);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FJavascriptDetailPropertyRow AddExternalObjectProperty(FJavascriptDetailChildrenBuilder ChildBuilder, TArray<UObject*>& Objects, FName PropertyName, FName UniqueIdName = NAME_None, bool bAllowChildrenOverride = false, bool bCreateCategoryNodesOverride = false);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FJavascriptSlateWidget GenerateStructValueWidget(FJavascriptDetailChildrenBuilder ChildBuilder, FJavascriptPropertyHandle StructPropertyHandle);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void RequestRefresh(FJavascriptPropertyTypeCustomizationUtils CustomizationUtils, bool bForce);

#pragma region FJavascriptDetailPropertyRow
	//void DisplayName(FJavascriptDetailPropertyRow Row, const FText& InDisplayName);
	//void ToolTip(FJavascriptDetailPropertyRow Row, const FText& InToolTip);
	//void ShowPropertyButtons(bool bShowPropertyButtons);
	/*void EditCondition(TAttribute<bool> EditConditionValue, FOnBooleanValueChanged OnEditConditionValueChanged);
	void IsEnabled(TAttribute<bool> InIsEnabled);*/
	//void ShouldAutoExpand(FJavascriptDetailPropertyRow Row, bool bForceExpansion);
	//void OverrideResetToDefault(const FResetToDefaultOverride& ResetToDefault);
	//void GetDefaultWidgets(TSharedPtr<SWidget>& OutNameWidget, TSharedPtr<SWidget>& OutValueWidget);
	//void GetDefaultWidgets(TSharedPtr<SWidget>& OutNameWidget, TSharedPtr<SWidget>& OutValueWidget, FDetailWidgetRow& Row);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FJavascriptDetailWidgetRow CustomWidget(FJavascriptDetailPropertyRow Row, bool bShowChildren);
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void BindVisibility(FJavascriptDetailPropertyRow Row, UJavascriptSimpleGetBoolDelegateWrapper* Wrapper);
#pragma endregion FJavascriptDetailPropertyRow

#pragma region FJavascriptDetailWidgetDecl
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
#pragma endregion FJavascriptDetailWidgetDecl

#endif
};
