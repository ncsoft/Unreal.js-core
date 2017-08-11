#include "JavascriptEditorInputProcessor.h"
#include "Framework/Application/IInputProcessor.h"
#include "SlateApplication.h"

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
		return Processor.IsValid() && Processor->HandleKeyDownEvent(InKeyEvent);
	}

	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
	{
		return Processor.IsValid() && Processor->HandleKeyUpEvent(InKeyEvent);
	}

	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& InPointerEvent)
	{
		return Processor.IsValid() && Processor->HandleMouseMoveEvent(InPointerEvent);
	}
	
	virtual bool HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent)
	{
		return Processor.IsValid() && Processor->HandleAnalogInputEvent(InAnalogInputEvent);
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
		else
		{
			FSlateApplication::Get().UnregisterAllInputPreProcessors();
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
	if (InputProcessor.IsValid())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);
	}
}
#endif