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

UWidget* UJavascriptUMGLibrary::SetContent(UNativeWidgetHost* TargetWidget, UWidget* SourceWidget)
{
	UWidget* Widget = nullptr;
	if (TargetWidget && SourceWidget)
	{
		TargetWidget->SetContent(SourceWidget->TakeWidget());
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
	if (NewWindow && RootWindow)
	{
		auto New = StaticCastSharedPtr<SWindow>(TSharedPtr<SWidget>(NewWindow->TakeWidget()));
		auto Root = StaticCastSharedPtr<SWindow>(TSharedPtr<SWidget>(RootWindow->TakeWidget()));

		if (New.IsValid() && Root.IsValid())
		{
			FSlateApplication::Get().AddWindowAsNativeChild(New.ToSharedRef(), Root.ToSharedRef());
		}
	}	
}

void UJavascriptUMGLibrary::AddWindow(UWidget* NewWindow, const bool bShowImmediately)
{
	if (NewWindow)
	{
		auto New = StaticCastSharedPtr<SWindow>(TSharedPtr<SWidget>(NewWindow->TakeWidget()));

		if (New.IsValid())
		{
			FSlateApplication::Get().AddWindow(New.ToSharedRef(), bShowImmediately);
		}
	}	
}

void UJavascriptUMGLibrary::ShowWindow(UWidget* NewWindow)
{
	if (NewWindow)
	{
		auto New = StaticCastSharedPtr<SWindow>(TSharedPtr<SWidget>(NewWindow->TakeWidget()));

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
}

FVector2D UJavascriptUMGLibrary::GenerateDynamicImageResource(const FName InDynamicBrushName)
{
	FIntPoint Size = FSlateApplication::Get().GetRenderer()->GenerateDynamicImageResource(InDynamicBrushName);
	return FVector2D(Size.X, Size.Y);
}

UWidget* UJavascriptUMGLibrary::CreateContainerWidget(TSharedRef<SWidget> Slate)
{
	UWidget* Widget = nullptr;
	UPackage* Package = ::CreatePackage(nullptr, TEXT("/Script/Javascript"));
	UNativeWidgetHost* NativeWidget = NewObject<UNativeWidgetHost>(Package, UNativeWidgetHost::StaticClass(), NAME_None, RF_Transactional);
	if (NativeWidget)
	{
		NativeWidget->SetContent(Slate);
		Widget = Cast<UWidget>(NativeWidget);
	}

	return Widget;
}
