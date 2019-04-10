// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/Widget.h"
#include "Types/SlateStructs.h"
#include "JavascriptGraphEditorLibrary.h"
#include "JavascriptGraphTextPropertyEditableTextBox.generated.h"

class FJavascriptEditableTextGraphPin;
class STextPropertyEditableTextBox;

/**
*
*/
UCLASS()
class JAVASCRIPTGRAPHEDITOR_API UJavascriptGraphTextPropertyEditableTextBox : public UWidget
{
	GENERATED_UCLASS_BODY()
	
public:
	DECLARE_DYNAMIC_DELEGATE_RetVal(FJavascriptEdGraphPin, FOnGetGraphPin);
	DECLARE_DYNAMIC_DELEGATE_RetVal(FJavascriptTextProperty, FOnGetDefaultValue);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEditableTextBoxCommittedEvent, const FJavascriptTextProperty&, TextProperty);

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGetGraphPin OnGetGraphPin;

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

	/** When specified, will report the MinDesiredWidth if larger than the content's desired width */
	FOptionalSize MinimumDesiredWidth;

	/** When specified, will report the MaxDesiredHeight if smaller than the content's desired height */
	FOptionalSize MaximumDesiredHeight;

public:
	//~ Begin UWidget interface
	virtual void SynchronizeProperties() override;
	// End UWidget interface

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	void HandleOnNamespaceKeyChanged(const FString& InNamespace, const FString& InKey);
	void HandleOnTextCommitted(const FText& InText);
	void HandleOnStringTableKeyChanged(const FName& InTableId, const FString& InKey);

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface

protected:
	FJavascriptTextProperty MyTextProperty;
	TSharedPtr<FJavascriptEditableTextGraphPin> MyEditableTextProperty;
	TSharedPtr<STextPropertyEditableTextBox> MyEditableTextBlock;
};