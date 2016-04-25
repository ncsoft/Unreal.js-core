#pragma once

#include "JavascriptMenuLibrary.h"
#include "JavascriptMultiBox.generated.h"


/**
 * 
 */
UCLASS()
class JAVASCRIPTUMG_API UJavascriptMultiBox : public UWidget
{
	GENERATED_BODY()

public:	
	DECLARE_DYNAMIC_DELEGATE_RetVal_ThreeParams(FJavascriptMenuBuilder, FOnHook, FName, Id, UJavascriptMultiBox*, Self, FJavascriptMenuBuilder, CurrentBuilder);

	UPROPERTY()
	FOnHook OnHook;

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	void AddPullDownMenu(FJavascriptMenuBuilder& Builder, FName Id, const FText& Label, const FText& ToolTip);
	
	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	void AddSubMenu(FJavascriptMenuBuilder& Builder, FName Id, const FText& Label, const FText& ToolTip, const bool bInOpenSubMenuOnClick);

	void Setup(TSharedRef<SBox> Box);
	
	virtual TSharedRef<SWidget> RebuildWidget();
};
