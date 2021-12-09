#pragma once

#include "CoreMinimal.h"
#include "Components/Widget.h"
#include "IPropertyTable.h"
#include "JavascriptUMG/JavascriptUMGLibrary.h"
#include "JavascriptPropertyTable.generated.h"

DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(FJavascriptSlateWidget, FOnGenerateCustomCellWidget, UObject*, Object, FString, ColumnName);
DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(bool, FOnUseCustomCellWidget, UObject*, Object, FString, ColumnName);

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
	FOnGenerateCustomCellWidget OnGenerateCustomCellWidget;

	UPROPERTY(EditAnywhere, Category = "JavascriptPropertyTable")
	FOnUseCustomCellWidget OnUseCustomCellWidget;

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
