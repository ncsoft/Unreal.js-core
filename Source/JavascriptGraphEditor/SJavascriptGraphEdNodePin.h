#pragma once
#include "SGraphPin.h"

class UEdGraphPin;

class SJavascriptGraphPin : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SJavascriptGraphPin)
		: _PinLabelStyle(NAME_DefaultPinLabelStyle)
		, _UsePinColorForText(false)
		, _SideToSideMargin(5.0f)
	{}
		SLATE_ARGUMENT(FName, PinLabelStyle)
		SLATE_ARGUMENT(bool, UsePinColorForText)
		SLATE_ARGUMENT(float, SideToSideMargin)
	SLATE_END_ARGS()
	void Construct(const FArguments& InArgs, UEdGraphPin* InPin);
protected:
	/** SGraphPin */
	virtual EVisibility GetDefaultValueVisibility() const;

	virtual const FSlateBrush* GetPinIcon() const override;
	virtual FSlateColor GetPinColor() const override;
	virtual FSlateColor GetPinTextColor() const override;
	// SWidget interface
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	// End of SWidget interface

	const FSlateBrush* GetPinBorder() const;

	EVisibility GetPinVisiblity() const;
	EVisibility GetPinLabelVisibility() const;
};
