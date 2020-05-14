#include "JavascriptCustomColumn.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "EditorStyleSet.h"
#include "IPropertyTable.h"
#include "IPropertyTableCell.h"
#include "JavascriptInvalidCellPresenter.h"

FJavascriptCustomColumn::FJavascriptCustomColumn(UJavascriptPropertyTable* InJavascriptPropertyTable)
	: JavascriptPropertyTable(InJavascriptPropertyTable)
{
}

bool FJavascriptCustomColumn::Supports(const TSharedRef< IPropertyTableColumn >& Column, const TSharedRef< IPropertyTableUtilities >& Utilities) const
{
	if (Column->GetDataSource()->IsValid())
	{
		TSharedPtr< FPropertyPath > PropertyPath = Column->GetDataSource()->AsPropertyPath();
		if (PropertyPath.IsValid() && PropertyPath->GetNumProperties() > 0)
		{
			const FPropertyInfo& PropertyInfo = PropertyPath->GetRootProperty();
			FProperty* Property = PropertyInfo.Property.Get();
			if (!Property->HasMetaData(TEXT("IgnoreCustomColumn")))
			{
				return true;
			}
		}
	}

	return false;
}

TSharedPtr< SWidget > FJavascriptCustomColumn::CreateColumnLabel(const TSharedRef< IPropertyTableColumn >& Column, const TSharedRef< IPropertyTableUtilities >& Utilities, const FName& Style) const
{
	TSharedPtr< FPropertyPath > PropertyPath = Column->GetDataSource()->AsPropertyPath();
	const FPropertyInfo& PropertyInfo = PropertyPath->GetRootProperty();
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(STextBlock)
			.Font(FEditorStyle::GetFontStyle(Style))
			.Text(Column->GetDisplayName())
		];
}

TSharedPtr< IPropertyTableCellPresenter > FJavascriptCustomColumn::CreateCellPresenter(const TSharedRef< IPropertyTableCell >& Cell, const TSharedRef< IPropertyTableUtilities >& Utilities, const FName& Style) const
{
	TSharedPtr< IPropertyHandle > PropertyHandle = Cell->GetPropertyHandle();
	if (PropertyHandle.IsValid())
	{
		if (Cell->IsReadOnly())
		{
			return MakeShareable(new FJavascriptInvalidCellPresenter(JavascriptPropertyTable));
		}
	}

	return NULL;
};
