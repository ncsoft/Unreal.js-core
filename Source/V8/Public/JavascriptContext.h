#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/ScriptMacros.h"
#include "JavascriptIsolate.h"
#include "JavascriptContext.generated.h"

struct FJavascriptContext;
struct IConsoleCommand;
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

#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	struct FConsoleCommandInfo
	{
		FConsoleCommandInfo() {}
		FConsoleCommandInfo(IConsoleCommand* _Command, TSharedPtr<FJavascriptFunction> _Function)
		{
			Command = _Command;
			Function = _Function;
		}

		IConsoleCommand* Command;
		TSharedPtr<FJavascriptFunction> Function;
	};
	TMap<FString, TSharedPtr<FConsoleCommandInfo>> JavascriptConsoleCommands;
#endif

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
	FString RunFile(FString Filename);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	FString RunFileWithArgs(FString Filename, const TArray<FString>& Args);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	FString RunScript(FString Script, bool bOutput = true);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	void RegisterConsoleCommand(FString Command, FString Help, FJavascriptFunction Function);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	void UnregisterConsoleCommand(FString Command);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	void RequestV8GarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
    void FindPathFile(FString TargetRootPath, FString TargetFileName, TArray<FString>& OutFiles);
	
    UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	bool WriteAliases(FString Target);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	bool WriteDTS(FString Target, bool bIncludingTooltip);

	UFUNCTION(BlueprintPure, Category = "Scripting|Javascript")
	bool IsDebugContext() const;

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	void CreateInspector(int32 Port = 9229);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	void DestroyInspector();

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	void ResetUnrealConsoleDelegate();

	bool RemoveObjectInJavacontext(UObject* TargetObj);

	bool HasProxyFunction(UObject* Holder, UFunction* Function);
	bool CallProxyFunction(UObject* Holder, UObject* This, UFunction* Function, void* Parms);
};
