#include "JavascriptEditor.h"
#include "JavascriptEditorMenu.h"
#include "JavascriptUIExtender.h"
#include "SJavascriptBox.h"

UJavascriptEditorMenu::UJavascriptEditorMenu(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{	
}

#if WITH_EDITOR	
void UJavascriptEditorMenu::Setup(TSharedRef<SBox> Box)
{	
	if (SubMenus.Num())
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
	else
	{
		FMenuBuilder MenuBuilder(true, CommandList.Handle);
		
		MenuBuilder.PushCommandList(CommandList.Handle.ToSharedRef());
		UJavascriptUIExtender::PushMenuBuilder(MenuBuilder);
		this->OnHook.ExecuteIfBound(FName());
		UJavascriptUIExtender::Reset();

		Box->SetContent(MenuBuilder.MakeWidget());
	}	
}

TSharedRef<SWidget> UJavascriptEditorMenu::RebuildWidget()
{	
	auto PrimaryArea = SNew(SJavascriptBox).Widget(this);

	Setup(PrimaryArea);

	return PrimaryArea;
}

#endif
