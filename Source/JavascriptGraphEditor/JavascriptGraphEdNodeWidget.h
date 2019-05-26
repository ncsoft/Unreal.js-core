#pragma once

#include "Components/Widget.h"
#include "SJavascriptGraphEdNode.h"
#include "JavascriptGraphEdNodeWidget.generated.h"

class UJavascriptGraphEdNode;

/**
 * 
 */
UCLASS()
class JAVASCRIPTGRAPHEDITOR_API UJavascriptGraphEdNodeWidget : public UWidget
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, Category = Content)
	UJavascriptGraphEdNode* EdNode;
	
	UFUNCTION(BlueprintCallable, Category = Content)
	void SetNode(UJavascriptGraphEdNode* InEdNode);

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface
};