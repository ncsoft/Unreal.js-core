#include "JavascriptWindow.h"
#include "SWindow.h"
#include "Modules/ModuleVersion.h"

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
}

TSharedRef<SWidget> UJavascriptWindow::RebuildWidget()
{
	auto Content = (GetChildrenCount() > 0) ? GetContentSlot()->Content : nullptr;

	MyWindow = SNew(SWindow)
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
		.Content()
			[
				Content == nullptr ? SNullWidget::NullWidget : Content->TakeWidget()
			];

	return BuildDesignTimeWidget(MyWindow.ToSharedRef());
}

void UJavascriptWindow::MoveWindowTo(FVector2D NewPosition)
{
	if (MyWindow.IsValid())
	{
		MyWindow->MoveWindowTo(NewPosition);
	}
}
void UJavascriptWindow::ReshapeWindow(FVector2D NewPosition, FVector2D NewSize)
{
	if (MyWindow.IsValid())
	{
		MyWindow->ReshapeWindow(NewPosition, NewSize);
	}
}
void UJavascriptWindow::Resize(FVector2D NewSize)
{
	if (MyWindow.IsValid())
	{
		MyWindow->Resize(NewSize);
	}
}
void UJavascriptWindow::FlashWindow()
{
	if (MyWindow.IsValid())
	{
		MyWindow->FlashWindow();
	}
}
void UJavascriptWindow::BringToFront()
{
	if (MyWindow.IsValid())
	{
		MyWindow->BringToFront();
	}
}
void UJavascriptWindow::RequestDestroyWindow()
{
	if (MyWindow.IsValid())
	{
		MyWindow->RequestDestroyWindow();
	}
}
void UJavascriptWindow::DestroyWindowImmediately()
{
	if (MyWindow.IsValid())
	{
		MyWindow->DestroyWindowImmediately();
	}
}
void UJavascriptWindow::ShowWindow()
{
	if (MyWindow.IsValid())
	{
		MyWindow->ShowWindow();
	}
}
void UJavascriptWindow::HideWindow()
{
	if (MyWindow.IsValid())
	{
		MyWindow->HideWindow();
	}
}
void UJavascriptWindow::EnableWindow(bool bEnable)
{
	if (MyWindow.IsValid())
	{
		MyWindow->EnableWindow(bEnable);
	}
}
void UJavascriptWindow::SetOpacity(const float InOpacity)
{
	if (MyWindow.IsValid())
	{
		MyWindow->SetOpacity(InOpacity);
	}
}
