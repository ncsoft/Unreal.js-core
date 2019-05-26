#include "JavascriptTextBlock.h"

UJavascriptTextBlock::UJavascriptTextBlock(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{	
}

void UJavascriptTextBlock::SetHighlightText(FText InHighlightText)
{
	HighlightText = InHighlightText;
	if (MyTextBlock.IsValid())
	{
		MyTextBlock->SetHighlightText(HighlightText);
	}
}

void UJavascriptTextBlock::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	TAttribute<FText> HighlightTextBinding = PROPERTY_BINDING(FText, HighlightText);

	MyTextBlock->SetHighlightText(HighlightTextBinding);	
}
