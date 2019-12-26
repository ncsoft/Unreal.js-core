#pragma once

#include "Components/Widget.h"
#include "SJavascriptGraphEdNode.h"
#include "JavascriptUMG/JavascriptUMGLibrary.h"
#include "JavascriptGraphEdCustomNodeWidget.generated.h"

class UJavascriptGraphEdNode;

/**
 * 
 */
UCLASS()
class JAVASCRIPTGRAPHEDITOR_API UJavascriptGraphEdCustomNodeWidget : public UWidget
{
	GENERATED_UCLASS_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = Content)
	void SetNode(UJavascriptGraphEdNode* InEdNode);

	UFUNCTION(BlueprintCallable, Category = Content)
	void SetGraphPanel(FJavascriptSlateWidget InGraphPanel);

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface

	UPROPERTY(EditAnywhere, Category = Content)
	UJavascriptGraphEdNode* EdNode;

	TWeakPtr<SGraphPanel> GraphPanel;
};
