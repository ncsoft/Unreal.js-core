#pragma once

#include "JavascriptEditorLibrary.h"
#include "JavascriptEditorToolbar.generated.h"


/**
 * 
 */
UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptEditorToolbar : public UWidget
{
	GENERATED_UCLASS_BODY()

public:	
	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnHook, FName, Hook);

#if WITH_EDITOR
	UPROPERTY()
	FJavascriptUICommandList CommandList;

	UPROPERTY()
	FOnHook OnHook;

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void AddToolBarButton(FJavascriptUICommandInfo CommandInfo);
	
	void Setup(TSharedRef<SBox> Box);
	
	virtual TSharedRef<SWidget> RebuildWidget();
#endif
};
