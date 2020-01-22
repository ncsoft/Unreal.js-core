#pragma once

#include "JavascriptMenuLibrary.h"
#include "Components/Widget.h"
#include "Widgets/Layout/SBox.h"
#include "JavascriptMultiBox.generated.h"


/**
 *
 */
UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptMultiBox : public UWidget
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnHook, FName, Id, UJavascriptMultiBox*, Self, FJavascriptMenuBuilder, CurrentBuilder);

	UPROPERTY()
	FOnHook OnHook;

	UFUNCTION(BlueprintInternalUseOnly, Category = "Scripting | Javascript")
	void AddPullDownMenu(FJavascriptMenuBuilder& Builder, FName Id, const FText& Label, const FText& ToolTip);

	UFUNCTION(BlueprintInternalUseOnly, Category = "Scripting | Javascript")
	void AddSubMenu(FJavascriptMenuBuilder& Builder, FName Id, const FText& Label, const FText& ToolTip, const bool bInOpenSubMenuOnClick);

	void Setup(TSharedRef<SBox> Box);

	UFUNCTION(BlueprintInternalUseOnly, Category = "Scripting | Javascript")
	static void Bind(FJavascriptMenuBuilder Builder);

	virtual TSharedRef<SWidget> RebuildWidget();
};
