#pragma once

#include "JavascriptUMGLibrary.h"
#include "Components/ContentWidget.h"
#include "Styling/SlateTypes.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "JavascriptComboButton.generated.h"

class SComboButton;
class SBorder;

UCLASS(Experimental)
class JAVASCRIPTUMG_API UJavascriptComboButton : public UContentWidget
{
	GENERATED_UCLASS_BODY()

public:
	DECLARE_DYNAMIC_DELEGATE_RetVal(FJavascriptSlateWidget, FOnGetContent);
	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnIsOpenChanged, bool, Value);
	DECLARE_DYNAMIC_DELEGATE(FOnComboBoxOpened);

	UPROPERTY()
	FComboButtonStyle ComboButtonStyle = FCoreStyle::Get().GetWidgetStyle<FComboButtonStyle>("ComboButton");

	UPROPERTY()
	FButtonStyle ButtonStyle = FCoreStyle::Get().GetWidgetStyle<FComboButtonStyle>("ComboButton").ButtonStyle;
	
	UPROPERTY(meta = (IsBindableEvent = "True"))
	FOnGetContent OnGetMenuContent;

	UPROPERTY(meta = (IsBindableEvent = "True"))
	FOnIsOpenChanged OnMenuOpenChanged;

	UPROPERTY(meta = (IsBindableEvent = "True"))
	FOnComboBoxOpened OnComboBoxOpened;
	
	UPROPERTY()
	bool bIsFocusable = true;

	UPROPERTY()
	bool bHasDownArrow = true;

	UPROPERTY()
	FSlateColor ForegroundColor = FCoreStyle::Get().GetSlateColor("InvertedForeground");

	UPROPERTY()
	FSlateColor ButtonColorAndOpacity = FLinearColor::White;

	UPROPERTY()
	FMargin ContentPadding = FMargin(5);

	UPROPERTY()
	TEnumAsByte<EMenuPlacement> MenuPlacement = MenuPlacement_ComboBox;

	UPROPERTY()
	TEnumAsByte<EHorizontalAlignment> HAlign = HAlign_Fill;

	UPROPERTY()
	TEnumAsByte<EVerticalAlignment> VAlign = VAlign_Center;

	/** Closes the menu if it is currently open. */
	UFUNCTION(BlueprintCallable, Category = "Menu Anchor")
	void SetIsOpen(bool InIsOpen, bool bFocusMenu);

	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void SynchronizeProperties() override;
	// End of UWidget interface
	//~ Begin UVisual Interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ End UVisual Interface

	void HandleComboBoxOpened();
	void HandleMenuOpenChanged(bool bOpen);

protected:
	TSharedRef<SWidget> GetComboBox();

	// UPanelWidget
	virtual void OnSlotAdded(UPanelSlot* Slot) override;
	// End UPanelWidget

	TSharedPtr<SBorder> MyContentContainer;
	TSharedPtr<SComboButton> MyComboButton;
};
