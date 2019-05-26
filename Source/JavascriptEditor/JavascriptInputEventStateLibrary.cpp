#include "JavascriptInputEventStateLibrary.h"

#if WITH_EDITOR
EInputEvent UJavascriptInputEventStateLibrary::GetInputEvent(const FJavascriptInputEventState& InputEvent)
{
	return InputEvent.InputEvent->GetInputEvent();
}

FKey UJavascriptInputEventStateLibrary::GetKey(const FJavascriptInputEventState& InputEvent)
{
	return InputEvent.InputEvent->GetKey();
}

/** return true if the event causing button is a control key */

bool UJavascriptInputEventStateLibrary::IsCtrlButtonEvent(const FJavascriptInputEventState& InputEvent)
{
	return InputEvent.InputEvent->IsCtrlButtonEvent();
}

bool UJavascriptInputEventStateLibrary::IsShiftButtonEvent(const FJavascriptInputEventState& InputEvent)
{
	return InputEvent.InputEvent->IsShiftButtonEvent();
}

bool UJavascriptInputEventStateLibrary::IsAltButtonEvent(const FJavascriptInputEventState& InputEvent)
{
	return InputEvent.InputEvent->IsAltButtonEvent();
}


bool UJavascriptInputEventStateLibrary::IsLeftMouseButtonPressed(const FJavascriptInputEventState& InputEvent)
{
	return InputEvent.InputEvent->IsLeftMouseButtonPressed();
}

bool UJavascriptInputEventStateLibrary::IsMiddleMouseButtonPressed(const FJavascriptInputEventState& InputEvent)
{
	return InputEvent.InputEvent->IsMiddleMouseButtonPressed();
}

bool UJavascriptInputEventStateLibrary::IsRightMouseButtonPressed(const FJavascriptInputEventState& InputEvent)
{
	return InputEvent.InputEvent->IsRightMouseButtonPressed();
}


bool UJavascriptInputEventStateLibrary::IsMouseButtonEvent(const FJavascriptInputEventState& InputEvent)
{
	return InputEvent.InputEvent->IsMouseButtonEvent();
}

bool UJavascriptInputEventStateLibrary::IsButtonPressed(const FJavascriptInputEventState& InputEvent, FKey InKey)
{
	return InputEvent.InputEvent->IsButtonPressed(InKey);
}

bool UJavascriptInputEventStateLibrary::IsAnyMouseButtonDown(const FJavascriptInputEventState& InputEvent)
{
	return InputEvent.InputEvent->IsAnyMouseButtonDown();
}

/** return true if alt is pressed right now.  This will be true even if the event was for a different key but an alt key is currently pressed */

bool UJavascriptInputEventStateLibrary::IsAltButtonPressed(const FJavascriptInputEventState& InputEvent)
{
	return InputEvent.InputEvent->IsAltButtonPressed();
}

bool UJavascriptInputEventStateLibrary::IsShiftButtonPressed(const FJavascriptInputEventState& InputEvent)
{
	return InputEvent.InputEvent->IsShiftButtonPressed();
}

bool UJavascriptInputEventStateLibrary::IsCtrlButtonPressed(const FJavascriptInputEventState& InputEvent)
{
	return InputEvent.InputEvent->IsCtrlButtonPressed();
}

bool UJavascriptInputEventStateLibrary::IsSpaceBarPressed(const FJavascriptInputEventState& InputEvent)
{
	return InputEvent.InputEvent->IsSpaceBarPressed();
}

#endif