#include "V8PCH.h"
#include "JavascriptTestLibrary.h"
#include "Misc/AutomationTest.h"

#if WITH_EDITOR
struct FJavascriptAutomatedTestImpl : FAutomationTestBase, TSharedFromThis<FJavascriptAutomatedTestImpl>
{
	FJavascriptAutomatedTest Recipe;

	bool bContinue{ false };

	FJavascriptAutomatedTestImpl(const FJavascriptAutomatedTest& InRecipe)
		: Recipe(InRecipe), FAutomationTestBase(InRecipe.Name, InRecipe.bComplexTask)
	{}

	virtual uint32 GetTestFlags() const override
	{
		return Recipe.TestFlags;
	}

	virtual uint32 GetRequiredDeviceNum() const
	{
		return Recipe.RequiredDeviceNum;
	}

	virtual void GetTests(TArray<FString>& OutBeautifiedNames, TArray <FString>& OutTestCommands) const
	{
		for (const FString& TestName : Recipe.TestFunctionNames)
		{
			OutBeautifiedNames.Add(TestName);
			OutTestCommands.Add(TestName);
		}
	}

	virtual bool RunTest(const FString& Parameters)
	{
		FJavascriptAutomatedTestParameters Params;
		for (int32 i = 0; i < Recipe.TestFunctionNames.Num(); ++i)
		{
			if (Recipe.TestFunctionNames[i] == Parameters)
			{
				Params.TestFunctionName = Recipe.TestFunctionNames[i];
				break;
			}
		}
		if (Params.TestFunctionName == TEXT(""))
		{
			return false;
		}

		// run the matching test
		{
			Params.Tester.Handle = this->AsShared();

			for (;;)
			{
				bContinue = false;

				Recipe.Function.Execute(FJavascriptAutomatedTestParameters::StaticStruct(), &Params);

				if (!bContinue) break;
			}
		}		

		return true;
	}
	
	virtual FString GetBeautifiedTestName() const
	{
		return Recipe.Name;
	}
};

FJavascriptAutomatedTestInstance UJavascriptTestLibrary::Create(const FJavascriptAutomatedTest& Test)
{
	return{ MakeShareable(new FJavascriptAutomatedTestImpl(Test)) };
}

void UJavascriptTestLibrary::Destroy(FJavascriptAutomatedTestInstance& Test)
{
	Test.Handle.Reset();
}

void UJavascriptTestLibrary::ClearExecutionInfo(const FJavascriptAutomatedTestInstance& Test)
{
	if (Test.Handle.IsValid())
	{
		Test.Handle->ClearExecutionInfo();
	}
}

void UJavascriptTestLibrary::SetContinue(const FJavascriptAutomatedTestInstance& Test, bool bContinue)
{
	if (Test.Handle.IsValid())
	{
		Test.Handle->bContinue = bContinue;
	}
}

void UJavascriptTestLibrary::AddError(const FJavascriptAutomatedTestInstance& Test, const FString& InError)
{
	if (Test.Handle.IsValid())
	{
		Test.Handle->AddError(InError);
	}
}

void UJavascriptTestLibrary::AddWarning(const FJavascriptAutomatedTestInstance& Test, const FString& InWarning)
{
	if (Test.Handle.IsValid())
	{
		Test.Handle->AddWarning(InWarning);
	}
}

void UJavascriptTestLibrary::AddLogItem(const FJavascriptAutomatedTestInstance& Test, const FString& InLogItem)
{
	if (Test.Handle.IsValid())
	{
		Test.Handle->AddLogItem(InLogItem);
	}
}

void UJavascriptTestLibrary::AddAnalyticsItem(const FJavascriptAutomatedTestInstance& Test, const FString& InAnalyticsItem)
{
	if (Test.Handle.IsValid())
	{
		Test.Handle->AddAnalyticsItem(InAnalyticsItem);
	}
}

UWorld* UJavascriptTestLibrary::NewWorld()
{
	UWorld *World = UWorld::CreateWorld(EWorldType::Game, false);
	FWorldContext &WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);

	return World;
}

void UJavascriptTestLibrary::InitializeActorsForPlay(UWorld* World, const FURL& URL)
{
	World->InitializeActorsForPlay(URL);
}

void UJavascriptTestLibrary::BeginPlay(UWorld* World)
{
	World->BeginPlay();
}

static TArray<uint64> GFrameCounterStack;
void UJavascriptTestLibrary::PushFrameCounter()
{
	GFrameCounterStack.Add(GFrameCounter);
}

void UJavascriptTestLibrary::PopFrameCounter()
{
	GFrameCounter = GFrameCounterStack[GFrameCounterStack.Num() - 1];
	GFrameCounterStack.RemoveAt(GFrameCounterStack.Num() - 1, 1);
}

void UJavascriptTestLibrary::DestroyWorld(UWorld* World)
{
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
}
#endif