#include "JavascriptMenuContext.h"

bool UJavascriptMenuContext::Public_CanExecute()
{
	return OnCanExecute.IsBound() ? OnCanExecute.Execute() : true;
}

void UJavascriptMenuContext::Public_Execute()
{
	OnExecute.ExecuteIfBound();
}

ECheckBoxState UJavascriptMenuContext::Public_GetActionCheckState()
{
	return OnGetActionCheckState.IsBound() ? OnGetActionCheckState.Execute() : ECheckBoxState::Unchecked;
}