#pragma once

#include "Components/TextBlock.h"
#include "JavascriptTextBlock.generated.h"

class SSearchBox;

UCLASS(Experimental)
class JAVASCRIPTUMG_API UJavascriptTextBlock : public UTextBlock
{
	GENERATED_UCLASS_BODY()

public:		
	/** Highlight text that appears when there is no text in the text box */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Content)
	FText HighlightText;

	/** A bindable delegate to allow logic to drive the Highlight text of the widget */
	UPROPERTY()
	FGetText HighlightTextDelegate;

	// UWidget interface
	virtual void SynchronizeProperties() override;
	// End of UWidget interface

	UFUNCTION(BlueprintCallable, Category = "Widget")
	void SetHighlightText(FText InHighlightText);

	PROPERTY_BINDING_IMPLEMENTATION(FText, HighlightText);
};
