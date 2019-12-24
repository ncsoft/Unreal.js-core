#include "JavascriptWidget.h"
#include "JavascriptContext.h"
#include "Blueprint/WidgetTree.h"
#include "Runtime/Launch/Resources/Version.h"

UJavascriptWidget::UJavascriptWidget(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	WidgetTree = CreateDefaultSubobject<UWidgetTree>(TEXT("WidgetTree"));
	WidgetTree->SetFlags(RF_Transactional);
}

void UJavascriptWidget::SetRootWidget(UWidget* Widget)
{
	WidgetTree->RootWidget = Widget;
}

void UJavascriptWidget::ProcessEvent(UFunction* Function, void* Parms)
{
	if (JavascriptContext && JavascriptContext->CallProxyFunction(this, this, Function, Parms))
	{
		return;
	}

	Super::ProcessEvent(Function, Parms);	
}

void UJavascriptWidget::CallSynchronizeProperties(UVisual* Visual)
{
	if (auto Widget = Cast<UWidget>(Visual))
	{
		TSharedPtr<SWidget> SafeWidget = Widget->GetCachedWidget();
		if (SafeWidget.IsValid())
		{
			Widget->SynchronizeProperties();
		}
	}
	else if (auto Slot = Cast<UPanelSlot>(Visual))
	{
		Slot->SynchronizeProperties();
	}	
}

bool UJavascriptWidget::HasValidCachedWidget(UWidget* Widget)
{
	return Widget && Widget->GetCachedWidget().IsValid();
}

UPanelSlot* UJavascriptWidget::AddChild(UWidget* Content)
{

	if (Content == nullptr)
	{
		return nullptr;
	}

	Content->RemoveFromParent();

	if (!ContentSlot)
	{
		ContentSlot = NewObject<UPanelSlot>(this, GetSlotClass());
		ContentSlot->SetFlags(RF_Transactional);
	}

	ContentSlot->Content = Content;
	
	if (Content)
	{
		Content->Slot = ContentSlot;
	}

	OnSlotAdded(Slot);

	SetRootWidget(Content);

	return ContentSlot;
}


bool UJavascriptWidget::RemoveChild()
{
	if (!ContentSlot) return false;

	if (ContentSlot->Content)
	{
		ContentSlot->Content->Slot = nullptr;
	}

	OnSlotRemoved(Slot);

	const bool bReleaseChildren = true;
	ContentSlot->ReleaseSlateResources(bReleaseChildren);

	ContentSlot->Parent = nullptr;
	ContentSlot->Content = nullptr;
	ContentSlot = nullptr;
	return true;
}

void UJavascriptWidget::OnListenForInputAction(FName ActionName, TEnumAsByte< EInputEvent > EventType, bool bConsume)
{
	if (!InputComponent)
	{
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 12
		if (APlayerController* Controller = GetOwningPlayer())
		{
			InputComponent = NewObject< UInputComponent >(this, NAME_None, RF_Transient);
			InputComponent->bBlockInput = bStopAction;
			InputComponent->Priority = Priority;
			Controller->PushInputComponent(InputComponent);
		}		
#else
		InitializeInputComponent();
#endif
	}

	if (InputComponent)
	{
		FInputActionBinding NewBinding(ActionName, EventType.GetValue());
		NewBinding.bConsumeInput = bConsume;
		if (EventType == IE_Pressed)
		{
			NewBinding.ActionDelegate.GetDelegateForManualSet().BindUObject(this, &ThisClass::OnInputActionByName, ActionName);
		}
		else
		{
			NewBinding.ActionDelegate.GetDelegateForManualSet().BindUObject(this, &ThisClass::OnReleaseInputActionByName, ActionName);
		}

		InputComponent->AddActionBinding(NewBinding);
	}
}

void UJavascriptWidget::OnInputActionByName_Implementation(FName ActionName)
{
	if (OnInputActionEvent.IsBound())
	{
		OnInputActionEvent.Broadcast(ActionName);
	}
}

void UJavascriptWidget::OnReleaseInputActionByName_Implementation(FName ActionName)
{
	if (OnReleaseActionEvent.IsBound())
	{
		OnReleaseActionEvent.Broadcast(ActionName);
	}
}

void UJavascriptWidget::OnListenForInputAxis(FName AxisName, TEnumAsByte< EInputEvent > EventType, bool bConsume)
{
	if (!InputComponent)
	{
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 12
		if (APlayerController* Controller = GetOwningPlayer())
		{
			InputComponent = NewObject< UInputComponent >(this, NAME_None, RF_Transient);
			InputComponent->bBlockInput = bStopAction;
			InputComponent->Priority = Priority;
			Controller->PushInputComponent(InputComponent);
		}
#else
		InitializeInputComponent();
#endif
	}

	if (InputComponent)
	{
		FInputAxisBinding NewBinding(AxisName);
		NewBinding.bConsumeInput = bConsume;
		NewBinding.AxisDelegate.GetDelegateForManualSet().BindUObject(this, &ThisClass::OnInputAxisByName, AxisName);
		InputComponent->AxisBindings.Add(NewBinding);
	}
}

void UJavascriptWidget::OnInputAxisByName_Implementation(float Axis, FName AxisName)
{
	if (OnInputAxisEvent.IsBound())
	{
		OnInputAxisEvent.Broadcast(Axis, AxisName);
	}
}

void UJavascriptWidget::ReleaseSlateResources(bool bReleaseChildren)
{
	if (CanSafelyRouteEvent())
	{
		OnDestroy(bReleaseChildren);
	}

	Super::ReleaseSlateResources(bReleaseChildren);
}
