#include "JavascriptUMG.h"
#include "JavascriptWidget.h"
#include "JavascriptContext.h"
#include "Blueprint/WidgetTree.h"

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

	return true;
}
