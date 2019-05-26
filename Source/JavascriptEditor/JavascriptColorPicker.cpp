#include "JavascriptColorPicker.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Colors/SColorPicker.h"

PRAGMA_DISABLE_SHADOW_VARIABLE_WARNINGS

UJavascriptColorPicker::UJavascriptColorPicker(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{	
	SelectedColor = FLinearColor::Black;
}

TSharedRef<SWidget> UJavascriptColorPicker::RebuildWidget()
{
	MyColorBlock = SNew(SColorBlock)
		.Color(TAttribute<FLinearColor>::Create([this]() {return SelectedColor; }))
		.ShowBackgroundForAlpha(true)
		.OnMouseButtonDown(FPointerEventHandler::CreateLambda([this](const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) {
			if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
			{
				TArray<FLinearColor*> LinearColorArray;
				LinearColorArray.Add(&SelectedColor);

				FColorPickerArgs PickerArgs;
				PickerArgs.bIsModal = true;
				PickerArgs.ParentWidget = MyColorBlock;
				PickerArgs.DisplayGamma = TAttribute<float>::Create(TAttribute<float>::FGetter::CreateUObject(GEngine, &UEngine::GetDisplayGamma));
				PickerArgs.LinearColorArray = &LinearColorArray;
				PickerArgs.OnColorCommitted = FOnLinearColorValueChanged::CreateLambda([this](FLinearColor color) {
					SelectedColor = color;
					OnColorChanged.Broadcast(SelectedColor);
				});
				PickerArgs.bUseAlpha = true;

				OpenColorPicker(PickerArgs);

				return FReply::Handled();
			}
			else
			{
				return FReply::Unhandled();
			}
	}));

	return MyColorBlock.ToSharedRef();
}

PRAGMA_ENABLE_SHADOW_VARIABLE_WARNINGS