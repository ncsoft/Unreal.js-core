#pragma once

#include "JavascriptSearchBox.generated.h"

class SSearchBox;

UCLASS(Experimental)
class JAVASCRIPTUMG_API UJavascriptSearchBox : public UWidget
{
	GENERATED_UCLASS_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEditableTextChangedEvent, const FText&, Text);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEditableTextCommittedEvent, const FText&, Text, ETextCommit::Type, CommitMethod);

public:		
	/** Called whenever the text is changed interactively by the user */
	UPROPERTY(BlueprintAssignable, Category = "Widget Event", meta = (DisplayName = "OnTextChanged (Search Box)"))
	FOnEditableTextChangedEvent OnTextChanged;

	/** Called whenever the text is committed.  This happens when the user presses enter or the text box loses focus. */
	UPROPERTY(BlueprintAssignable, Category = "Widget Event", meta = (DisplayName = "OnTextCommitted (Search Box)"))
	FOnEditableTextCommittedEvent OnTextCommitted;

	/** The text content for this editable text box widget */
	UPROPERTY(EditAnywhere, Category = Content)
	FText Text;

	/** A bindable delegate to allow logic to drive the text of the widget */
	UPROPERTY()
	FGetText TextDelegate;

	/** Hint text that appears when there is no text in the text box */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Content)
	FText HintText;

	/** A bindable delegate to allow logic to drive the hint text of the widget */
	UPROPERTY()
	FGetText HintTextDelegate;

	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void SynchronizeProperties() override;
	// End of UWidget interface

	UFUNCTION(BlueprintCallable, Category = "Widget", meta = (DisplayName = "SetText (Search Box)"))
	void SetText(FText InText);

	UFUNCTION(BlueprintCallable, Category = "Widget")
	void SetHintText(FText InHintText);

protected:
	TSharedPtr<SSearchBox> MySearchBox;
	PROPERTY_BINDING_IMPLEMENTATION(FText, Text);
	PROPERTY_BINDING_IMPLEMENTATION(FText, HintText);

};
