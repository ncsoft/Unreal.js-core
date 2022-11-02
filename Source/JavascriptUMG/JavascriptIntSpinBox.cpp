// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "JavascriptIntSpinBox.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Font.h"

static FSpinBoxStyle* DefaultSpinBoxStyle = nullptr;

UJavascriptIntSpinBox::UJavascriptIntSpinBox(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (!IsRunningDedicatedServer())
	{
		static ConstructorHelpers::FObjectFinder<UFont> RobotoFontObj(*UWidget::GetDefaultFontName());
		Font = FSlateFontInfo(RobotoFontObj.Object, 12, FName("Bold"));
	}

	Value = 0;
	MinValue = 0;
	MaxValue = 10;
	MinSliderValue = 0;
	MaxSliderValue = 0;
	Delta = 0;
	SliderExponent = 1;
	MinDesiredWidth = 0;
	ClearKeyboardFocusOnCommit = false;
	SelectAllTextOnCommit = true;
	ForegroundColor = FSlateColor(FLinearColor::Black);

	if (DefaultSpinBoxStyle == nullptr)
	{
		// HACK: THIS SHOULD NOT COME FROM CORESTYLE AND SHOULD INSTEAD BE DEFINED BY ENGINE TEXTURES/PROJECT SETTINGS
		DefaultSpinBoxStyle = new FSpinBoxStyle(FCoreStyle::Get().GetWidgetStyle<FSpinBoxStyle>("SpinBox"));

		// Unlink UMG default colors from the editor settings colors.
		DefaultSpinBoxStyle->UnlinkColors();
	}

	WidgetStyle = *DefaultSpinBoxStyle;
}

void UJavascriptIntSpinBox::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MySpinBox.Reset();
}

TSharedRef<SWidget> UJavascriptIntSpinBox::RebuildWidget()
{
	MySpinBox = SNew(SSpinBox<int32>)
		.Style(&WidgetStyle)
		.Font(Font)
		.ClearKeyboardFocusOnCommit(ClearKeyboardFocusOnCommit)
		.SelectAllTextOnCommit(SelectAllTextOnCommit)
		.Justification(Justification)
		.OnValueChanged(BIND_UOBJECT_DELEGATE(FOnInt32ValueChanged, HandleOnValueChanged))
		.OnValueCommitted(BIND_UOBJECT_DELEGATE(FOnInt32ValueCommitted, HandleOnValueCommitted))
		.OnBeginSliderMovement(BIND_UOBJECT_DELEGATE(FSimpleDelegate, HandleOnBeginSliderMovement))
		.OnEndSliderMovement(BIND_UOBJECT_DELEGATE(FOnInt32ValueChanged, HandleOnEndSliderMovement))
		;

	return MySpinBox.ToSharedRef();
}

void UJavascriptIntSpinBox::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	MySpinBox->SetDelta(Delta);
	MySpinBox->SetSliderExponent(SliderExponent);
	MySpinBox->SetMinDesiredWidth(MinDesiredWidth);

	MySpinBox->SetForegroundColor(ForegroundColor);

	// Set optional values
	bOverride_MinValue ? SetMinValue(MinValue) : ClearMinValue();
	bOverride_MaxValue ? SetMaxValue(MaxValue) : ClearMaxValue();
	bOverride_MinSliderValue ? SetMinSliderValue(MinSliderValue) : ClearMinSliderValue();
	bOverride_MaxSliderValue ? SetMaxSliderValue(MaxSliderValue) : ClearMaxSliderValue();

	// Always set the value last so that the max/min values are taken into account.
	TAttribute<int32> ValueBinding = PROPERTY_BINDING(int32, Value);
	MySpinBox->SetValue(ValueBinding);
}

int32 UJavascriptIntSpinBox::GetValue() const
{
	if (MySpinBox.IsValid())
	{
		return MySpinBox->GetValue();
	}

	return Value;
}

void UJavascriptIntSpinBox::SetValue(int32 InValue)
{
	Value = InValue;
	if (MySpinBox.IsValid())
	{
		MySpinBox->SetValue(InValue);
	}
}

// MIN VALUE
int32 UJavascriptIntSpinBox::GetMinValue() const
{
	int32 ReturnVal = TNumericLimits<int32>::Lowest();

	if (MySpinBox.IsValid())
	{
		ReturnVal = MySpinBox->GetMinValue();
	}
	else if (bOverride_MinValue)
	{
		ReturnVal = MinValue;
	}

	return ReturnVal;
}

void UJavascriptIntSpinBox::SetMinValue(int32 InMinValue)
{
	bOverride_MinValue = true;
	MinValue = InMinValue;
	if (MySpinBox.IsValid())
	{
		MySpinBox->SetMinValue(InMinValue);
	}
}

void UJavascriptIntSpinBox::ClearMinValue()
{
	bOverride_MinValue = false;
	if (MySpinBox.IsValid())
	{
		MySpinBox->SetMinValue(TOptional<int32>());
	}
}

// MAX VALUE
int32 UJavascriptIntSpinBox::GetMaxValue() const
{
	int32 ReturnVal = TNumericLimits<int32>::Max();

	if (MySpinBox.IsValid())
	{
		ReturnVal = MySpinBox->GetMaxValue();
	}
	else if (bOverride_MaxValue)
	{
		ReturnVal = MaxValue;
	}

	return ReturnVal;
}

void UJavascriptIntSpinBox::SetMaxValue(int32 InMaxValue)
{
	bOverride_MaxValue = true;
	MaxValue = InMaxValue;
	if (MySpinBox.IsValid())
	{
		MySpinBox->SetMaxValue(InMaxValue);
	}
}
void UJavascriptIntSpinBox::ClearMaxValue()
{
	bOverride_MaxValue = false;
	if (MySpinBox.IsValid())
	{
		MySpinBox->SetMaxValue(TOptional<int32>());
	}
}

// MIN SLIDER VALUE
int32 UJavascriptIntSpinBox::GetMinSliderValue() const
{
	int32 ReturnVal = TNumericLimits<int32>::Min();

	if (MySpinBox.IsValid())
	{
		ReturnVal = MySpinBox->GetMinSliderValue();
	}
	else if (bOverride_MinSliderValue)
	{
		ReturnVal = MinSliderValue;
	}

	return ReturnVal;
}

void UJavascriptIntSpinBox::SetMinSliderValue(int32 InMinSliderValue)
{
	bOverride_MinSliderValue = true;
	MinSliderValue = InMinSliderValue;
	if (MySpinBox.IsValid())
	{
		MySpinBox->SetMinSliderValue(InMinSliderValue);
	}
}

void UJavascriptIntSpinBox::ClearMinSliderValue()
{
	bOverride_MinSliderValue = false;
	if (MySpinBox.IsValid())
	{
		MySpinBox->SetMinSliderValue(TOptional<int32>());
	}
}

// MAX SLIDER VALUE
int32 UJavascriptIntSpinBox::GetMaxSliderValue() const
{
	int32 ReturnVal = TNumericLimits<int32>::Max();

	if (MySpinBox.IsValid())
	{
		ReturnVal = MySpinBox->GetMaxSliderValue();
	}
	else if (bOverride_MaxSliderValue)
	{
		ReturnVal = MaxSliderValue;
	}

	return ReturnVal;
}

void UJavascriptIntSpinBox::SetMaxSliderValue(int32 InMaxSliderValue)
{
	bOverride_MaxSliderValue = true;
	MaxSliderValue = InMaxSliderValue;
	if (MySpinBox.IsValid())
	{
		MySpinBox->SetMaxSliderValue(InMaxSliderValue);
	}
}

void UJavascriptIntSpinBox::ClearMaxSliderValue()
{
	bOverride_MaxSliderValue = false;
	if (MySpinBox.IsValid())
	{
		MySpinBox->SetMaxSliderValue(TOptional<int32>());
	}
}

void UJavascriptIntSpinBox::SetForegroundColor(FSlateColor InForegroundColor)
{
	ForegroundColor = InForegroundColor;
	if (MySpinBox.IsValid())
	{
		MySpinBox->SetForegroundColor(ForegroundColor);
	}
}

// Event handlers
void UJavascriptIntSpinBox::HandleOnValueChanged(int32 InValue)
{
	if (!IsDesignTime())
	{
		OnValueChanged.Broadcast(InValue);
	}
}

void UJavascriptIntSpinBox::HandleOnValueCommitted(int32 InValue, ETextCommit::Type CommitMethod)
{
	if (!IsDesignTime())
	{
		OnValueCommitted.Broadcast(InValue, CommitMethod);
	}
}

void UJavascriptIntSpinBox::HandleOnBeginSliderMovement()
{
	if (!IsDesignTime())
	{
		OnBeginSliderMovement.Broadcast();
	}
}

void UJavascriptIntSpinBox::HandleOnEndSliderMovement(int32 InValue)
{
	if (!IsDesignTime())
	{
		OnEndSliderMovement.Broadcast(InValue);
	}
}

void UJavascriptIntSpinBox::PostLoad()
{
	Super::PostLoad();

	if (GetLinkerUEVersion().FileVersionUE4 < VER_UE4_DEPRECATE_UMG_STYLE_ASSETS)
	{
		if (Style_DEPRECATED != nullptr)
		{
			const FSpinBoxStyle* StylePtr = Style_DEPRECATED->GetStyle<FSpinBoxStyle>();
			if (StylePtr != nullptr)
			{
				WidgetStyle = *StylePtr;
			}

			Style_DEPRECATED = nullptr;
		}
	}
}
