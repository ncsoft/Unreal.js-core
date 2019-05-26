#include "JavascriptEditorToolbar.h"
#include "JavascriptUMG/SJavascriptBox.h"
#include "Widgets/Layout/SSpacer.h"

#if WITH_EDITOR	
void UJavascriptEditorToolbar::Setup(TSharedRef<SBox> Box)
{	
	auto Builder = OnHook.Execute();	

	if (Builder.MultiBox)
	{
		Box->SetContent(Builder.MultiBox->MakeWidget());
	}
	else
	{
		Box->SetContent(SNew(SSpacer));
	}	
}

TSharedRef<SWidget> UJavascriptEditorToolbar::RebuildWidget()
{	
	auto PrimaryArea = SNew(SJavascriptBox).Widget(this);

	Setup(PrimaryArea);

	return PrimaryArea;
}

#endif
