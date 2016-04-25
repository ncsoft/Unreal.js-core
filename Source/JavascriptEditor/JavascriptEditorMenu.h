#pragma once

#include "JavascriptMenuLibrary.h"
#include "JavascriptEditorMenu.generated.h"

USTRUCT()
struct FJavascriptEditorPullDownMenu
{
	GENERATED_BODY()

	UPROPERTY()
	FName Id;

	UPROPERTY()
	FText Label;

	UPROPERTY()
	FText Tooltip;
};
/**
 * 
 */
UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptEditorMenu : public UWidget
{
	GENERATED_UCLASS_BODY()

public:	
	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnHook, FName, Hook);

#if WITH_EDITOR
	UPROPERTY()
	TArray<FJavascriptEditorPullDownMenu> SubMenus;

	UPROPERTY()
	FJavascriptUICommandList CommandList;

	UPROPERTY()
	FOnHook OnHook;
	
	void Setup(TSharedRef<SBox> VerticalBox);
	
	virtual TSharedRef<SWidget> RebuildWidget();
#endif
};
