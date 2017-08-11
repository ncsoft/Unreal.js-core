#include "JavascriptSearchBox.h"
#include "SSearchBox.h"

UJavascriptSearchBox::UJavascriptSearchBox(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{	
}

TSharedRef<SWidget> UJavascriptSearchBox::RebuildWidget()
{
	MySearchBox = SNew(SSearchBox)

		.OnTextChanged_Lambda([this](const FText& InText) { OnTextChanged.Broadcast(InText); })
		.OnTextCommitted_Lambda([this](const FText& InText, ETextCommit::Type CommitMethod) { OnTextCommitted.Broadcast(InText, CommitMethod); })
		;

	return MySearchBox.ToSharedRef();
}

void UJavascriptSearchBox::SetText(FText InText)
{
	Text = InText;
	if (MySearchBox.IsValid())
	{
		MySearchBox->SetText(Text);
	}
}

void UJavascriptSearchBox::SetHintText(FText InHintText)
{
	HintText = InHintText;
	if (MySearchBox.IsValid())
	{
		MySearchBox->SetHintText(HintText);
	}
}

void UJavascriptSearchBox::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	TAttribute<FText> TextBinding = PROPERTY_BINDING(FText, Text);
	TAttribute<FText> HintTextBinding = PROPERTY_BINDING(FText, HintText);

	MySearchBox->SetText(TextBinding);
	MySearchBox->SetHintText(HintTextBinding);	
}
