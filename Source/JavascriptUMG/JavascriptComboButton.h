#pragma once

#include "JavascriptUMGLibrary.h"
#include "ContentWidget.h"
#include "SlateTypes.h"
#include "JavascriptComboButton.generated.h"

class SComboButton;

UCLASS(Experimental)
class JAVASCRIPTUMG_API UJavascriptComboButton : public UContentWidget
{
	GENERATED_UCLASS_BODY()

public:
	DECLARE_DYNAMIC_DELEGATE_RetVal(FJavascriptSlateWidget, FOnGetContent);
	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnIsOpenChanged, bool, Value);
	DECLARE_DYNAMIC_DELEGATE(FOnComboBoxOpened);

	UPROPERTY()
	FComboButtonStyle ComboButtonStyle;

	UPROPERTY()
	FButtonStyle ButtonStyle;
	
	UPROPERTY(meta = (IsBindableEvent = "True"))
	FOnGetContent OnGetMenuContent;

	UPROPERTY(meta = (IsBindableEvent = "True"))
	FOnIsOpenChanged OnMenuOpenChanged;

	UPROPERTY(meta = (IsBindableEvent = "True"))
	FOnComboBoxOpened OnComboBoxOpened;
	
	UPROPERTY()
	bool bIsFocusable;

	UPROPERTY()
	bool bHasDownArrow;

	UPROPERTY()
	FSlateColor ForegroundColor;

	UPROPERTY()
	FSlateColor ButtonColorAndOpacity;

	UPROPERTY()
	FMargin ContentPadding;

	UPROPERTY()
	TEnumAsByte<EMenuPlacement> MenuPlacement;

	UPROPERTY()
	TEnumAsByte<EHorizontalAlignment> HAlign;

	UPROPERTY()
	TEnumAsByte<EVerticalAlignment> VAlign;

	/** Closes the menu if it is currently open. */
	UFUNCTION(BlueprintCallable, Category = "Menu Anchor")
	void SetIsOpen(bool InIsOpen, bool bFocusMenu);

	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void SynchronizeProperties() override;
	// End of UWidget interface

	void HandleComboBoxOpened();
	void HandleMenuOpenChanged(bool bOpen);

protected:
	TSharedPtr<SComboButton> MyComboButton;
};
