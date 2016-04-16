#include "JavascriptEditor.h"
#include "JavascriptEditorMenu.h"
#include "JavascriptUIExtender.h"

UJavascriptEditorMenu::UJavascriptEditorMenu(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{	
}

#if WITH_EDITOR	
void UJavascriptEditorMenu::Setup(TSharedRef<SBox> Box)
{	
	FMenuBarBuilder MenuBarBuilder = FMenuBarBuilder(CommandList.Handle);
	for (const auto& SubMenu : SubMenus)
	{
		auto Id = SubMenu.Id;
		MenuBarBuilder.AddPullDownMenu(
			SubMenu.Label,
			SubMenu.Tooltip,
			FNewMenuDelegate::CreateLambda([this, Id](FMenuBuilder& MenuBuilder) {
				MenuBuilder.PushCommandList(CommandList.Handle.ToSharedRef());
				UJavascriptUIExtender::PushMenuBuilder(MenuBuilder);
				this->OnHook.ExecuteIfBound(Id);
				UJavascriptUIExtender::Reset();
			}),
			Id
		);
	}	

	Box->SetContent(MenuBarBuilder.MakeWidget());
}

TSharedRef<SWidget> UJavascriptEditorMenu::RebuildWidget()
{	
	auto PrimaryArea = SNew(SBox);			

	Setup(PrimaryArea);

	return PrimaryArea;
}

#endif
