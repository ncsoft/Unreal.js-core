#include "JavascriptUMG.h"
#include "JavascriptMultiBox.h"
#include "SJavascriptBox.h"

void UJavascriptMultiBox::Setup(TSharedRef<SBox> Box)
{	
	auto Builder = OnHook.Execute(FName("Main"),this,FJavascriptMenuBuilder());

	if (IsValidBuilder(Builder.MultiBox))
	{
		Box->SetContent(Builder.MultiBox->MakeWidget());
	}
	else
	{
		Box->SetContent(SNew(SSpacer));
	}	
}

TSharedRef<SWidget> UJavascriptMultiBox::RebuildWidget()
{	
	auto PrimaryArea = SNew(SJavascriptBox).Widget(this);

	Setup(PrimaryArea);

	return PrimaryArea;
}

void UJavascriptMultiBox::AddPullDownMenu(FJavascriptMenuBuilder& Builder, FName Id, const FText& Label, const FText& ToolTip)
{
	if (IsValidBuilder(Builder.MenuBar))
	{
		Builder.MenuBar->AddPullDownMenu(
			Label,
			ToolTip,
			FNewMenuDelegate::CreateLambda([this, Id](FMenuBuilder& MenuBuilder) {
				FJavascriptMenuBuilder Builder;
				Builder.MultiBox = Builder.Menu = TakeBuilder(new FMenuBuilder(MenuBuilder));
				this->OnHook.Execute(Id,this,Builder);
#if !PLATFORM_MAC				
				MenuBuilder = *Builder.Menu.Get();
#endif
			})
		);
	}
}

void UJavascriptMultiBox::AddSubMenu(FJavascriptMenuBuilder& Builder, FName Id, const FText& Label, const FText& ToolTip, const bool bInOpenSubMenuOnClick)
{
	if (IsValidBuilder(Builder.Menu))
	{
		Builder.Menu->AddSubMenu(
			Label,
			ToolTip,
			FNewMenuDelegate::CreateLambda([this, Id](FMenuBuilder& MenuBuilder) {
				FJavascriptMenuBuilder Builder;
				Builder.MultiBox = Builder.Menu = TakeBuilder(new FMenuBuilder(MenuBuilder));
				this->OnHook.Execute(Id, this, Builder);
#if !PLATFORM_MAC
				MenuBuilder = *Builder.Menu.Get();
#endif
			}),
			bInOpenSubMenuOnClick,
			FSlateIcon()
		);
	}
}
