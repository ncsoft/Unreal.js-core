#pragma once
#include "Components/Widget.h"
#include "JavascriptClassViewer.generated.h"

class SMenuAnchor;

UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptClassViewer : public UWidget
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = Content)
	void SetCategoryClass(UClass* InCategoryClass) { CategoryClass = InCategoryClass; }

	UFUNCTION(BlueprintCallable, Category = Content)
	void SetAllowedChildrenOfClasses(TArray<UClass*> InAllowedChildrenOfClasses) { AllowedChildrenOfClasses = InAllowedChildrenOfClasses; }

	UFUNCTION(BlueprintCallable, Category = Content)
	void SetDefaultClass(UClass* InDefaultClass) { DefaultClass = InDefaultClass; }

private:

	FReply OnClickUse();

	FText GetValue() const;

	FText GetObjectToolTip() const;

	FReply OnClickBrowse();

	FText OnGetComboTextValue() const;

	TSharedRef<SWidget> GeneratePathPicker();

public:

	DECLARE_DYNAMIC_DELEGATE_RetVal(UClass*, FOnGetDefaultValue);

	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSetDefaultValue, FText, Value);

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGetDefaultValue OnGetDefaultValue;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnSetDefaultValue OnSetDefaultValue;

	UPROPERTY()
	UClass* CategoryClass;

	UPROPERTY()
	TArray<UClass*> AllowedChildrenOfClasses;

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface

	TSharedPtr< class SMenuAnchor > ClassViewerAnchor;

	class UClass* DefaultClass;
};
