#pragma once

#include "JavascriptIsolate.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "JavascriptTestLibrary.generated.h"

struct FJavascriptAutomatedTestImpl;

USTRUCT(BlueprintType)
struct FJavascriptAutomatedTestInstance
{
	GENERATED_BODY()

public:
	TSharedPtr<FJavascriptAutomatedTestImpl> Handle;
};

USTRUCT(BlueprintType)
struct FJavascriptAutomatedTestParameters
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString TestFunctionName;

	UPROPERTY()
	FJavascriptAutomatedTestInstance Tester;
};

USTRUCT(BlueprintType)
struct FJavascriptAutomatedTest
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FString Name;

	UPROPERTY()
	bool bComplexTask;

	UPROPERTY()
	int32 TestFlags;

	UPROPERTY()
	int32 RequiredDeviceNum;

	UPROPERTY()
	TArray<FString> TestFunctionNames;

	UPROPERTY()
	FJavascriptFunction Function;

#if WITH_EDITOR	
#endif
};



/**
 * 
 */
UCLASS()
class V8_API UJavascriptTestLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static FJavascriptAutomatedTestInstance Create(const FJavascriptAutomatedTest& Test);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void Destroy(FJavascriptAutomatedTestInstance& Test);

	/** Clear any execution info/results from a prior running of this test */
	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void ClearExecutionInfo(const FJavascriptAutomatedTestInstance& Test);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void SetContinue(const FJavascriptAutomatedTestInstance& Test, bool bInContinue);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void AddError(const FJavascriptAutomatedTestInstance& Test,const FString& InError);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void AddWarning(const FJavascriptAutomatedTestInstance& Test,const FString& InWarning);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void AddLogItem(const FJavascriptAutomatedTestInstance& Test,const FString& InLogItem);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void AddAnalyticsItem(const FJavascriptAutomatedTestInstance& Test,const FString& InAnalyticsItem);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static UWorld* NewWorld();

	UFUNCTION(BlueprintInternalUseOnly, Category = "Javascript | Editor")
	static void InitializeActorsForPlay(UWorld* World, const FURL& URL);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void BeginPlay(UWorld* World);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void PushFrameCounter();

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void PopFrameCounter();

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void DestroyWorld(UWorld* World);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void DestroyUObject(UObject* Object);
};
