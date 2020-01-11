#pragma once

#include "Components/Widget.h"
#include "JavascriptEditorLibrary.h"
#include "JavascriptEditorToolbar.generated.h"


/**
 * 
 */
UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptEditorToolbar : public UWidget
{
	GENERATED_BODY()

public:	
	DECLARE_DYNAMIC_DELEGATE_RetVal(FJavascriptMenuBuilder, FOnHook);

#if WITH_EDITOR
	UPROPERTY()
	FOnHook OnHook;

	void Setup(TSharedRef<SBox> Box);
	
	virtual TSharedRef<SWidget> RebuildWidget();
#endif
};
