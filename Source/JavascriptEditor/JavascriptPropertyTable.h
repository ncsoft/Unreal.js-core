#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "IPropertyTable.h"
#include "JavascriptUMG/JavascriptUMGLibrary.h"
#include "JavascriptPropertyTable.generated.h"

DECLARE_DYNAMIC_DELEGATE_RetVal(FJavascriptSlateWidget, FOnGenerateInvalidCellWidget);

UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptPropertyTable : public UWidget
{
	GENERATED_UCLASS_BODY()

public:

	virtual void ReleaseSlateResources(bool bReleaseChildren) override;

	UFUNCTION(BlueprintCallable, Category = "JavascriptPropertyTable")
	TArray<UObject*> GetEditingObjects() { return EditingObjects; };
	
	UFUNCTION(BlueprintCallable, Category = "JavascriptPropertyTable")
	void SetEditingObjects(TArray<UObject*> InEditingObjects);

	UFUNCTION(BlueprintCallable, Category = "JavascriptPropertyTable")
	TArray<UObject*> GetSelectedTableObjects();

	UPROPERTY(EditAnywhere, Category = "JavascriptPropertyTable")
	FOnGenerateInvalidCellWidget OnGenerateInvalidCellWidget;

	UPROPERTY(EditAnywhere, Category = "JavascriptPropertyTable")
	bool bUseCustomColumns;

protected:

	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface

private:
	TSharedPtr< class IPropertyTable > PropertyTable;
	TArray<UObject*> EditingObjects;
};
