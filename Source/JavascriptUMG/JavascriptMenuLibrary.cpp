#include "JavascriptUMG.h"
#include "JavascriptMenuLibrary.h"
#include "SJavascriptBox.h"

FJavascriptUICommandList UJavascriptMenuLibrary::CreateUICommandList()
{
	FJavascriptUICommandList Out;
	Out.Handle = MakeShareable(new FUICommandList);
	return Out;
}

bool UJavascriptMenuLibrary::ProcessCommandBindings_KeyEvent(FJavascriptUICommandList CommandList, const FKeyEvent& InKeyEvent)
{
	return CommandList.Handle->ProcessCommandBindings(InKeyEvent);
}

bool UJavascriptMenuLibrary::ProcessCommandBindings_PointerEvent(FJavascriptUICommandList CommandList, const FPointerEvent& InMouseEvent)
{
	return CommandList.Handle->ProcessCommandBindings(InMouseEvent);
}

void UJavascriptMenuLibrary::CreateToolbarBuilder(FJavascriptUICommandList CommandList, EOrientation Orientation, FJavascriptFunction Function)
{
	FJavascriptMenuBuilder Out;
	FToolBarBuilder Builder(CommandList.Handle, FMultiBoxCustomization::None, nullptr, Orientation);
	Out.MultiBox = Out.ToolBar = &Builder;
	Function.Execute(FJavascriptMenuBuilder::StaticStruct(), &Out);
}

void UJavascriptMenuLibrary::CreateMenuBuilder(FJavascriptUICommandList CommandList, bool bInShouldCloseWindowAfterMenuSelection, FJavascriptFunction Function)
{
	FJavascriptMenuBuilder Out;
	FMenuBuilder Builder(bInShouldCloseWindowAfterMenuSelection, CommandList.Handle);
	Out.MultiBox = Out.Menu = &Builder;
	Function.Execute(FJavascriptMenuBuilder::StaticStruct(), &Out);
}

void UJavascriptMenuLibrary::CreateMenuBarBuilder(FJavascriptUICommandList CommandList, FJavascriptFunction Function)
{
	FJavascriptMenuBuilder Out;
	FMenuBarBuilder Builder(CommandList.Handle);
	Out.MultiBox = Out.MenuBar = &Builder;
	Function.Execute(FJavascriptMenuBuilder::StaticStruct(), &Out);
}

void UJavascriptMenuLibrary::BeginSection(FJavascriptMenuBuilder& Builder, FName InExtensionHook)
{
	if (Builder.ToolBar)
	{
		Builder.ToolBar->BeginSection(InExtensionHook);
	}
	else if (Builder.Menu)
	{
		Builder.Menu->BeginSection(InExtensionHook);
	}
}

void UJavascriptMenuLibrary::EndSection(FJavascriptMenuBuilder& Builder)
{
	if (Builder.ToolBar)
	{
		Builder.ToolBar->EndSection();
	}
	else if (Builder.Menu)
	{
		Builder.Menu->EndSection();
	}
}

void UJavascriptMenuLibrary::AddSeparator(FJavascriptMenuBuilder& Builder)
{
	if (Builder.ToolBar)
	{
		Builder.ToolBar->AddSeparator();
	}
	else if (Builder.Menu)
	{
		Builder.Menu->AddMenuSeparator();
	}
}

void UJavascriptMenuLibrary::AddToolBarButton(FJavascriptMenuBuilder& Builder, FJavascriptUICommandInfo CommandInfo)
{
	if (Builder.ToolBar)
	{
		Builder.ToolBar->AddToolBarButton(CommandInfo.Handle);
	}
	else if (Builder.Menu)
	{
		Builder.Menu->AddMenuEntry(CommandInfo.Handle);
	}
}

void UJavascriptMenuLibrary::AddWidget(FJavascriptMenuBuilder& Builder, UWidget* Widget, const FText& Label, bool bNoIndent, FName InTutorialHighlightName, bool bSearchable)
{
	if (Builder.ToolBar)
	{
		Builder.ToolBar->AddWidget(
			SNew(SJavascriptBox).Widget(Widget)[Widget->TakeWidget()],
			InTutorialHighlightName,
			bSearchable
			);
	}
	else if (Builder.Menu)
	{
		Builder.Menu->AddWidget(
			SNew(SJavascriptBox).Widget(Widget)[Widget->TakeWidget()],
			Label,
			bNoIndent,
			bSearchable
			);
	}
	
}

void UJavascriptMenuLibrary::PushCommandList(FJavascriptMenuBuilder& Builder, FJavascriptUICommandList List)
{
	if (Builder.MultiBox)
	{
		Builder.MultiBox->PushCommandList(List.Handle.ToSharedRef());
	}
}

void UJavascriptMenuLibrary::PopCommandList(FJavascriptMenuBuilder& Builder)
{
	if (Builder.MultiBox)
	{
		Builder.MultiBox->PopCommandList();
	}
}

FJavascriptBindingContext UJavascriptMenuLibrary::NewBindingContext(const FName InContextName, const FText& InContextDesc, const FName InContextParent, const FName InStyleSetName)
{
	FJavascriptBindingContext Out;
	Out.Handle = MakeShareable(new FBindingContext(InContextName, InContextDesc, InContextParent, InStyleSetName));
	return Out;
}

void UJavascriptMenuLibrary::Destroy(FJavascriptBindingContext Context)
{
	Context.Destroy();
}

FJavascriptUICommandInfo UJavascriptMenuLibrary::UI_COMMAND_Function(FJavascriptBindingContext This, FJavascriptUICommand info)
{
	FJavascriptUICommandInfo Out;

	if (info.FriendlyName.Len() == 0)
	{
		info.FriendlyName = info.Id;
	}

	::UI_COMMAND_Function(
		This.Handle.Get(),
		Out.Handle,
		TEXT(""),
		*info.Id,
		*FString::Printf(TEXT("%s_Tooltip"), *info.Id),
		TCHAR_TO_ANSI(*FString::Printf(TEXT(".%s"), *info.Id)),
		*info.FriendlyName,
		*info.Description,
		EUserInterfaceActionType::Type(info.ActionType.GetValue()),
		info.DefaultChord);

	return Out;
}

FJavascriptExtensionBase UJavascriptMenuLibrary::AddToolBarExtension(FJavascriptExtender Extender, FName ExtensionHook, EJavascriptExtensionHook::Type HookPosition, FJavascriptUICommandList CommandList, FJavascriptFunction Function)
{
	TSharedPtr<FJavascriptFunction> Copy(new FJavascriptFunction);
	*(Copy.Get()) = Function;
	return { Extender->AddToolBarExtension(ExtensionHook, (EExtensionHook::Position)HookPosition, CommandList.Handle, FToolBarExtensionDelegate::CreateLambda([=](class FToolBarBuilder& Builder) {
		FJavascriptMenuBuilder Out;
		Out.MultiBox = Out.ToolBar = &Builder;
		Copy->Execute(FJavascriptMenuBuilder::StaticStruct(), &Out);
	}))};
}
FJavascriptExtensionBase UJavascriptMenuLibrary::AddMenuExtension(FJavascriptExtender Extender, FName ExtensionHook, EJavascriptExtensionHook::Type HookPosition, FJavascriptUICommandList CommandList, FJavascriptFunction Function)
{
	TSharedPtr<FJavascriptFunction> Copy(new FJavascriptFunction);
	*(Copy.Get()) = Function;
	return{ Extender->AddMenuExtension(ExtensionHook, (EExtensionHook::Position)HookPosition, CommandList.Handle, FMenuExtensionDelegate::CreateLambda([=](class FMenuBuilder& Builder) {
		FJavascriptMenuBuilder Out;
		Out.MultiBox = Out.Menu = &Builder;
		Copy->Execute(FJavascriptMenuBuilder::StaticStruct(), &Out);
	})) };
}
FJavascriptExtensionBase UJavascriptMenuLibrary::AddMenubarExtension(FJavascriptExtender Extender, FName ExtensionHook, EJavascriptExtensionHook::Type HookPosition, FJavascriptUICommandList CommandList, FJavascriptFunction Function)
{
	TSharedPtr<FJavascriptFunction> Copy(new FJavascriptFunction);
	*(Copy.Get()) = Function;
	return{ Extender->AddMenuBarExtension(ExtensionHook, (EExtensionHook::Position)HookPosition, CommandList.Handle, FMenuBarExtensionDelegate::CreateLambda([=](class FMenuBarBuilder& Builder) {
		FJavascriptMenuBuilder Out;
		Out.MultiBox = Out.MenuBar = &Builder;
		Copy->Execute(FJavascriptMenuBuilder::StaticStruct(), &Out);
	})) };
}

void UJavascriptMenuLibrary::RemoveExtension(FJavascriptExtender Extender, FJavascriptExtensionBase Extension)
{
	Extender->RemoveExtension(Extension.Handle.ToSharedRef());
}

void UJavascriptMenuLibrary::Apply(FJavascriptExtender Extender, FName ExtensionHook, EJavascriptExtensionHook::Type HookPosition, FJavascriptMenuBuilder& MenuBuilder)
{
	if (MenuBuilder.ToolBar) 
	{
		Extender->Apply(ExtensionHook, (EExtensionHook::Position)HookPosition, *MenuBuilder.ToolBar);
	}
	else if (MenuBuilder.Menu)
	{
		Extender->Apply(ExtensionHook, (EExtensionHook::Position)HookPosition, *MenuBuilder.Menu);
	}
	else if (MenuBuilder.MenuBar)
	{
		Extender->Apply(ExtensionHook, (EExtensionHook::Position)HookPosition, *MenuBuilder.MenuBar);
	}	
}

FJavascriptExtender UJavascriptMenuLibrary::Combine(const TArray<FJavascriptExtender>& Extenders)
{
	TArray<TSharedPtr<FExtender>> _Extenders;
	for (auto Extender : Extenders)
	{
		_Extenders.Add(Extender.Handle);
	}
	return{ FExtender::Combine(_Extenders) };
}

FJavascriptExtender::FJavascriptExtender()
	: Handle(new FExtender)
{}

FJavascriptExtender::FJavascriptExtender(TSharedPtr<FExtender> Extender)
	: Handle(Extender)
{}

void UJavascriptMenuLibrary::AddPullDownMenu(FJavascriptMenuBuilder& MenuBuilder, const FText& InMenuLabel, const FText& InToolTip, FJavascriptFunction InPullDownMenu, FName InExtensionHook, FName InTutorialHighlightName)
{
	if (MenuBuilder.MenuBar)
	{
		TSharedPtr<FJavascriptFunction> Copy(new FJavascriptFunction);
		*(Copy.Get()) = InPullDownMenu;
		auto Delegate = FNewMenuDelegate::CreateLambda([=](class FMenuBuilder& Builder) {
			FJavascriptMenuBuilder Out;
			Out.MultiBox = Out.Menu = &Builder;
			Copy->Execute(FJavascriptMenuBuilder::StaticStruct(), &Out);			
		});
		MenuBuilder.MenuBar->AddPullDownMenu(InMenuLabel, InToolTip, Delegate, InExtensionHook, InTutorialHighlightName);
	}
}