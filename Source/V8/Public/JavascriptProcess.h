#pragma once

#include "JavascriptProcess.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class V8_API UJavascriptProcess: public UObject
{
	GENERATED_UCLASS_BODY()

public:
	FProcHandle ProcessHandle;
	uint32 ProcessID;
	void* ReadPipe;
	void* WritePipe;

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	static UJavascriptProcess* Create(const FString& URL, const FString& Parms, bool bLaunchDetached, bool bLaunchHidden, bool bLaunchReallyHidden, int32 PriorityModifier, const FString& OptionalWorkingDirectory, bool bUsePipe);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	static UJavascriptProcess* Open(const FString& ProcName);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	static UJavascriptProcess* Open_PID(int32 ProcessId);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	static FString GetApplicationName(int32 ProcessId);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	static int32 GetCurrentProcessId();

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	static bool IsApplicationRunning_PID(int32 ProcessId);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	static bool IsApplicationRunning(const FString& ProcName);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	static void SetEnvironmentVar(const FString& VarName, const FString& VarValue);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	static FString GetEnvironmentVar(const FString& VarName);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	bool IsRunning();

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	void Close();

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	static void Sleep(float Seconds);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	void Wait();

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	void Terminate(bool KillTree = false);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	bool GetReturnCode(int32& ReturnCode);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	FString ReadFromPipe();

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	bool ReadArrayFromPipe(TArray<uint8>& Array);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	bool WriteToPipe(const FString& Message, FString& OutWritten);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	static void SimulateKeypress(int32 KeyEvent);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	static void LaunchURL(const FString& URL, const FString& Parms, FString& Error);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	static bool CanLaunchURL(const FString& URL);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Scripting")
	static FString GetString(const FString& Key, bool bFlag);
};
