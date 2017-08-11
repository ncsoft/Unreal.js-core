// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Widget.h"
#include "SJavascriptGraphPinObject.h"
#include "JavascriptGraphEditorLibrary.h"
#include "JavascriptGraphPinObject.generated.h"

/**
*
*/
UCLASS()
class JAVASCRIPTGRAPHEDITOR_API UJavascriptGraphPinObject : public UWidget
{
	GENERATED_UCLASS_BODY()
	
public:

	//~ Begin UWidget interface
	virtual void SynchronizeProperties() override;
	// End UWidget interface

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	DECLARE_DYNAMIC_DELEGATE_RetVal(FJavascriptEdGraphPin, FOnGetGraphPin);

	DECLARE_DYNAMIC_DELEGATE_RetVal(UObject*, FOnGetDefaultValue);
	
	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnSetDefaultValue, FText, Value);

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGetGraphPin OnGetGraphPin;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGetDefaultValue OnGetDefaultValue;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnSetDefaultValue OnSetDefaultValue;

protected:
	TSharedPtr<class SJavascriptGraphPinObject> Pin;

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface
};