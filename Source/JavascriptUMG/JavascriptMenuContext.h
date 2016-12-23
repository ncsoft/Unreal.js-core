#pragma once

#include "JavascriptUMGLibrary.h"
#include "JavascriptMenuContext.generated.h"

/**
 * 
 */
UCLASS()
class JAVASCRIPTUMG_API UJavascriptMenuContext : public UObject
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_DELEGATE_RetVal(bool, FBoolDelegate);
	DECLARE_DYNAMIC_DELEGATE(FExecuteAction);

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FText Description;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FText ToolTip;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FJavascriptSlateIcon Icon;	

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FBoolDelegate OnCanExecute;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FExecuteAction OnExecute;
	
	bool Public_CanExecute();
	void Public_Execute();
};
