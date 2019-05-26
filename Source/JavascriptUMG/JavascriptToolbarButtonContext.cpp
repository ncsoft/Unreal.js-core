#include "JavascriptToolbarButtonContext.h"

FSlateIcon UJavascriptToolbarButtonContext::Public_OnGetSlateIcon()
{
	return OnGetIcon.IsBound() ? OnGetIcon.Execute() : FSlateIcon();
}

FText UJavascriptToolbarButtonContext::Public_OnGetTooltip()
{
	return OnGetTooltip.IsBound() ? OnGetTooltip.Execute() : FText();
}

FText UJavascriptToolbarButtonContext::Public_OnGetLabel()
{
	return OnGetLabel.IsBound() ? OnGetLabel.Execute() : FText();
}

void UJavascriptToolbarButtonContext::Public_OnExecuteAction(UObject* EditingObject)
{
	OnExecuteAction.ExecuteIfBound(EditingObject);
}

bool UJavascriptToolbarButtonContext::Public_OnCanExecuteAction(UObject* EditingObject)
{
	return OnCanExecuteAction.IsBound() ? OnCanExecuteAction.Execute(EditingObject) : true;
}

bool UJavascriptToolbarButtonContext::Public_OnIsActionChecked(UObject* EditingObject)
{
	return OnIsActionChecked.IsBound() ? OnIsActionChecked.Execute(EditingObject) : true;
}

bool UJavascriptToolbarButtonContext::Public_OnIsActionButtonVisible(UObject* EditingObject)
{
	return OnIsActionButtonVisible.IsBound() ? OnIsActionButtonVisible.Execute(EditingObject) : true;
}

void UJavascriptToolbarButtonContext::MarkReferencedObject()
{
	AddToRoot();
}

void UJavascriptToolbarButtonContext::UnmarkReferencedObject()
{
	RemoveFromRoot();
}
