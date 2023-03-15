#include "JavascriptComboButton.h"
#include "Widgets/Input/SComboButton.h"

PRAGMA_DISABLE_SHADOW_VARIABLE_WARNINGS

UJavascriptComboButton::UJavascriptComboButton(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{	
}

TSharedRef<SWidget> UJavascriptComboButton::RebuildWidget()
{
	auto ContentContainer = SNew(SBorder)
		.BorderImage(FStyleDefaults::GetNoBrush())
		.Padding(0.0f)
		[
			GetComboBox()
		];

	MyContentContainer = ContentContainer;
	return ContentContainer;
}

TSharedRef<SWidget> UJavascriptComboButton::GetComboBox()
{
	auto Content = (GetChildrenCount() > 0) ? GetContentSlot()->Content : nullptr;
	
	auto ComboButton = SNew(SComboButton)
		.ComboButtonStyle(&ComboButtonStyle)
		.ButtonStyle(&ButtonStyle)
		.OnMenuOpenChanged(BIND_UOBJECT_DELEGATE(::FOnIsOpenChanged, HandleMenuOpenChanged))
		.OnComboBoxOpened(BIND_UOBJECT_DELEGATE(::FOnComboBoxOpened, HandleComboBoxOpened))
		.ButtonContent()
		[
			Content == nullptr ? SNullWidget::NullWidget : Content->TakeWidget()
		]
		.OnGetMenuContent_Lambda([this]() {
			if (OnGetMenuContent.IsBound()) 
			{
				auto Content = OnGetMenuContent.Execute();
				return Content.Widget.IsValid() ? Content.Widget.ToSharedRef() : SNullWidget::NullWidget;
			}
			else
			{
				return SNullWidget::NullWidget;
			}
		})
		.IsFocusable(bIsFocusable)
		.HasDownArrow(bHasDownArrow)
		.ForegroundColor(ForegroundColor)
		.ButtonColorAndOpacity(ButtonColorAndOpacity)
		.ContentPadding(ContentPadding)
		.MenuPlacement(MenuPlacement)
		.HAlign(HAlign)
		.VAlign(VAlign);

	MyComboButton = ComboButton;
	return ComboButton;
}

void UJavascriptComboButton::OnSlotAdded(UPanelSlot* InSlot)
{
	if (MyContentContainer.IsValid())
	{
		MyContentContainer->SetContent(GetComboBox());
	}
}

void UJavascriptComboButton::HandleComboBoxOpened()
{
	OnComboBoxOpened.ExecuteIfBound();
}

void UJavascriptComboButton::HandleMenuOpenChanged(bool bOpen)
{
	OnMenuOpenChanged.ExecuteIfBound(bOpen);
}

void UJavascriptComboButton::SynchronizeProperties()
{
	Super::SynchronizeProperties();
}

void UJavascriptComboButton::SetIsOpen(bool InIsOpen, bool bFocusMenu)
{
	if (MyComboButton.IsValid())
	{
		return MyComboButton->SetIsOpen(InIsOpen, bFocusMenu);
	}
}

void UJavascriptComboButton::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyComboButton.Reset();
}

PRAGMA_ENABLE_SHADOW_VARIABLE_WARNINGS
