#include "JavascriptEditor.h"
#include "JavascriptEditorToolbar.h"
#include "JavascriptUIExtender.h"

UJavascriptEditorToolbar::UJavascriptEditorToolbar(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{	
}

#if WITH_EDITOR	
namespace 
{
	static FToolBarBuilder* CurrentToolBarBuilder{ nullptr };
}
void UJavascriptEditorToolbar::Setup(TSharedRef<SBox> Box)
{	
	FToolBarBuilder ToolBarBuilder = FToolBarBuilder(CommandList.Handle, FMultiBoxCustomization::None);
	CurrentToolBarBuilder = &ToolBarBuilder;
	OnHook.ExecuteIfBound("ToolBar");	
	CurrentToolBarBuilder = nullptr;

	Box->SetContent(ToolBarBuilder.MakeWidget());
}

void UJavascriptEditorToolbar::AddToolBarButton(FJavascriptUICommandInfo CommandInfo)
{
	CurrentToolBarBuilder->AddToolBarButton(CommandInfo.Handle);
}

TSharedRef<SWidget> UJavascriptEditorToolbar::RebuildWidget()
{	
	auto PrimaryArea = SNew(SBox);

	Setup(PrimaryArea);

	return PrimaryArea;
}

#endif
