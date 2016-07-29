#include "V8PCH.h"
#include "JavascriptTestLibrary.h"
#include "Misc/AutomationTest.h"

#if WITH_EDITOR
struct FJavascriptAutomatedTestImpl : FAutomationTestBase, TSharedFromThis<FJavascriptAutomatedTestImpl>
{
	FJavascriptAutomatedTest Recipe;

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
		uint64 InitialFrameCounter = GFrameCounter;
		{
			Params.Tester.Handle = this->AsShared();

			Recipe.Function.Execute(FJavascriptAutomatedTestParameters::StaticStruct(), &Params);
		}		

		return true;
	}

	void RunWorld(const FURL& URL, FJavascriptFunction Function)
	{
		UWorld *World = UWorld::CreateWorld(EWorldType::Game, false);
		FWorldContext &WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
		WorldContext.SetCurrentWorld(World);

		World->InitializeActorsForPlay(URL);
		World->BeginPlay();

		// run the matching test
		uint64 InitialFrameCounter = GFrameCounter;
		{
			FJavascriptRunWorldParameters Params;
			Params.World = World;

			Function.Execute(FJavascriptRunWorldParameters::StaticStruct(), &Params);
		}
		GFrameCounter = InitialFrameCounter;

		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
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

void UJavascriptTestLibrary::RunWorld(const FJavascriptAutomatedTestInstance& Test, const FURL& URL, FJavascriptFunction Function)
{
	if (Test.Handle.IsValid())
	{
		Test.Handle->RunWorld(URL, Function);
	}
}

void UJavascriptTestLibrary::ClearExecutionInfo(const FJavascriptAutomatedTestInstance& Test)
{
	if (Test.Handle.IsValid())
	{
		Test.Handle->ClearExecutionInfo();
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
#endif