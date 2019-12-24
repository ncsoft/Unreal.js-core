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
	//@hack: OnGenerateRowEvent등의 Non-userwidget slate를 받는 패턴에서, &this.StrikeBrush를 ptr로 넘기기 때문에 widget-slate 의존성 생김. 임시로 disabled.
	MyTextBlock->SetStrikeBrush(TAttribute<const FSlateBrush*>());
}
