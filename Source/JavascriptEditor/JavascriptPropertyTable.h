#pragma once

#include "Components/Widget.h"
#include "IPropertyTable.h"
#include "JavascriptPropertyTable.generated.h"

UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptPropertyTable : public UWidget
{
	GENERATED_UCLASS_BODY()

public:

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	UFUNCTION(BlueprintCallable, Category = "JavascriptPropertyTable")
	TArray<UObject*> GetEditingObjects() { return EditingObjects; };
	
	UFUNCTION(BlueprintCallable, Category = "JavascriptPropertyTable")
	void SetEditingObjects(TArray<UObject*> InEditingObjects) { EditingObjects = InEditingObjects; };

protected:

	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface

private:
	TSharedPtr< class IPropertyTable > PropertyTable;
	TArray<UObject*> EditingObjects;
};