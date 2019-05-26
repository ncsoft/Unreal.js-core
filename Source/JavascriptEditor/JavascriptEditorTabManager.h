#pragma once

#include "Components/Widget.h"
#include "Widgets/Layout/SBox.h"
#include "JavascriptEditorTab.h"
#include "JavascriptEditorTabManager.generated.h"

class UJavascriptEditorTabManager;

/**
 * 
 */
UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptEditorTabManager : public UWidget
{
	GENERATED_UCLASS_BODY()

public:	
#if WITH_EDITOR
	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	FString Layout;

	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	TArray<UJavascriptEditorTab*> Tabs;
	
	void Commit();
	void Setup(TSharedRef<SBox> VerticalBox);

	virtual TSharedRef<SWidget> RebuildWidget();
#endif
};
