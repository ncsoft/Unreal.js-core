#pragma once

#include "JavascriptEditorLibrary.h"
#include "JavascriptUICommands.generated.h"

/**
*
*/
UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptUICommands : public UObject, public IEditorExtension
{
	GENERATED_UCLASS_BODY()

public:
	DECLARE_DYNAMIC_DELEGATE_OneParam(FJavascriptExecuteAction, FString, Id);

	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FJavascriptCanExecuteAction, FString, Id);

#if WITH_EDITOR
	UPROPERTY()
	FJavascriptExecuteAction OnExecuteAction;	

	UPROPERTY()
	FJavascriptCanExecuteAction OnCanExecuteAction;	

	UPROPERTY()
	FJavascriptCanExecuteAction OnIsActionChecked;	

	UPROPERTY()
	FJavascriptCanExecuteAction OnIsActionButtonVisible;	

	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	TArray<FJavascriptUICommand> Commands;

	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	FString ContextName;
	
	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	FText ContextDesc;
	
	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	FName ContextNameParent;
	
	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	FName StyleSetName;

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	void Commit();

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	void Discard();

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	void Refresh();

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	void Bind(FJavascriptUICommandList List);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	void Unbind(FJavascriptUICommandList List);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void BroadcastCommandsChanged();

	bool bRegistered;

	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	TArray<FJavascriptUICommandInfo> CommandInfos;

	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	FJavascriptBindingContext BindingContext;

	void Bind(FUICommandList* CommandList);
	void Unbind(FUICommandList* CommandList);

	virtual void Register() override;
	virtual void Unregister() override;

	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	FJavascriptUICommandInfo GetAction(FString Id);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	void Initialize();

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	void Uninitialize();
#endif
};
