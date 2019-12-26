#include "JavascriptUMGLibrary.h"
#include "JavascriptWidget.h"
#include "Components/NativeWidgetHost.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Docking/TabManager.h"

FJavascriptSlateStyle UJavascriptUMGLibrary::CreateSlateStyle(FName InStyleSetName)
{
	FJavascriptSlateStyle Out;
	Out.Handle = MakeShareable(new FSlateStyleSet(InStyleSetName));
	return Out;
}

void UJavascriptUMGLibrary::Register(FJavascriptSlateStyle StyleSet)
{
	if (FSlateStyleRegistry::FindSlateStyle(StyleSet.Handle->GetStyleSetName()))
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Handle.Get());
	}
	FSlateStyleRegistry::RegisterSlateStyle(*StyleSet.Handle.Get());
}

void UJavascriptUMGLibrary::Unregister(FJavascriptSlateStyle StyleSet)
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleSet.Handle.Get());
}

void UJavascriptUMGLibrary::SetContentRoot(FJavascriptSlateStyle StyleSet, const FString& InContentRootDir)
{
	StyleSet.Handle->SetContentRoot(InContentRootDir);
}

void UJavascriptUMGLibrary::SetCoreContentRoot(FJavascriptSlateStyle StyleSet, const FString& InCoreContentRootDir)
{
	StyleSet.Handle->SetCoreContentRoot(InCoreContentRootDir);
}

FString UJavascriptUMGLibrary::RootToContentDir(FJavascriptSlateStyle StyleSet, const FString& RelativePath)
{
	return StyleSet.Handle->RootToContentDir(RelativePath);
}

FString UJavascriptUMGLibrary::RootToCoreContentDir(FJavascriptSlateStyle StyleSet, const FString& RelativePath)
{
	return StyleSet.Handle->RootToCoreContentDir(RelativePath);
}

void UJavascriptUMGLibrary::AddImageBrush(FJavascriptSlateStyle StyleSet, FName PropertyName, const FString& InImageName, const FVector2D& InImageSize, const FLinearColor& InTint, ESlateBrushTileType::Type InTiling, ESlateBrushImageType::Type InImageType)
{
	StyleSet.Handle->Set(PropertyName, new FSlateImageBrush(InImageName, InImageSize, InTint, InTiling, InImageType));
}

void UJavascriptUMGLibrary::AddBorderBrush(FJavascriptSlateStyle StyleSet, FName PropertyName, const FString& InImageName, const FMargin& InMargin, const FLinearColor& InColorAndOpacity, ESlateBrushImageType::Type InImageType)
{
	StyleSet.Handle->Set(PropertyName, new FSlateBorderBrush(InImageName, InMargin, InColorAndOpacity, InImageType));
}

void UJavascriptUMGLibrary::AddBoxBrush(FJavascriptSlateStyle StyleSet, FName PropertyName, const FString& InImageName, const FMargin& InMargin, const FLinearColor& InColorAndOpacity, ESlateBrushImageType::Type InImageType)
{
	StyleSet.Handle->Set(PropertyName, new FSlateBoxBrush(InImageName, InMargin, InColorAndOpacity, InImageType));
}

void UJavascriptUMGLibrary::AddSound(FJavascriptSlateStyle StyleSet, FName PropertyName, const FSlateSound& Sound)
{
	StyleSet.Handle->Set(PropertyName, Sound);
}

void UJavascriptUMGLibrary::AddFontInfo(FJavascriptSlateStyle StyleSet, FName PropertyName, const FSlateFontInfo& FontInfo)
{
	StyleSet.Handle->Set(PropertyName, FontInfo);
}

FJavascriptSlateWidget UJavascriptUMGLibrary::TakeWidget(UWidget* Widget)
{
	FJavascriptSlateWidget Out;
	if (Widget)
	{
		Out.Widget = Widget->TakeWidget();
	}
	return Out;
}

UWidget* UJavascriptUMGLibrary::SetContent(UNativeWidgetHost* TargetWidget, FJavascriptSlateWidget SlateWidget)
{
	UWidget* Widget = nullptr;
	if (TargetWidget != nullptr && SlateWidget.Widget.IsValid())
	{
		TargetWidget->SetContent(SlateWidget.Widget.ToSharedRef());
		Widget = Cast<UWidget>(TargetWidget);
	}

	return Widget;
}

FJavascriptSlateWidget UJavascriptUMGLibrary::GetRootWindow()
{
	FJavascriptSlateWidget Out;
	Out.Widget = FGlobalTabmanager::Get()->GetRootWindow();

	return Out;
}

void UJavascriptUMGLibrary::AddWindowAsNativeChild(FJavascriptSlateWidget NewWindow, FJavascriptSlateWidget RootWindow)
{
	auto New = StaticCastSharedPtr<SWindow>(NewWindow.Widget);
	auto Root = StaticCastSharedPtr<SWindow>(RootWindow.Widget);

	if (New.IsValid() && Root.IsValid())
	{
		FSlateApplication::Get().AddWindowAsNativeChild(New.ToSharedRef(), Root.ToSharedRef());
	}
}

void UJavascriptUMGLibrary::AddWindow(FJavascriptSlateWidget NewWindow, const bool bShowImmediately)
{
	auto New = StaticCastSharedPtr<SWindow>(NewWindow.Widget);

	if (New.IsValid())
	{
		FSlateApplication::Get().AddWindow(New.ToSharedRef(), bShowImmediately);
	}
}

void UJavascriptUMGLibrary::ShowWindow(FJavascriptSlateWidget NewWindow)
{
	auto New = StaticCastSharedPtr<SWindow>(NewWindow.Widget);

	if (New.IsValid())
	{
		auto SlateWindow = New.ToSharedRef();
		SlateWindow->ShowWindow();
		//@todo Slate: Potentially dangerous and annoying if all slate windows that are created steal focus.
		if (SlateWindow->SupportsKeyboardFocus() && SlateWindow->IsFocusedInitially())
		{
			SlateWindow->GetNativeWindow()->SetWindowFocus();
		}
	}
}

FVector2D UJavascriptUMGLibrary::GenerateDynamicImageResource(const FName InDynamicBrushName)
{
	FIntPoint Size = FSlateApplication::Get().GetRenderer()->GenerateDynamicImageResource(InDynamicBrushName);
	return FVector2D(Size.X, Size.Y);
}
