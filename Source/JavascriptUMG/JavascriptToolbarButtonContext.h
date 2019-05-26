#pragma once

#include "JavascriptUMGLibrary.h"
#include "JavascriptToolbarButtonContext.generated.h"

/**
 * 
 */
UCLASS()
class JAVASCRIPTUMG_API UJavascriptToolbarButtonContext : public UObject
{
	GENERATED_BODY()

public:

	DECLARE_DYNAMIC_DELEGATE_OneParam(FJavascriptExecuteActionWithEditingObject, UObject*, EditingObject);
	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FJavascriptCanExecuteActionWithEditingObject, UObject*, EditingObject);

	DECLARE_DYNAMIC_DELEGATE_RetVal(FText, FTextDelegate);
	DECLARE_DYNAMIC_DELEGATE_RetVal(FJavascriptSlateIcon, FIconDelegate);
	DECLARE_DYNAMIC_DELEGATE_RetVal(bool, FBoolDelegate);

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FTextDelegate OnGetLabel;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FTextDelegate OnGetTooltip;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FIconDelegate OnGetIcon;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FJavascriptExecuteActionWithEditingObject OnExecuteAction;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FJavascriptCanExecuteActionWithEditingObject OnCanExecuteAction;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FJavascriptCanExecuteActionWithEditingObject OnIsActionChecked;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FJavascriptCanExecuteActionWithEditingObject OnIsActionButtonVisible;
	
	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	void MarkReferencedObject();

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	void UnmarkReferencedObject();

	FSlateIcon Public_OnGetSlateIcon();
	FText Public_OnGetTooltip();
	FText Public_OnGetLabel();

	void Public_OnExecuteAction(UObject* EditingObject);
	bool Public_OnCanExecuteAction(UObject* EditingObject);
	bool Public_OnIsActionChecked(UObject* EditingObject);
	bool Public_OnIsActionButtonVisible(UObject* EditingObject);
};
