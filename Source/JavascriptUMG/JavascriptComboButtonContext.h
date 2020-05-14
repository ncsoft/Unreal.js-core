#pragma once

#include "JavascriptUMGLibrary.h"
#include "JavascriptComboButtonContext.generated.h"

/**
 * 
 */
UCLASS()
class JAVASCRIPTUMG_API UJavascriptComboButtonContext : public UObject
{
	GENERATED_BODY()

public:

	DECLARE_DYNAMIC_DELEGATE_RetVal(FText, FTextDelegate);
	DECLARE_DYNAMIC_DELEGATE_RetVal(FJavascriptSlateIcon, FIconDelegate);
	//DECLARE_DYNAMIC_DELEGATE_RetVal(FJavascriptSlateWidget, FWidgetDelegate);
	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(FJavascriptSlateWidget, FJavascriptGetWidgetWithEditingObject, UObject*, EditingObject);
	DECLARE_DYNAMIC_DELEGATE_RetVal(bool, FBoolDelegate);

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FTextDelegate OnGetLabel;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FTextDelegate OnGetTooltip;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FIconDelegate OnGetIcon;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FJavascriptGetWidgetWithEditingObject OnGetWidget;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FBoolDelegate OnCanExecute;

	TSharedRef<SWidget> Public_OnGetWidget(UObject* EditingObject);
	FSlateIcon Public_OnGetSlateIcon();
	bool Public_CanExecute();
	FText Public_OnGetTooltip();
	FText Public_OnGetLabel();
};
