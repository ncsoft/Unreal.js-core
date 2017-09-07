#pragma once

#include "JavascriptIsolate.generated.h"

class UJavascriptContext;
class FJavascriptIsolate;

USTRUCT()
struct FJavascriptRawAccess_Data
{
	GENERATED_BODY()
};

USTRUCT()
struct FJavascriptRawAccess
{
	GENERATED_BODY()
public:
    virtual ~FJavascriptRawAccess() {}
	virtual UScriptStruct* GetScriptStruct(int32 Index) { return nullptr; }
	virtual void* GetData(int32 Index) { return nullptr; }
	virtual int32 GetNumData() { return 0; }
	virtual FName GetDataName(int32 Index) { return FName(); }
};

USTRUCT()
struct FJavascriptMemoryStruct
{
	GENERATED_BODY()

public:
    virtual ~FJavascriptMemoryStruct() {}
	virtual int32 GetDimension() { return 1;  }
	virtual void* GetMemory(const int32* Dim) { return nullptr; }
	virtual int32 GetSize(int32 Dim) { return 0; }
};

struct FPrivateJavascriptFunction;
struct FPrivateJavascriptRef;

USTRUCT(BlueprintType)
struct V8_API FJavascriptFunction
{
	GENERATED_BODY()

public:
	void Execute();
	void Execute(UScriptStruct* Struct, void* Buffer);

	TSharedPtr<FPrivateJavascriptFunction> Handle;
};

USTRUCT(BlueprintType)
struct V8_API FJavascriptRef
{
	GENERATED_BODY()

public:
	TSharedPtr<FPrivateJavascriptRef> Handle;
};

USTRUCT(BlueprintType)
struct V8_API FJavascriptHeapStatistics
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Scripting | Javascript")
	int32 TotalHeapSize;

	UPROPERTY(BlueprintReadOnly, Category = "Scripting | Javascript")
	int32 TotalHeapSizeExecutable;

	UPROPERTY(BlueprintReadOnly, Category = "Scripting | Javascript")
	int32 TotalPhysicalSize;

	UPROPERTY(BlueprintReadOnly, Category = "Scripting | Javascript")
	int32 TotalAvailableSize;

	UPROPERTY(BlueprintReadOnly, Category = "Scripting | Javascript")
	int32 UsedHeapSize;

	UPROPERTY(BlueprintReadOnly, Category = "Scripting | Javascript")
	int32 HeapSizeLimit;

	UPROPERTY(BlueprintReadOnly, Category = "Scripting | Javascript")
	int32 MallocedMemory;

	UPROPERTY(BlueprintReadOnly, Category = "Scripting | Javascript")
	bool bDoesZapGarbage;
};

UCLASS()
class V8_API UJavascriptIsolate : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	virtual void BeginDestroy() override;

	TSharedPtr<FJavascriptIsolate> JavascriptIsolate;

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	UJavascriptContext* CreateContext();

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	void GetHeapStatistics(FJavascriptHeapStatistics& Statistics);

	// Begin UObject interface.
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
	// End UObject interface.
};
