#pragma once

#include "CoreMinimal.h"
#include "ObjectMacros.h"
#include "Object.h"
#include "UObjectGlobals.h"
#include "ScriptMacros.h"
#include "JavascriptContext.generated.h"

struct FJavascriptContext;
class UJavascriptIsolate;

struct V8_API FArrayBufferAccessor
{	
	static int32 GetSize();
	static void* GetData();
	static void Discard();
};

UCLASS()
class V8_API UJavascriptContext : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	// Begin UObject interface.
	virtual void BeginDestroy() override;
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
	// End UObject interface.

	TSharedPtr<FJavascriptContext> JavascriptContext;

	TSharedPtr<FString> ContextId;

	UPROPERTY(BlueprintReadWrite, Category = "Scripting|Javascript")
	TArray<FString> Paths;

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	void SetContextId(FString Name);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	void Expose(FString Name, UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	FString GetScriptFileFullPath(FString Filename);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	FString ReadScriptFile(FString Filename);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	void RunFile(FString Filename);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	FString RunScript(FString Script, bool bOutput = true);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	void RequestV8GarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
    void FindPathFile(FString TargetRootPath, FString TargetFileName, TArray<FString>& OutFiles);
	
    UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	bool WriteAliases(FString Target);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	bool WriteDTS(FString Target, bool bIncludingTooltip);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	void SetAsDebugContext(int32 InPort = 5858);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	void ResetAsDebugContext();

	UFUNCTION(BlueprintPure, Category = "Scripting|Javascript")
	bool IsDebugContext() const;

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	void CreateInspector(int32 Port = 9229);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	void DestroyInspector();

	bool HasProxyFunction(UObject* Holder, UFunction* Function);
	bool CallProxyFunction(UObject* Holder, UObject* This, UFunction* Function, void* Parms);
};