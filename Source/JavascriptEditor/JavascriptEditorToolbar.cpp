#include "JavascriptEditor.h"
#include "JavascriptEditorToolbar.h"

#if WITH_EDITOR	
void UJavascriptEditorToolbar::Setup(TSharedRef<SBox> Box)
{	
	auto Builder = OnHook.Execute();	

	if (Builder.Handle.IsValid())
	{
		Box->SetContent(Builder.Handle->MakeWidget());
	}
	else
	{
		Box->SetContent(SNew(SSpacer));
	}	
}

TSharedRef<SWidget> UJavascriptEditorToolbar::RebuildWidget()
{	
	auto PrimaryArea = SNew(SBox);

	Setup(PrimaryArea);

	return PrimaryArea;
}

#endif
