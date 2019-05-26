#pragma once
#include "EditorViewportClient.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/ScriptMacros.h"
#include "JavascriptInputEventStateLibrary.generated.h"

USTRUCT(BlueprintType)
struct FJavascriptInputEventState
{
	GENERATED_BODY()

	FJavascriptInputEventState() 
	{}

	FJavascriptInputEventState(const FInputEventState& InInputEvent)
		: InputEvent(&InInputEvent)
	{}

	const FInputEventState* InputEvent;
};

/**
 * 
 */
UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptInputEventStateLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, Category = "Javascript | InputEvent")
	static EInputEvent GetInputEvent(const FJavascriptInputEventState& InputEvent);
	UFUNCTION(BlueprintCallable, Category = "Javascript | InputEvent")
	static FKey GetKey(const FJavascriptInputEventState& InputEvent);

	/** return true if the event causing button is a control key */
	UFUNCTION(BlueprintCallable, Category = "Javascript | InputEvent")
	static bool IsCtrlButtonEvent(const FJavascriptInputEventState& InputEvent);
	UFUNCTION(BlueprintCallable, Category = "Javascript | InputEvent")
	static bool IsShiftButtonEvent(const FJavascriptInputEventState& InputEvent);
	UFUNCTION(BlueprintCallable, Category = "Javascript | InputEvent")
	static bool IsAltButtonEvent(const FJavascriptInputEventState& InputEvent);

	UFUNCTION(BlueprintCallable, Category = "Javascript | InputEvent")
	static bool IsLeftMouseButtonPressed(const FJavascriptInputEventState& InputEvent);
	UFUNCTION(BlueprintCallable, Category = "Javascript | InputEvent")
	static bool IsMiddleMouseButtonPressed(const FJavascriptInputEventState& InputEvent);
	UFUNCTION(BlueprintCallable, Category = "Javascript | InputEvent")
	static bool IsRightMouseButtonPressed(const FJavascriptInputEventState& InputEvent);

	UFUNCTION(BlueprintCallable, Category = "Javascript | InputEvent")
	static bool IsMouseButtonEvent(const FJavascriptInputEventState& InputEvent);
	UFUNCTION(BlueprintCallable, Category = "Javascript | InputEvent")
	static bool IsButtonPressed(const FJavascriptInputEventState& InputEvent, FKey InKey);
	UFUNCTION(BlueprintCallable, Category = "Javascript | InputEvent")
	static bool IsAnyMouseButtonDown(const FJavascriptInputEventState& InputEvent);

	/** return true if alt is pressed right now.  This will be true even if the event was for a different key but an alt key is currently pressed */
	UFUNCTION(BlueprintCallable, Category = "Javascript | InputEvent")
	static bool IsAltButtonPressed(const FJavascriptInputEventState& InputEvent);
	UFUNCTION(BlueprintCallable, Category = "Javascript | InputEvent")
	static bool IsShiftButtonPressed(const FJavascriptInputEventState& InputEvent);
	UFUNCTION(BlueprintCallable, Category = "Javascript | InputEvent")
	static bool IsCtrlButtonPressed(const FJavascriptInputEventState& InputEvent);
	UFUNCTION(BlueprintCallable, Category = "Javascript | InputEvent")
	static bool IsSpaceBarPressed(const FJavascriptInputEventState& InputEvent);
#endif
};
