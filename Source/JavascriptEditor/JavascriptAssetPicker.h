#pragma once

#include "Components/Widget.h"
#include "JavascriptAssetPicker.generated.h"

class SMenuAnchor;

UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptAssetPicker : public UWidget
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = Content)
	void SetCategoryObject(UObject* InCategoryObject) { CategoryObject = InCategoryObject; }

	UFUNCTION(BlueprintCallable, Category = Content)
	void SetAllowedClasses(const FString& InAllowedClasses) { AllowedClasses = InAllowedClasses; }

private:

	FReply OnClickUse();

	FText GetValue() const;

	FText GetObjectToolTip() const;

	FReply OnClickBrowse();

	FText OnGetComboTextValue() const;

	TSharedRef<SWidget> GenerateAssetPicker();

public:

	DECLARE_DYNAMIC_DELEGATE_RetVal(UObject*, FOnGetDefaultValue);
	
	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSetDefaultValue, FText, Value);
	
	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGetDefaultValue OnGetDefaultValue;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnSetDefaultValue OnSetDefaultValue;

	UPROPERTY()
	UObject* CategoryObject;

	UPROPERTY()
	FString AllowedClasses;

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface
	
	TSharedPtr< class SMenuAnchor > AssetPickerAnchor;

	class UObject* DefaultObject;
};