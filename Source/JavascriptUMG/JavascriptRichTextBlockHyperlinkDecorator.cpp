#include "JavascriptRichTextBlockHyperlinkDecorator.h"
#include "Widgets/Text/SRichTextBlock.h"

TSharedPtr<ITextDecorator> UJavascriptRichTextBlockHyperlinkDecorator::CreateDecorator(URichTextBlock* InOwner)
{
	return SRichTextBlock::HyperlinkDecorator(HyperlinkId, FSlateHyperlinkRun::FOnClick::CreateLambda([this](const FSlateHyperlinkRun::FMetadata& metadata) {
		Current = &metadata;
		this->OnClick.Broadcast(this);
		Current = nullptr;
	}));
}

FString UJavascriptRichTextBlockHyperlinkDecorator::GetMetadata(const FString& Key)
{
	if (Current)
	{
		auto Value = Current->Find(Key);
		if (Value)
		{
			return *Value;
		}
	}
	
	return TEXT("");
}