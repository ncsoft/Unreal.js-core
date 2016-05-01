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

FJavascriptMenuBuilder UJavascriptMenuLibrary::CreateToolbarBuilder(FJavascriptUICommandList CommandList, EOrientation Orientation)
{
	FJavascriptMenuBuilder Out;
	Out.MultiBox = Out.ToolBar = TakeBuilder(new FToolBarBuilder(CommandList.Handle, FMultiBoxCustomization::None, nullptr, Orientation));
	return Out;
}

FJavascriptMenuBuilder UJavascriptMenuLibrary::CreateMenuBuilder(FJavascriptUICommandList CommandList, bool bInShouldCloseWindowAfterMenuSelection)
{
	FJavascriptMenuBuilder Out;
	Out.MultiBox = Out.Menu = TakeBuilder(new FMenuBuilder(bInShouldCloseWindowAfterMenuSelection, CommandList.Handle));
	return Out;
}

FJavascriptMenuBuilder UJavascriptMenuLibrary::CreateMenuBarBuilder(FJavascriptUICommandList CommandList)
{
	FJavascriptMenuBuilder Out;
	Out.MultiBox = Out.MenuBar = TakeBuilder(new FMenuBarBuilder(CommandList.Handle));
	return Out;
}

void UJavascriptMenuLibrary::BeginSection(FJavascriptMenuBuilder& Builder, FName InExtensionHook)
{
	if (IsValidBuilder(Builder.ToolBar))
	{
		Builder.ToolBar->BeginSection(InExtensionHook);
	}
	else if (IsValidBuilder(Builder.Menu))
	{
		Builder.Menu->BeginSection(InExtensionHook);
	}
}

void UJavascriptMenuLibrary::EndSection(FJavascriptMenuBuilder& Builder)
{
	if (IsValidBuilder(Builder.ToolBar))
	{
		Builder.ToolBar->EndSection();
	}
	else if (IsValidBuilder(Builder.Menu))
	{
		Builder.Menu->EndSection();
	}
}

void UJavascriptMenuLibrary::AddSeparator(FJavascriptMenuBuilder& Builder)
{
	if (IsValidBuilder(Builder.ToolBar))
	{
		Builder.ToolBar->AddSeparator();
	}
	else if (IsValidBuilder(Builder.Menu))
	{
		Builder.Menu->AddMenuSeparator();
	}
}

void UJavascriptMenuLibrary::AddToolBarButton(FJavascriptMenuBuilder& Builder, FJavascriptUICommandInfo CommandInfo)
{
	if (IsValidBuilder(Builder.ToolBar))
	{
		Builder.ToolBar->AddToolBarButton(CommandInfo.Handle);
	}
	else if (IsValidBuilder(Builder.Menu))
	{
		Builder.Menu->AddMenuEntry(CommandInfo.Handle);
	}
}

void UJavascriptMenuLibrary::AddWidget(FJavascriptMenuBuilder& Builder, UWidget* Widget, const FText& Label, bool bNoIndent, FName InTutorialHighlightName, bool bSearchable)
{
	if (IsValidBuilder(Builder.ToolBar))
	{
		Builder.ToolBar->AddWidget(
			SNew(SJavascriptBox).Widget(Widget)[Widget->TakeWidget()],
			InTutorialHighlightName,
			bSearchable
			);
	}
	else if (IsValidBuilder(Builder.Menu))
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
	if (IsValidBuilder(Builder.MultiBox))
	{
		Builder.MultiBox->PushCommandList(List.Handle.ToSharedRef());
	}
}

void UJavascriptMenuLibrary::PopCommandList(FJavascriptMenuBuilder& Builder)
{
	if (IsValidBuilder(Builder.MultiBox))
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