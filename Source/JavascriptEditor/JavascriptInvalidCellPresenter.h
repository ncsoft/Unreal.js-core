#pragma once

#include "CoreMinimal.h"
#include "Widgets/SWidget.h"
#include "IPropertyTableCell.h"
#include "IPropertyTableCellPresenter.h"
#include "JavascriptPropertyTable.h"

class FJavascriptInvalidCellPresenter : public TSharedFromThis< FJavascriptInvalidCellPresenter >, public IPropertyTableCellPresenter
{
public:

	FJavascriptInvalidCellPresenter(UJavascriptPropertyTable* InJavascriptPropertyTable);

	virtual ~FJavascriptInvalidCellPresenter() {}

	virtual TSharedRef< class SWidget > ConstructDisplayWidget() override;

	virtual bool RequiresDropDown() override;

	virtual TSharedRef< class SWidget > ConstructEditModeCellWidget() override;

	virtual TSharedRef< class SWidget > ConstructEditModeDropDownWidget() override;

	virtual TSharedRef< class SWidget > WidgetToFocusOnEdit() override;

	virtual FString GetValueAsString() override;

	virtual FText GetValueAsText() override;

	virtual bool HasReadOnlyEditMode() override { return false; }
	
private:

	UJavascriptPropertyTable* JavascriptPropertyTable;
};
