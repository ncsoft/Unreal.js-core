#include "JavascriptEditor.h"
#include "JavascriptEditorMenu.h"
#include "JavascriptUIExtender.h"

UJavascriptEditorMenu::UJavascriptEditorMenu(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{	
}

#if WITH_EDITOR	
namespace 
{
	static FMenuBarBuilder* CurrentMenuBarBuilder{ nullptr };
	static UJavascriptEditorMenu* CurrentEditorMenu{ nullptr };
	static TSharedPtr<const FUICommandList> CurrentCommandList;
}
void UJavascriptEditorMenu::Setup(TSharedRef<SBox> Box)
{	
	CurrentCommandList = CommandList.Handle;

	FMenuBarBuilder MenuBarBuilder = FMenuBarBuilder(CommandList.Handle);
	CurrentMenuBarBuilder = &MenuBarBuilder;
	CurrentEditorMenu = this;
	OnHook.ExecuteIfBound("Menubar");	
	CurrentMenuBarBuilder = nullptr;
	CurrentEditorMenu = nullptr;
	CurrentCommandList.Reset();

	Box->SetContent(MenuBarBuilder.MakeWidget());
}

void UJavascriptEditorMenu::AddPullDownMenu(const FName& Id, const FText& MenuLabel, const FText& Tooltip)
{
	auto Self = CurrentEditorMenu;
	auto CommandList = CurrentCommandList.ToSharedRef();
	CurrentMenuBarBuilder->AddPullDownMenu(
		MenuLabel,
		Tooltip,
		FNewMenuDelegate::CreateLambda([Self, Id, CommandList](FMenuBuilder& MenuBuilder) {
			MenuBuilder.PushCommandList(CommandList);
			UJavascriptUIExtender::PushMenuBuilder(MenuBuilder);
			Self->OnHook.ExecuteIfBound(Id);
			UJavascriptUIExtender::Reset();
		}),
		Id
	);
}

TSharedRef<SWidget> UJavascriptEditorMenu::RebuildWidget()
{	
	auto PrimaryArea = SNew(SBox);			

	Setup(PrimaryArea);

	return PrimaryArea;
}

#endif
