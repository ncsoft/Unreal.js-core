#pragma once

#include "Components/Widget.h"
#include "JavascriptColorPicker.generated.h"

class SColorBlock;

UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptColorPicker : public UWidget
{
	GENERATED_UCLASS_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnColorChangedEvent, const FLinearColor&, Color);

public:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface

public:
	UPROPERTY(BlueprintAssignable, Category = "Widget Event", meta = (DisplayName = "OnColorChangedEvent"))
	FOnColorChangedEvent OnColorChanged;

	UPROPERTY()
	FLinearColor SelectedColor;

private:
	TSharedPtr<SColorBlock> MyColorBlock;
};
