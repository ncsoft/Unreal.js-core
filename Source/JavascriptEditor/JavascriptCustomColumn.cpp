#include "JavascriptCustomColumn.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "EditorStyleSet.h"
#include "IPropertyTable.h"
#include "IPropertyTableCell.h"
#include "JavascriptCustomCellPresenter.h"

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
			.Font(FAppStyle::Get().GetFontStyle(Style))
			.Text(Column->GetDisplayName())
		];
}

TSharedPtr< IPropertyTableCellPresenter > FJavascriptCustomColumn::CreateCellPresenter(const TSharedRef< IPropertyTableCell >& Cell, const TSharedRef< IPropertyTableUtilities >& Utilities, const FName& Style) const
{
	if (IsValid(JavascriptPropertyTable) && JavascriptPropertyTable->OnUseCustomCellWidget.IsBound())
	{
		TWeakObjectPtr<UObject> WeakObject = Cell->GetObject();
		TSharedRef<IPropertyTableColumn> Column = Cell->GetColumn();
		if (WeakObject.IsValid())
		{
			FString ColumnName = Column->GetId().ToString();
			if (JavascriptPropertyTable->OnUseCustomCellWidget.Execute(WeakObject.Get(), ColumnName))
			{
				return MakeShareable(new FJavascriptCustomCellPresenter(JavascriptPropertyTable, Cell));
			}
		}
	}

	return nullptr;
};
