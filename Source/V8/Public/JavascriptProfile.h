#pragma once

#include "CoreMinimal.h"
#include "JavascriptProfile.generated.h"

USTRUCT(BlueprintType)
struct FJavascriptCpuProfiler
{
	GENERATED_BODY()

	void* Profiler{ nullptr };
};

USTRUCT(BlueprintType)
struct FJavascriptProfileNode
{
	GENERATED_BODY()

	const void* Node;
};

UCLASS(BlueprintType)
class V8_API UJavascriptProfile : public UObject
{
	GENERATED_BODY()

public:
	void* Profile{ nullptr };

	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	FJavascriptProfileNode GetTopDownRoot();

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	int32 GetSamplesCount();
	
	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	FJavascriptProfileNode GetSample(int32 index);
		
	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	float GetSampleTimestamp(int32 index);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	static FJavascriptCpuProfiler Start(const FString& Title, bool bRecordSamples);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	static UJavascriptProfile* Stop(const FJavascriptCpuProfiler& Profiler, const FString& Title);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	static void SetSamplingInterval(const FJavascriptCpuProfiler& Profiler, int32 us);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	static void SetIdle(const FJavascriptCpuProfiler& Profiler, bool is_idle);
};
