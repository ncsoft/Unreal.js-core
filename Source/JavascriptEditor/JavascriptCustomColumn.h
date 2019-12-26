#pragma once

#include "CoreMinimal.h"
#include "IPropertyTableColumn.h"
#include "IPropertyTableUtilities.h"
#include "IPropertyTableCustomColumn.h"
#include "JavascriptPropertyTable.h"

class IPropertyHandle;
class IPropertyTableCell;
class IPropertyTableCellPresenter;

class FJavascriptCustomColumn : public TSharedFromThis< FJavascriptCustomColumn >, public IPropertyTableCustomColumn
{
public:

	FJavascriptCustomColumn(UJavascriptPropertyTable* InJavascriptPropertyTable);

	/** Begin IPropertyTableCustomColumn interface */
	virtual bool Supports(const TSharedRef< IPropertyTableColumn >& Column, const TSharedRef< IPropertyTableUtilities >& Utilities) const override;
	virtual TSharedPtr< SWidget > CreateColumnLabel(const TSharedRef< IPropertyTableColumn >& Column, const TSharedRef< IPropertyTableUtilities >& Utilities, const FName& Style) const override;
	virtual TSharedPtr< IPropertyTableCellPresenter > CreateCellPresenter(const TSharedRef< IPropertyTableCell >& Cell, const TSharedRef< IPropertyTableUtilities >& Utilities, const FName& Style) const override;
	/** End IPropertyTableCustomColumn interface */

private:

	UJavascriptPropertyTable* JavascriptPropertyTable;
};
