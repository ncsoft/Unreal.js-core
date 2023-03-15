//

#include "JavascriptCommandExecutor.h"

#include "IV8.h"
#include "JavascriptContext.h"
#include "JavascriptEditor/JavascriptEditorModule.h"
#include "Toolkits/GlobalEditorCommonCommands.h"

#define LOCTEXT_NAMESPACE "JavascriptCommandExecutor"

FJavascriptCommandExecutor::FJavascriptCommandExecutor(TSharedPtr<FString> InTargetContext)
	: TargetContext(InTargetContext)
{
	CacheJavascriptContext();
}

FName FJavascriptCommandExecutor::StaticName()
{
	static const FName CmdExecName = TEXT("Javascript");
	return CmdExecName;
}

FName FJavascriptCommandExecutor::GetName() const
{
	return CommandExecutorName;
}

FText FJavascriptCommandExecutor::GetDisplayName() const
{
	if (CommandExecutorName == StaticName())
	{
		return LOCTEXT("JavascriptCommandExecutorDisplayName", "Javascript");
	}

	return FText::Format(LOCTEXT("JavascriptCommandExecutorDisplayName", "Javascript ({0})"), FText::FromString(*TargetContext));
}

FText FJavascriptCommandExecutor::GetDescription() const
{
	return LOCTEXT("JavascriptCommandExecutorDescription", "Execute a single Javascript statement and show its result.");
}

FText FJavascriptCommandExecutor::GetHintText() const
{
	return LOCTEXT("JavascriptCommandExecutorHintText", "Enter Javascript statement.");
}

void FJavascriptCommandExecutor::GetAutoCompleteSuggestions(const TCHAR* Input, TArray<FString>& Out)
{
	if (!TargetContext.IsValid())
	{
		return;
	}

	IV8::Get().FillAutoCompletion(TargetContext, Out, Input);
}

void FJavascriptCommandExecutor::GetExecHistory(TArray<FString>& Out)
{
	IConsoleManager::Get().GetConsoleHistory(TEXT("Javascript"), Out);
}

bool FJavascriptCommandExecutor::Exec(const TCHAR* Input)
{
	if (!TargetContext.IsValid())
	{
		UE_LOG(LogJavascript, Error, TEXT("Failed to execute: No JavascriptContext for Editor.\n%s"), Input);
		return false;
	}

	IConsoleManager::Get().AddConsoleHistoryEntry(TEXT("Javascript"), Input);
	FEditorScriptExecutionGuard ScriptGuard;
	IV8::Get().Exec(TargetContext, Input);

	return true;
}

bool FJavascriptCommandExecutor::AllowHotKeyClose() const
{
	return true;
}

bool FJavascriptCommandExecutor::AllowMultiLine() const
{
	return true;
}

FInputChord FJavascriptCommandExecutor::GetHotKey() const
{
#if WITH_EDITOR
	return FGlobalEditorCommonCommands::Get().OpenConsoleCommandBox->GetActiveChord(EMultipleKeyBindingIndex::Primary).Get();
#else
	return FInputChord();
#endif
}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
FInputChord FJavascriptCommandExecutor::GetIterateExecutorHotKey() const
{
#if WITH_EDITOR
	return FGlobalEditorCommonCommands::Get().SelectNextConsoleExecutor->GetActiveChord(EMultipleKeyBindingIndex::Primary).Get();
#else
	return FInputChord();
#endif
}
#endif

bool FJavascriptCommandExecutor::CacheJavascriptContext()
{
	if (!TargetContext.IsValid())
	{
		// Use Editor context only 
		const auto JavascriptContext = IJavascriptEditorModule::Get().GetJavascriptContext();
		TargetContext = JavascriptContext->ContextId;
	}

	if (TargetContext.IsValid())
	{
		const auto EditorContext = IJavascriptEditorModule::Get().GetJavascriptContext();
		if (TargetContext == EditorContext->ContextId)
		{
			CommandExecutorName = StaticName();
		}
		else
		{
			CommandExecutorName = *FString::Printf(TEXT("Javascript%s"), **TargetContext.Get());
		}
	}

	return TargetContext.IsValid();
}

#undef LOCTEXT_NAMESPACE