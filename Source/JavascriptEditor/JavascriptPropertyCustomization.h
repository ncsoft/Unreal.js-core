#pragma once

#include "JavascriptPropertyCustomizationLibrary.h"
#include "JavascriptPropertyCustomization.generated.h"

/**
 * 
 */
UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptPropertyCustomization : public UObject
{
	GENERATED_BODY()

public:	
#if WITH_EDITOR
	DECLARE_DYNAMIC_DELEGATE_FourParams(FCustomHeader, FJavascriptPropertyHandle, Handle, FJavascriptDetailWidgetRow, HeaderRow, FJavascriptPropertyTypeCustomizationUtils, Utils, int32, Id);
	DECLARE_DYNAMIC_DELEGATE_FourParams(FCustomChildren, FJavascriptPropertyHandle, Handle, FJavascriptDetailChildrenBuilder, ChildBuilder, FJavascriptPropertyTypeCustomizationUtils, Utils, int32, Id);
	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnDestroy, int32, Id);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPropertyValueChanged);

	UPROPERTY()
	FName PropertyTypeName;

	UPROPERTY()
	FOnDestroy OnDestroy;

	UPROPERTY()
	FCustomHeader OnCustomizeHeader;	

	UPROPERTY()
	FCustomChildren OnCustomizeChildren;

	UPROPERTY()
	FOnPropertyValueChanged OnPropertyValueChanged;

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	void Register();

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	void Unregister();	
#endif
};
