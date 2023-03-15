#include "JavascriptMenuLibrary.h"
#include "JavascriptUMG/SJavascriptBox.h"
#include "JavascriptUMG/JavascriptToolbarButtonContext.h"
#include "Components/Widget.h"
#include "Framework/Commands/GenericCommands.h"
#include "JavascriptUICommands.h"
#include "../../Launch/Resources/Version.h"
#include "ToolMenu.h"

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
	FToolBarBuilder Builder(CommandList.Handle, FMultiBoxCustomization::None, nullptr);
	FVerticalToolBarBuilder VTBuilder(CommandList.Handle, FMultiBoxCustomization::None, nullptr);
	Out.MultiBox = Out.ToolBar = (Orientation == Orient_Horizontal) ? &Builder : &VTBuilder;
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

void UJavascriptMenuLibrary::BeginSection(FJavascriptMenuBuilder& Builder, FName InExtensionHook, FText MenuHeadingText)
{
	if (Builder.ToolBar)
	{
		Builder.ToolBar->BeginSection(InExtensionHook);
	}
	else if (Builder.Menu)
	{
		Builder.Menu->BeginSection(InExtensionHook, MenuHeadingText);
	}
	else if (Builder.ToolMenu)
	{
		Builder.ToolMenu->AddSection(InExtensionHook, MenuHeadingText);
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
	else if (Builder.ToolMenu)
	{
		FToolMenuSection& Section = Builder.ToolMenu->Sections.Num() > 0 ? Builder.ToolMenu->Sections.Top() : Builder.ToolMenu->AddSection(FName());
#if ENGINE_MAJOR_VERSION > 4 
		Section.AddSeparator(FName());
#else
		Section.AddMenuSeparator(FName());
#endif
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
	else if (Builder.ToolMenu && CommandInfo.Handle.IsValid())
	{
		FToolMenuSection& Section = Builder.ToolMenu->Sections.Num() > 0 ? Builder.ToolMenu->Sections.Top() : Builder.ToolMenu->AddSection(FName());
		Section.AddMenuEntry(CommandInfo.Handle);
	}
}

void UJavascriptMenuLibrary::AddToolBarButtonByContext(FJavascriptMenuBuilder& Builder, UJavascriptToolbarButtonContext* Context, UObject* EditingObject)
{
	if (Builder.ToolBar)
	{
		FUIAction DefaultAction;
		DefaultAction.ExecuteAction = FExecuteAction::CreateUObject(Context, &UJavascriptToolbarButtonContext::Public_OnExecuteAction, EditingObject);
		DefaultAction.CanExecuteAction = FCanExecuteAction::CreateUObject(Context, &UJavascriptToolbarButtonContext::Public_OnCanExecuteAction, EditingObject);
		DefaultAction.IsActionVisibleDelegate = FCanExecuteAction::CreateUObject(Context, &UJavascriptToolbarButtonContext::Public_OnIsActionButtonVisible, EditingObject);
		Builder.ToolBar->AddToolBarButton(
			DefaultAction,
			NAME_None,
			TAttribute< FText >::Create(TAttribute< FText >::FGetter::CreateUObject(Context, &UJavascriptToolbarButtonContext::Public_OnGetLabel)),
			TAttribute< FText >::Create(TAttribute< FText >::FGetter::CreateUObject(Context, &UJavascriptToolbarButtonContext::Public_OnGetTooltip)),
			TAttribute< FSlateIcon >::Create(TAttribute< FSlateIcon >::FGetter::CreateUObject(Context, &UJavascriptToolbarButtonContext::Public_OnGetSlateIcon))
		);
	}
}

void UJavascriptMenuLibrary::AddComboButton(FJavascriptMenuBuilder& Builder, UJavascriptComboButtonContext* Object, UObject* EditingObject)
{
	if (Builder.ToolBar)
	{
		FUIAction DefaultAction;
		DefaultAction.CanExecuteAction = FCanExecuteAction::CreateUObject(Object, &UJavascriptComboButtonContext::Public_CanExecute);
		Builder.ToolBar->AddComboButton(
			DefaultAction,
			FOnGetContent::CreateUObject(Object, &UJavascriptComboButtonContext::Public_OnGetWidget, EditingObject),
			TAttribute< FText >::Create(TAttribute< FText >::FGetter::CreateUObject(Object, &UJavascriptComboButtonContext::Public_OnGetLabel)),
			TAttribute< FText >::Create(TAttribute< FText >::FGetter::CreateUObject(Object, &UJavascriptComboButtonContext::Public_OnGetTooltip)),
			TAttribute< FSlateIcon >::Create(TAttribute< FSlateIcon >::FGetter::CreateUObject(Object, &UJavascriptComboButtonContext::Public_OnGetSlateIcon))
		);
	}
}

void UJavascriptMenuLibrary::AddMenuEntry(FJavascriptMenuBuilder& Builder, UJavascriptMenuContext* Object)
{
	if (Builder.Menu)
	{
		FUIAction DefaultAction;
		DefaultAction.CanExecuteAction = FCanExecuteAction::CreateUObject(Object, &UJavascriptMenuContext::Public_CanExecute);
		DefaultAction.ExecuteAction = FExecuteAction::CreateUObject(Object, &UJavascriptMenuContext::Public_Execute);
		DefaultAction.GetActionCheckState = FGetActionCheckState::CreateUObject(Object, &UJavascriptMenuContext::Public_GetActionCheckState);
#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION > 22) || ENGINE_MAJOR_VERSION > 4
		const EUserInterfaceActionType CommandType = EUserInterfaceActionType(Object->ActionType.GetValue());
#else
		const EUserInterfaceActionType::Type CommandType = EUserInterfaceActionType::Type(Object->ActionType.GetValue());
#endif
		Builder.Menu->AddMenuEntry(
			Object->Description,
			Object->ToolTip,
			Object->Icon,
			DefaultAction,
			NAME_None,
			CommandType
		);
	}
	else if (Builder.ToolMenu)
	{
		FUIAction DefaultAction;
		DefaultAction.CanExecuteAction = FCanExecuteAction::CreateUObject(Object, &UJavascriptMenuContext::Public_CanExecute);
		DefaultAction.ExecuteAction = FExecuteAction::CreateUObject(Object, &UJavascriptMenuContext::Public_Execute);
		DefaultAction.GetActionCheckState = FGetActionCheckState::CreateUObject(Object, &UJavascriptMenuContext::Public_GetActionCheckState);
#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION > 22) || ENGINE_MAJOR_VERSION > 4
		const EUserInterfaceActionType CommandType = EUserInterfaceActionType(Object->ActionType.GetValue());
#else
		const EUserInterfaceActionType::Type CommandType = EUserInterfaceActionType::Type(Object->ActionType.GetValue());
#endif
		FToolMenuSection& Section = Builder.ToolMenu->Sections.Num() > 0 ? Builder.ToolMenu->Sections.Top() : Builder.ToolMenu->AddSection(FName());
		Section.AddMenuEntry(
			*Object->Description.ToString(),
			Object->Description,
			Object->ToolTip,
			Object->Icon,
			DefaultAction,
			CommandType
		);
	}
}

void UJavascriptMenuLibrary::AddSubMenu(FJavascriptMenuBuilder& Builder, const FText& Label, const FText& ToolTip, const bool bInOpenSubMenuOnClick, FJavascriptFunction Function)
{
	if (Builder.Menu)
	{
		TSharedPtr<FJavascriptFunction> Copy(new FJavascriptFunction);
		*(Copy.Get()) = Function;
		Builder.Menu->AddSubMenu(
			Label,
			ToolTip,
			FNewMenuDelegate::CreateLambda([=](FMenuBuilder& SubMenuBuilder) {
				FJavascriptMenuBuilder Out;
				Out.MultiBox = Out.Menu = &SubMenuBuilder;
				Copy->Execute(FJavascriptMenuBuilder::StaticStruct(), &Out);
			}),
			bInOpenSubMenuOnClick,
			FSlateIcon()
		);
	}
	else if (Builder.ToolMenu)
	{
		TSharedPtr<FJavascriptFunction> Copy(new FJavascriptFunction);
		*(Copy.Get()) = Function;

		FNewToolMenuChoice NewToolMenuChoice(
			FNewMenuDelegate::CreateLambda([=](FMenuBuilder& SubMenuBuilder) {
				FJavascriptMenuBuilder Out;
				Out.MultiBox = Out.Menu = &SubMenuBuilder;
				Copy->Execute(FJavascriptMenuBuilder::StaticStruct(), &Out);
			})
		);
		FToolMenuSection& Section = Builder.ToolMenu->Sections.Num() > 0 ? Builder.ToolMenu->Sections.Top() : Builder.ToolMenu->AddSection(FName());
		Section.AddSubMenu(
			*Label.ToString(),
			Label,
			ToolTip,
			NewToolMenuChoice,
			bInOpenSubMenuOnClick,
			FSlateIcon()
		);
	}
}

void UJavascriptMenuLibrary::AddMenuByCommands(FJavascriptMenuBuilder& Builder, UJavascriptUICommands* UICommands)
{
	if (Builder.Menu && UICommands)
	{
		for (FJavascriptUICommandInfo CommandInfo : UICommands->CommandInfos)
		{
			Builder.Menu->AddMenuEntry(CommandInfo.Handle);
		}
	}
	else if (Builder.ToolMenu && UICommands)
	{
		for (FJavascriptUICommandInfo CommandInfo : UICommands->CommandInfos)
		{
			FToolMenuSection& Section = Builder.ToolMenu->Sections.Num() > 0 ? Builder.ToolMenu->Sections.Top() : Builder.ToolMenu->AddSection(FName());
			Section.AddMenuEntry(CommandInfo.Handle);
		}
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

FJavascriptUICommandInfo UJavascriptMenuLibrary::UI_COMMAND_Function(FJavascriptBindingContext This, FJavascriptUICommand info, const FString& InTextSubNamespace)
{
	FJavascriptUICommandInfo Out;

	if (info.FriendlyName.Len() == 0)
	{
		info.FriendlyName = info.Id;
	}
	//////////////////////////////////////////////////////////////////////////
	// @NOTE: Commands/Commands.cpp <UI_COMMAND_Function>
	FBindingContext* ThisBindingContext = This.Handle.Get();
	TSharedPtr< FUICommandInfo >& OutCommand = Out.Handle;
	const TCHAR* OutSubNamespace = *InTextSubNamespace;
	const TCHAR* OutCommandName = *info.Id;
	const FString OutCommandNameUnderscoreTooltip = FString::Printf(TEXT("%s_Tooltip"), *info.Id);
	const FString DotOutCommandName = FString::Printf(TEXT(".%s"), *info.Id);
	const TCHAR* FriendlyName = *info.FriendlyName;
	const TCHAR* InDescription = *info.Description;
#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION > 22) || ENGINE_MAJOR_VERSION > 4
	const EUserInterfaceActionType CommandType = EUserInterfaceActionType(info.ActionType.GetValue());
#else
	const EUserInterfaceActionType::Type CommandType = EUserInterfaceActionType::Type(info.ActionType.GetValue());
#endif
	const FInputChord& InDefaultChord = info.DefaultChord;
	const FInputChord& InAlternateDefaultChord = FInputChord();
	const FString IconStyleName = *info.IconStyleName;

	static const FString UICommandsStr(TEXT("UICommands"));
	const FString Namespace = OutSubNamespace && FCString::Strlen(OutSubNamespace) > 0 ? UICommandsStr + TEXT(".") + OutSubNamespace : UICommandsStr;

	FString OrignContextName, ContextIdx;
	ThisBindingContext->GetContextName().ToString().Split("@", &OrignContextName, &ContextIdx);

	FUICommandInfo::MakeCommandInfo(
		ThisBindingContext->AsShared(),
		OutCommand,
		OutCommandName,
		FInternationalization::ForUseOnlyByLocMacroAndGraphNodeTextLiterals_CreateText(FriendlyName, *Namespace, OutCommandName),
		FInternationalization::ForUseOnlyByLocMacroAndGraphNodeTextLiterals_CreateText(InDescription, *Namespace, *OutCommandNameUnderscoreTooltip),
		FSlateIcon(ThisBindingContext->GetStyleSetName(), IconStyleName.IsEmpty() ? ISlateStyle::Join(FName(*OrignContextName), TCHAR_TO_ANSI(*DotOutCommandName)) : FName(*IconStyleName)),
		CommandType,
		InDefaultChord,
		InAlternateDefaultChord
	);
	//////////////////////////////////////////////////////////////////////////
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
	return FJavascriptExtender(FExtender::Combine(_Extenders));
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

FJavascriptUICommandInfo UJavascriptMenuLibrary::GenericCommand(FString What)
{
	auto Commands = FGenericCommands::Get();
	FJavascriptUICommandInfo Out;

#define OP(x) if (What == TEXT(#x)) { Out.Handle = Commands.x; }
	OP(Cut);
	OP(Copy);
	OP(Paste);
	OP(Duplicate);
	OP(Undo);
	OP(Redo);
	OP(Delete);
	OP(Rename);
	OP(SelectAll);

#undef OP
	return Out;
}
