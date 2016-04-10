#pragma once

#include "JavascriptEditorViewport.generated.h"

class SAutoRefreshEditorViewport;
/**
 * 
 */
UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptEditorViewport : public UContentWidget
{
	GENERATED_BODY()

public:	
#if WITH_EDITOR
	virtual TSharedRef<SWidget> RebuildWidget();

	TSharedPtr<class SAutoRefreshEditorViewport> ViewportWidget;
#endif

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	UWorld* GetViewportWorld() const;

	UFUNCTION(BlueprintCallable, Category = "Viewport")
	void Redraw();
};
