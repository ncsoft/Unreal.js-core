PRAGMA_DISABLE_SHADOW_VARIABLE_WARNINGS

#include "JavascriptTestLibrary.h"
#include "Engine/Engine.h"
#include "Misc/AutomationTest.h"

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
struct FJavascriptAutomatedTestImpl : FAutomationTestBase, TSharedFromThis<FJavascriptAutomatedTestImpl>
{
	FJavascriptAutomatedTest Recipe;

	bool bContinue{ false };

	FJavascriptAutomatedTestImpl(const FJavascriptAutomatedTest& InRecipe)
		: FAutomationTestBase(InRecipe.Name, InRecipe.bComplexTask), Recipe(InRecipe)
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
#else
struct FJavascriptAutomatedTestImpl : TSharedFromThis<FJavascriptAutomatedTestImpl>
{};
#endif

FJavascriptAutomatedTestInstance UJavascriptTestLibrary::Create(const FJavascriptAutomatedTest& Test)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	return{ MakeShareable(new FJavascriptAutomatedTestImpl(Test)) };
#else
	return{ MakeShareable(new FJavascriptAutomatedTestImpl) };
#endif
}

void UJavascriptTestLibrary::Destroy(FJavascriptAutomatedTestInstance& Test)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	Test.Handle.Reset();
#endif
}

void UJavascriptTestLibrary::ClearExecutionInfo(const FJavascriptAutomatedTestInstance& Test)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (Test.Handle.IsValid())
	{
		Test.Handle->ClearExecutionInfo();
	}
#endif
}

void UJavascriptTestLibrary::SetContinue(const FJavascriptAutomatedTestInstance& Test, bool bContinue)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (Test.Handle.IsValid())
	{
		Test.Handle->bContinue = bContinue;
	}
#endif
}

void UJavascriptTestLibrary::AddError(const FJavascriptAutomatedTestInstance& Test, const FString& InError)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (Test.Handle.IsValid())
	{
		Test.Handle->AddError(InError);
	}
#endif
}

void UJavascriptTestLibrary::AddWarning(const FJavascriptAutomatedTestInstance& Test, const FString& InWarning)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (Test.Handle.IsValid())
	{
		Test.Handle->AddWarning(InWarning);
	}
#endif
}

void UJavascriptTestLibrary::AddLogItem(const FJavascriptAutomatedTestInstance& Test, const FString& InLogItem)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (Test.Handle.IsValid())
	{
		Test.Handle->AddInfo(InLogItem);
	}
#endif
}

void UJavascriptTestLibrary::AddAnalyticsItem(const FJavascriptAutomatedTestInstance& Test, const FString& InAnalyticsItem)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	if (Test.Handle.IsValid())
	{
		Test.Handle->AddAnalyticsItem(InAnalyticsItem);
	}
#endif
}

UWorld* UJavascriptTestLibrary::NewWorld()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	UWorld *World = UWorld::CreateWorld(EWorldType::Game, false);
	FWorldContext &WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);

	return World;
#else
	return nullptr;
#endif
}

void UJavascriptTestLibrary::InitializeActorsForPlay(UWorld* World, const FURL& URL)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	World->InitializeActorsForPlay(URL);
#endif
}

void UJavascriptTestLibrary::BeginPlay(UWorld* World)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	World->BeginPlay();
#endif
}

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
static TArray<uint64> GFrameCounterStack;
#endif

void UJavascriptTestLibrary::PushFrameCounter()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	GFrameCounterStack.Add(GFrameCounter);
#endif
}

void UJavascriptTestLibrary::PopFrameCounter()
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	GFrameCounter = GFrameCounterStack[GFrameCounterStack.Num() - 1];
	GFrameCounterStack.RemoveAt(GFrameCounterStack.Num() - 1, 1);
#endif
}

void UJavascriptTestLibrary::DestroyWorld(UWorld* World)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
#endif
}

void UJavascriptTestLibrary::DestroyUObject(UObject* Object)
{
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	Object->ConditionalBeginDestroy();
#endif
}

PRAGMA_ENABLE_SHADOW_VARIABLE_WARNINGS

