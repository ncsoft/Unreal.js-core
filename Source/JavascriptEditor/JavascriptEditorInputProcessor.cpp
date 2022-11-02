#include "JavascriptEditorInputProcessor.h"
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "UObject/UObjectThreadContext.h"

#if WITH_EDITOR
class FMyInputProcessor : public IInputProcessor
{
public:
	TWeakObjectPtr<UJavascriptEditorInputProcessor> Processor;

	FMyInputProcessor(UJavascriptEditorInputProcessor* InProcessor)
	{
		Processor = InProcessor;
	}

	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor)
	{
		return;
	}

	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
	{
		return !FUObjectThreadContext::Get().IsRoutingPostLoad
			&& Processor.IsValid() && Processor->HandleKeyDownEvent(InKeyEvent);
	}

	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
	{
		return !FUObjectThreadContext::Get().IsRoutingPostLoad
			&& Processor.IsValid() && Processor->HandleKeyUpEvent(InKeyEvent);
	}

	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& InPointerEvent)
	{
		return !FUObjectThreadContext::Get().IsRoutingPostLoad
			&& Processor.IsValid() && Processor->HandleMouseMoveEvent(InPointerEvent);
	}
	
	virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent)
	{
		return !FUObjectThreadContext::Get().IsRoutingPostLoad
			&& Processor.IsValid() && Processor->HandleAnalogInputEvent(InAnalogInputEvent);
	}

	/** Mouse button press */
	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
	{
		return !FUObjectThreadContext::Get().IsRoutingPostLoad
			&& Processor.IsValid() && Processor->HandleMouseButtonDownEvent(MouseEvent);
	}

	/** Mouse button release */
	virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
	{
		return !FUObjectThreadContext::Get().IsRoutingPostLoad
			&& Processor.IsValid() && Processor->HandleMouseButtonUpEvent(MouseEvent);
	}

	/** Mouse button double clicked. */
	virtual bool HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
	{
		return !FUObjectThreadContext::Get().IsRoutingPostLoad
			&& Processor.IsValid() && Processor->HandleMouseButtonDoubleClickEvent(MouseEvent);
	}

	/** Mouse wheel input */
	virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent)
	{
		FPointerEvent GestureEvent = FPointerEvent();
		if (InGestureEvent)
		{
			GestureEvent = *InGestureEvent;
		}
		return !FUObjectThreadContext::Get().IsRoutingPostLoad
			&& Processor.IsValid() && Processor->HandleMouseWheelOrGestureEvent(InWheelEvent, GestureEvent);
	}
};

void UJavascriptEditorInputProcessor::BeginDestroy()
{
	Activate(false);

	Super::BeginDestroy();
}

void UJavascriptEditorInputProcessor::Activate(bool bActivate)
{
	if (bActivate)
	{
		if (bActivated) return;
		bActivated = true;
		InputProcessor = MakeShareable(new FMyInputProcessor(this));
		FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor);
	}
	else
	{
		if (!bActivated) return;
		bActivated = false;

		if (InputProcessor.IsValid())
		{
			FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);
		}
	}
}

void UJavascriptEditorInputProcessor::Register()
{
	InputProcessor = MakeShareable(new FMyInputProcessor(this));
	FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor);
}

void UJavascriptEditorInputProcessor::UnRegister()
{
	if (InputProcessor.IsValid() && FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);
	}
}
#endif
