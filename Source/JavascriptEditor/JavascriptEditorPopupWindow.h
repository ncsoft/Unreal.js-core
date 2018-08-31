#pragma once

#include "Components/Widget.h"
#include "Framework/Application/IMenu.h"
#include "JavascriptEditorPopupWindow.generated.h"

/**
 * 
 */
UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptEditorPopupWindow : public UObject
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	UPROPERTY(BlueprintReadOnly, Category = "Scripting | Javascript")
	UWidget* Widget;

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	bool Open(const FText& Heading, const FVector2D& DesiredSize);

	UFUNCTION(BlueprintImplementableEvent, Category = "Scripting | Javascript")
	void OnDismissed();

	void OnDismissedRaw(TSharedRef<IMenu> Menu);
#endif
};
