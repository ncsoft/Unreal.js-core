#include "JavascriptUMG.h"
#include "JavascriptMultiBox.h"
#include "SJavascriptBox.h"

void UJavascriptMultiBox::Setup(TSharedRef<SBox> Box)
{	
	auto Builder = OnHook.Execute(FName("Main"),this,FJavascriptMenuBuilder());

	if (Builder.MultiBox.IsValid())
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
	if (Builder.MenuBar.IsValid())
	{
		Builder.MenuBar->AddPullDownMenu(
			Label,
			ToolTip,
			FNewMenuDelegate::CreateLambda([this, Id](FMenuBuilder& MenuBuilder) {
				FJavascriptMenuBuilder Builder;
				Builder.MultiBox = Builder.Menu = MakeShareable(new FMenuBuilder(MenuBuilder));
				this->OnHook.Execute(Id,this,Builder);
				MenuBuilder = *Builder.Menu.Get();
			})
		);
	}
}

void UJavascriptMultiBox::AddSubMenu(FJavascriptMenuBuilder& Builder, FName Id, const FText& Label, const FText& ToolTip, const bool bInOpenSubMenuOnClick)
{
	if (Builder.Menu.IsValid())
	{
		Builder.Menu->AddSubMenu(
			Label,
			ToolTip,
			FNewMenuDelegate::CreateLambda([this, Id](FMenuBuilder& MenuBuilder) {
				FJavascriptMenuBuilder Builder;
				Builder.MultiBox = Builder.Menu = MakeShareable(new FMenuBuilder(MenuBuilder));
				this->OnHook.Execute(Id, this, Builder);
				MenuBuilder = *Builder.Menu.Get();
			}),
			bInOpenSubMenuOnClick,
			FSlateIcon()
		);
	}
}
