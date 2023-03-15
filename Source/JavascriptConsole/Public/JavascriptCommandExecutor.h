#pragma once

class FJavascriptCommandExecutor : public IConsoleCommandExecutor
{
public:
	FJavascriptCommandExecutor(TSharedPtr<FString> InTargetContext = nullptr);

	static FName StaticName();
	virtual FName GetName() const override;
	virtual FText GetDisplayName() const override;
	virtual FText GetDescription() const override;
	virtual FText GetHintText() const override;
	virtual void GetAutoCompleteSuggestions(const TCHAR* Input, TArray<FString>& Out) override;
	virtual void GetExecHistory(TArray<FString>& Out) override;
	virtual bool Exec(const TCHAR* Input) override;
	virtual bool AllowHotKeyClose() const override;
	virtual bool AllowMultiLine() const override;
	virtual FInputChord GetHotKey() const override;

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1 
	virtual FInputChord GetIterateExecutorHotKey() const override;
#endif

	TSharedPtr<FString> GetTargetContext() const
	{
		return TargetContext;
	}

private:
	bool CacheJavascriptContext();

private:
	FName CommandExecutorName;
	TSharedPtr<FString> TargetContext;
};