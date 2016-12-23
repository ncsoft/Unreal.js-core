#include "JavascriptUMG.h"
#include "JavascriptMenuContext.h"

bool UJavascriptMenuContext::Public_CanExecute()
{
	return OnCanExecute.IsBound() ? OnCanExecute.Execute() : true;
}

void UJavascriptMenuContext::Public_Execute()
{
	OnExecute.ExecuteIfBound();
}