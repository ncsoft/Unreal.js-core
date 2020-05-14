#include "JavascriptComboButtonContext.h"
#include "Components/Widget.h"

TSharedRef<SWidget> UJavascriptComboButtonContext::Public_OnGetWidget(UObject* EditingObject)
{
	if (OnGetWidget.IsBound())
	{
		auto Widget = OnGetWidget.Execute(EditingObject);
		if (Widget.Widget.IsValid())
		{
			return Widget.Widget.ToSharedRef();
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
