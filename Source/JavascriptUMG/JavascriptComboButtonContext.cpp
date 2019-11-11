#include "JavascriptComboButtonContext.h"
#include "Components/Widget.h"

TSharedRef<SWidget> UJavascriptComboButtonContext::Public_OnGetWidget()
{
	if (OnGetWidget.IsBound())
	{
		UWidget* Widget = OnGetWidget.Execute();
		if (Widget)
		{
			return Widget->TakeWidget();
		}
	}
	return SNullWidget::NullWidget;
}

FSlateIcon UJavascriptComboButtonContext::Public_OnGetSlateIcon()
{
	return OnGetIcon.IsBound() ? OnGetIcon.Execute() : FSlateIcon();
}

bool UJavascriptComboButtonContext::Public_CanExecute()
{
	return OnCanExecute.IsBound() ? OnCanExecute.Execute() : true;
}

FText UJavascriptComboButtonContext::Public_OnGetTooltip()
{
	return OnGetTooltip.IsBound() ? OnGetTooltip.Execute() : FText();
}

FText UJavascriptComboButtonContext::Public_OnGetLabel()
{
	return OnGetLabel.IsBound() ? OnGetLabel.Execute() : FText();
}
