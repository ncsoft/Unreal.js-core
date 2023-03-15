#pragma once
#include "Components/Widget.h"
#include "JavascriptUMGLibrary.h"
#include "JavascriptFTextBox.generated.h"

class FJavascriptEditableText;
class STextPropertyEditableTextBox;

UCLASS()
class JAVASCRIPTUMG_API UJavascriptFTextBox : public UWidget
{
	GENERATED_UCLASS_BODY()

public:
	DECLARE_DYNAMIC_DELEGATE_RetVal(bool, FOnIsReadOnly);
	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(FString, FOnIsValidText, const FString&, TextValue);
	DECLARE_DYNAMIC_DELEGATE_RetVal(FJavascriptTextProperty, FOnGetDefaultValue);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEditableTextBoxCommittedEvent, const FJavascriptTextProperty&, TextProperty);

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnIsReadOnly OnIsReadOnly;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnIsValidText OnIsValidText;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGetDefaultValue OnGetDefaultValue;

	UPROPERTY(BlueprintAssignable, Category = "TextBox|Event")
	FOnEditableTextBoxCommittedEvent OnTextCommitted;

	/** The style */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Appearance, meta = (DisplayName = "Style", ShowOnlyInnerProperties))
	FEditableTextBoxStyle WidgetStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Appearance)
	float WrapTextAt;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Behavior, AdvancedDisplay)
	bool AutoWrapText;

#if WITH_EDITOR
	/** When specified, will report the MinDesiredWidth if larger than the content's desired width */
	FOptionalSize MinimumDesiredWidth;

	/** When specified, will report the MaxDesiredHeight if smaller than the content's desired height */
	FOptionalSize MaximumDesiredHeight;

public:
	UFUNCTION(BlueprintCallable, Category = Content)
	void SetText(FJavascriptTextProperty& JavascriptTextProperty);

	//~ Begin UWidget interface
	virtual void SynchronizeProperties() override;
	// End UWidget interface

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	void HandleOnNamespaceKeyChanged(const FString& InNamespace, const FString& InKey);
	void HandleOnTextCommitted(const FText& InText);
	void HandleOnStringTableKeyChanged(const FName& InTableId, const FString& InKey);

	FText TextValue;
	FText DefaultTextValue;
protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface

protected:
	FJavascriptTextProperty MyTextProperty;
	TSharedPtr<FJavascriptEditableText> MyEditableTextProperty;
	TSharedPtr<STextPropertyEditableTextBox> MyEditableTextBlock;
#endif
};
