#include "JavascriptUMG.h"
#include "JavascriptMenuLibrary.h"

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

FJavascriptMenuBuilder UJavascriptMenuLibrary::CreateToolbarBuilder(FJavascriptUICommandList CommandList)
{
	FJavascriptMenuBuilder Out;
	Out.MultiBox = Out.ToolBar = MakeShareable(new FToolBarBuilder(CommandList.Handle, FMultiBoxCustomization::None));
	return Out;
}

FJavascriptMenuBuilder UJavascriptMenuLibrary::CreateMenuBuilder(FJavascriptUICommandList CommandList, bool bInShouldCloseWindowAfterMenuSelection)
{
	FJavascriptMenuBuilder Out;
	Out.MultiBox = Out.Menu = MakeShareable(new FMenuBuilder(bInShouldCloseWindowAfterMenuSelection, CommandList.Handle));
	return Out;
}

FJavascriptMenuBuilder UJavascriptMenuLibrary::CreateMenuBarBuilder(FJavascriptUICommandList CommandList)
{
	FJavascriptMenuBuilder Out;
	Out.MultiBox = Out.MenuBar = MakeShareable(new FMenuBarBuilder(CommandList.Handle));
	return Out;
}

void UJavascriptMenuLibrary::BeginSection(FJavascriptMenuBuilder& Builder, FName InExtensionHook)
{
	if (Builder.ToolBar.IsValid())
	{
		Builder.ToolBar->BeginSection(InExtensionHook);
	}
	else if (Builder.Menu.IsValid())
	{
		Builder.Menu->BeginSection(InExtensionHook);
	}
}

void UJavascriptMenuLibrary::EndSection(FJavascriptMenuBuilder& Builder)
{
	if (Builder.ToolBar.IsValid())
	{
		Builder.ToolBar->EndSection();
	}
	else if (Builder.Menu.IsValid())
	{
		Builder.Menu->EndSection();
	}
}

void UJavascriptMenuLibrary::AddSeparator(FJavascriptMenuBuilder& Builder)
{
	if (Builder.ToolBar.IsValid())
	{
		Builder.ToolBar->AddSeparator();
	}
	else if (Builder.Menu.IsValid())
	{
		Builder.Menu->AddMenuSeparator();
	}
}

void UJavascriptMenuLibrary::AddToolBarButton(FJavascriptMenuBuilder& Builder, FJavascriptUICommandInfo CommandInfo)
{
	if (Builder.ToolBar.IsValid())
	{
		Builder.ToolBar->AddToolBarButton(CommandInfo.Handle);
	}
	else if (Builder.Menu.IsValid())
	{
		Builder.Menu->AddMenuEntry(CommandInfo.Handle);
	}
}

void UJavascriptMenuLibrary::PushCommandList(FJavascriptMenuBuilder& Builder, FJavascriptUICommandList List)
{
	if (Builder.MultiBox.IsValid())
	{
		Builder.MultiBox->PushCommandList(List.Handle.ToSharedRef());
	}
}

void UJavascriptMenuLibrary::PopCommandList(FJavascriptMenuBuilder& Builder)
{
	if (Builder.MultiBox.IsValid())
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