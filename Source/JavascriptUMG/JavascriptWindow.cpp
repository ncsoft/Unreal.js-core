#include "JavascriptWindow.h"
#include "Widgets/SWindow.h"
#include "Launch/Resources/Version.h"

UJavascriptWindow::UJavascriptWindow(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{	
	Type = EJavascriptWindowType::Normal;
	//Style = FCoreStyle::Get().GetWidgetStyle<FWindowStyle>("Window");
	AutoCenter = EJavascriptAutoCenter::PreferredWorkArea;
	ScreenPosition = FVector2D::ZeroVector;
	ClientSize = FVector2D::ZeroVector;
	SupportsTransparency = EJavascriptWindowTransparency::None;
	InitialOpacity = 1.0f;
	IsInitiallyMaximized = false;
	SizingRule = EJavascriptSizingRule::UserSized;
	IsPopupWindow = false;
	FocusWhenFirstShown = true;
	ActivateWhenFirstShown = true;
	UseOSWindowBorder = false;
	HasCloseButton = true;
	SupportsMaximize = true;
	SupportsMinimize = true;
	CreateTitleBar = true;
	SaneWindowPlacement = true;
	LayoutBorder = FMargin(5, 5, 5, 5);
	UserResizeBorder = FMargin(5, 5, 5, 5);
	bIsCloseRequested = false;
}

TSharedRef<SWidget> UJavascriptWindow::RebuildWidget()
{
	auto Content = (GetChildrenCount() > 0) ? GetContentSlot()->Content : nullptr;

	TSharedRef<SWindow> Window = SNew(SWindow)
		.Type((EWindowType)Type)
		.Style(&FCoreStyle::Get().GetWidgetStyle<FWindowStyle>("Window"))
		.Title(Title)
		.bDragAnywhere(bDragAnywhere)
		.AutoCenter((EAutoCenter)AutoCenter)
		.ScreenPosition(ScreenPosition)
		.ClientSize(ClientSize)
		.SupportsTransparency((EWindowTransparency)SupportsTransparency)
		.InitialOpacity(InitialOpacity)
		.IsInitiallyMaximized(IsInitiallyMaximized)
		.SizingRule((ESizingRule)SizingRule)
		.IsPopupWindow(IsPopupWindow)
		.FocusWhenFirstShown(FocusWhenFirstShown)
		.ActivationPolicy(EWindowActivationPolicy::FirstShown)
		.UseOSWindowBorder(UseOSWindowBorder)
		.HasCloseButton(HasCloseButton)
		.SupportsMaximize(SupportsMaximize)
		.SupportsMinimize(SupportsMinimize)
		.CreateTitleBar(CreateTitleBar)
		.SaneWindowPlacement(SaneWindowPlacement)
		.LayoutBorder(LayoutBorder)
		.UserResizeBorder(UserResizeBorder)
		.IsTopmostWindow(IsTopmostWindow)
		.Content()
			[
				Content == nullptr ? SNullWidget::NullWidget : Content->TakeWidget()
			];

	Window->SetOnWindowClosed(FOnWindowClosed::CreateLambda([&](const TSharedRef<SWindow>&)
	{
		OnWindowClosed.ExecuteIfBound();
	}));

	Window->GetOnWindowDeactivatedEvent().AddUObject(this, &UJavascriptWindow::OnWindowDeactivatedEvent);

	WeakWindow = Window;
	return Window;
}

void UJavascriptWindow::OnWindowDeactivatedEvent()
{
	if (!bIsCloseRequested)
		OnWindowDeactivated.ExecuteIfBound();
}

void UJavascriptWindow::MoveWindowTo(FVector2D NewPosition)
{
	auto MyWindow = GetSlatePtr();
	if (MyWindow.IsValid())
	{
		MyWindow->MoveWindowTo(NewPosition);
	}
}
void UJavascriptWindow::ReshapeWindow(FVector2D NewPosition, FVector2D NewSize)
{
	auto MyWindow = GetSlatePtr();
	if (MyWindow.IsValid())
	{
		MyWindow->ReshapeWindow(NewPosition, NewSize);
	}
}
void UJavascriptWindow::Resize(FVector2D NewSize)
{
	auto MyWindow = GetSlatePtr();
	if (MyWindow.IsValid())
	{
		MyWindow->Resize(NewSize);
	}
}
void UJavascriptWindow::FlashWindow()
{
	auto MyWindow = GetSlatePtr();
	if (MyWindow.IsValid())
	{
		MyWindow->FlashWindow();
	}
}
void UJavascriptWindow::BringToFront()
{
	auto MyWindow = GetSlatePtr();
	if (MyWindow.IsValid())
	{
		MyWindow->BringToFront();
	}
}
void UJavascriptWindow::RequestDestroyWindow()
{
	if (!bIsCloseRequested)
	{
		bIsCloseRequested = true;
		auto MyWindow = GetSlatePtr();
		if (MyWindow.IsValid())
		{
			MyWindow->RequestDestroyWindow();
		}
	}
}
void UJavascriptWindow::DestroyWindowImmediately()
{
	auto MyWindow = GetSlatePtr();
	if (MyWindow.IsValid())
	{
		MyWindow->DestroyWindowImmediately();
	}
}
void UJavascriptWindow::ShowWindow()
{
	auto MyWindow = GetSlatePtr();
	if (MyWindow.IsValid())
	{
		MyWindow->ShowWindow();
	}
}
void UJavascriptWindow::HideWindow()
{
	auto MyWindow = GetSlatePtr();
	if (MyWindow.IsValid())
	{
		MyWindow->HideWindow();
	}
}
void UJavascriptWindow::EnableWindow(bool bEnable)
{
	auto MyWindow = GetSlatePtr();
	if (MyWindow.IsValid())
	{
		MyWindow->EnableWindow(bEnable);
	}
}
void UJavascriptWindow::SetOpacity(const float InOpacity)
{
	auto MyWindow = GetSlatePtr();
	if (MyWindow.IsValid())
	{
		MyWindow->SetOpacity(InOpacity);
	}
}
