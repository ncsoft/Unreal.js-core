PRAGMA_DISABLE_SHADOW_VARIABLE_WARNINGS

#include "JavascriptMultiBox.h"
#include "JavascriptUMG/SJavascriptBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSpacer.h"

static SBox* Target;
void UJavascriptMultiBox::Setup(TSharedRef<SBox> Box)
{	
	Box->SetContent(SNew(SSpacer));

	Target = &(Box.Get());
	OnHook.Execute(FName("Main"),this,FJavascriptMenuBuilder());
	Target = nullptr;
}

void UJavascriptMultiBox::Bind(FJavascriptMenuBuilder Builder)
{
	if (Builder.MultiBox)
	{
		Target->SetContent(Builder.MultiBox->MakeWidget());
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
	if (Builder.MenuBar)
	{
		Builder.MenuBar->AddPullDownMenu(
			Label,
			ToolTip,
			FNewMenuDelegate::CreateLambda([this, Id](FMenuBuilder& MenuBuilder) {
				FJavascriptMenuBuilder Builder;
				Builder.MultiBox = Builder.Menu = &MenuBuilder;
				this->OnHook.Execute(Id,this,Builder);
			})
		);
	}
}

void UJavascriptMultiBox::AddSubMenu(FJavascriptMenuBuilder& Builder, FName Id, const FText& Label, const FText& ToolTip, const bool bInOpenSubMenuOnClick)
{
	if (Builder.Menu)
	{
		Builder.Menu->AddSubMenu(
			Label,
			ToolTip,
			FNewMenuDelegate::CreateLambda([this, Id](FMenuBuilder& MenuBuilder) {
				FJavascriptMenuBuilder Builder;
				Builder.MultiBox = Builder.Menu = &MenuBuilder;
				this->OnHook.Execute(Id, this, Builder);
			}),
			bInOpenSubMenuOnClick,
			FSlateIcon()
		);
	}
}

PRAGMA_ENABLE_SHADOW_VARIABLE_WARNINGS