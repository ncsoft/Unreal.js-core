#include "JavascriptConsoleModule.h"

#include "IV8.h"
#include "Framework/Application/SlateApplication.h"

IMPLEMENT_MODULE( FJavascriptConsoleModule, JavascriptConsole );

FJavascriptConsoleModule::FJavascriptConsoleModule()
	: bModuleIsRunning(false)
{
}

void FJavascriptConsoleModule::StartupModule()
{
	const auto EditorExecutor = MakeShared<FJavascriptCommandExecutor>();
	CommandExecutors.Add(EditorExecutor->GetTargetContext(), EditorExecutor);
	
	IModularFeatures::Get().RegisterModularFeature(IConsoleCommandExecutor::ModularFeatureName(), &EditorExecutor.Get());
	
	bModuleIsRunning = true;
}

void FJavascriptConsoleModule::ShutdownModule()
{
	bModuleIsRunning = false;

	for (auto [ContextId, CmdExec] : CommandExecutors)
	{
		IModularFeatures::Get().UnregisterModularFeature(IConsoleCommandExecutor::ModularFeatureName(), CmdExec.Get());
	}
}

void FJavascriptConsoleModule::Tick(float DeltaTime)
{
	TArray<TSharedPtr<FString>> ContextArray;
	IV8::Get().GetContextIds(ContextArray);

	if (ContextArray.Num() == CommandExecutors.Num())
	{
		bool bEquals = true;

		for (auto ContextId : ContextArray)
		{
			if (!CommandExecutors.Contains(ContextId))
			{
				bEquals = false;
				break;
			}
		}

		if (bEquals)
		{
			return;
		}
	}

	auto OldExecutors = CommandExecutors;
	CommandExecutors.Empty();

	for (auto ContextId : ContextArray)
	{
		const auto Executor = OldExecutors.Find(ContextId);
		if (Executor == nullptr)
		{
			const auto NewExecutor = MakeShared<FJavascriptCommandExecutor>(ContextId);
			CommandExecutors.Add(NewExecutor->GetTargetContext(), NewExecutor);
			IModularFeatures::Get().RegisterModularFeature(IConsoleCommandExecutor::ModularFeatureName(), &NewExecutor.Get());
		}
		else
		{
			CommandExecutors.Add(ContextId, *Executor);
			OldExecutors.Remove(ContextId);
		}
	}

	for (auto [ContextId, Executor] : OldExecutors)
	{
		IModularFeatures::Get().UnregisterModularFeature(IConsoleCommandExecutor::ModularFeatureName(), Executor.Get());
	}
}

TStatId FJavascriptConsoleModule::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(FJavascriptConsoleModule, STATGROUP_Tickables);
}

bool FJavascriptConsoleModule::IsAllowedToTick() const
{
	return bModuleIsRunning;
}