#include "JavascriptEdModeLibrary.h"

#if WITH_EDITOR
#include "EditorModeManager.h"
#include "EdMode.h"
#include "Editor.h"

void UJavascriptEdModeLibrary::SetDefaultMode(FJavascriptEditorModeTools& Tools, FName DefaultID)
{
	Tools->SetDefaultMode(DefaultID);
}
void UJavascriptEdModeLibrary::ActivateDefaultMode(FJavascriptEditorModeTools& Tools)
{
	Tools->ActivateDefaultMode();
}
bool UJavascriptEdModeLibrary::IsDefaultModeActive(FJavascriptEditorModeTools& Tools)
{
	return Tools->IsDefaultModeActive();
}
void UJavascriptEdModeLibrary::ActivateMode(FJavascriptEditorModeTools& Tools, FName InID, bool bToggle)
{
	Tools->ActivateMode(InID, bToggle);
}
void UJavascriptEdModeLibrary::DeactivateMode(FJavascriptEditorModeTools& Tools, FName InID)
{
	Tools->DeactivateMode(InID);
}
void UJavascriptEdModeLibrary::DestroyMode(FJavascriptEditorModeTools& Tools, FName InID)
{
	Tools->DestroyMode(InID);
}
void UJavascriptEdModeLibrary::DeactivateAllModes(FJavascriptEditorModeTools& Tools)
{
	Tools->DeactivateAllModes();
}
bool UJavascriptEdModeLibrary::EnsureNotInMode(FJavascriptEditorModeTools& Tools, FName ModeID, const FText& ErrorMsg, bool bNotifyUser)
{
	return Tools->EnsureNotInMode(ModeID, ErrorMsg, bNotifyUser);
}
bool UJavascriptEdModeLibrary::IsModeActive(FJavascriptEditorModeTools& Tools, FName InID)
{
	return Tools->IsModeActive(InID);
}

bool UJavascriptEdModeLibrary::StartTracking(FJavascriptEditorModeTools Tools, FJavascriptEdViewport Viewport)
{
	return Tools->StartTracking(Viewport.ViewportClient, Viewport.Viewport);
}

bool UJavascriptEdModeLibrary::EndTracking(FJavascriptEditorModeTools Tools, FJavascriptEdViewport Viewport)
{
	return Tools->EndTracking(Viewport.ViewportClient, Viewport.Viewport);
}

bool UJavascriptEdModeLibrary::IsTracking(FJavascriptEditorModeTools Tools)
{
	return Tools->IsTracking();
}

FJavascriptHitProxy UJavascriptEdModeLibrary::GetHitProxy(FJavascriptEdViewport Viewport)
{
	int32 HitX = Viewport.Viewport->GetMouseX();
	int32 HitY = Viewport.Viewport->GetMouseY();
	HHitProxy* HitProxy = Viewport.Viewport->GetHitProxy(HitX, HitY);
	FJavascriptHitProxy Proxy;
	Proxy.HitProxy = HitProxy;
	return Proxy;
}

FJavascriptEditorModeTools UJavascriptEdModeLibrary::GetLevelEditorModeTools()
{
	return FJavascriptEditorModeTools(&GLevelEditorModeTools());
}

FJavascriptEditorModeTools UJavascriptEdModeLibrary::GetModeManager(FJavascriptEditorMode Mode)
{
	return Mode->GetModeManager();
}

int32 UJavascriptEdModeLibrary::GetCurrentWidgetAxis(FJavascriptEditorMode Mode)
{
	return Mode->GetCurrentWidgetAxis();
}

void UJavascriptEdModeLibrary::SetCurrentWidgetAxis(FJavascriptEditorMode Mode, int32 InAxis)
{
	Mode->SetCurrentWidgetAxis((EAxisList::Type)InAxis);
}
void UJavascriptEdModeLibrary::SelectNone(FJavascriptEditorMode Mode)
{
	Mode->SelectNone();
}


#endif
