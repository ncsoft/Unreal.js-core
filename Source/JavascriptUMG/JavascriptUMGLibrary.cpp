#include "JavascriptUMG.h"
#include "JavascriptUMGLibrary.h"

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