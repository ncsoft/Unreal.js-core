#include "JavascriptEditor.h"
#include "JavascriptEdModeLibrary.h"

#if WITH_EDITOR

void UJavascriptEdModeLibrary::SetDefaultMode(FJavascriptEditorModeTools& Tools, FName DefaultID)
{
	Tools.Tools->SetDefaultMode(DefaultID);
}
void UJavascriptEdModeLibrary::ActivateDefaultMode(FJavascriptEditorModeTools& Tools)
{
	Tools.Tools->ActivateDefaultMode();
}
bool UJavascriptEdModeLibrary::IsDefaultModeActive(FJavascriptEditorModeTools& Tools)
{
	return Tools.Tools->IsDefaultModeActive();
}
void UJavascriptEdModeLibrary::ActivateMode(FJavascriptEditorModeTools& Tools, FName InID, bool bToggle)
{
	Tools.Tools->ActivateMode(InID, bToggle);
}
void UJavascriptEdModeLibrary::DeactivateMode(FJavascriptEditorModeTools& Tools, FName InID)
{
	Tools.Tools->DeactivateMode(InID);
}
void UJavascriptEdModeLibrary::DestroyMode(FJavascriptEditorModeTools& Tools, FName InID)
{
	Tools.Tools->DestroyMode(InID);
}
void UJavascriptEdModeLibrary::DeactivateAllModes(FJavascriptEditorModeTools& Tools)
{
	Tools.Tools->DeactivateAllModes();
}
bool UJavascriptEdModeLibrary::EnsureNotInMode(FJavascriptEditorModeTools& Tools, FName ModeID, const FText& ErrorMsg, bool bNotifyUser)
{
	return Tools.Tools->EnsureNotInMode(ModeID, ErrorMsg, bNotifyUser);
}
bool UJavascriptEdModeLibrary::IsModeActive(FJavascriptEditorModeTools& Tools, FName InID)
{
	return Tools.Tools->IsModeActive(InID);
}


#endif