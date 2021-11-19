#include "JavascriptCustomCellPresenter.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SBoxPanel.h"
#include "IPropertyTableColumn.h"

FJavascriptCustomCellPresenter::FJavascriptCustomCellPresenter(UJavascriptPropertyTable* InJavascriptPropertyTable, const TSharedRef< IPropertyTableCell >& InCell)
	: JavascriptPropertyTable(InJavascriptPropertyTable)
	, Cell(InCell)
{
}

TSharedRef< class SWidget > FJavascriptCustomCellPresenter::ConstructDisplayWidget()
{
	if (IsValid(JavascriptPropertyTable) && JavascriptPropertyTable->OnGenerateCustomCellWidget.IsBound())
	{
		TWeakObjectPtr<UObject> WeakObject = Cell->GetObject();
		TSharedRef<IPropertyTableColumn> Column = Cell->GetColumn();
		if (WeakObject.IsValid())
		{
			FString ColumnName = Column->GetId().ToString();
			auto Widget = JavascriptPropertyTable->OnGenerateCustomCellWidget.Execute(WeakObject.Get(), ColumnName).Widget;
			if (Widget.IsValid())
			{
				return Widget.ToSharedRef();
			}
		}
	}
	return SNew(SHorizontalBox);
}

bool FJavascriptCustomCellPresenter::RequiresDropDown()
{
	return false;
}

TSharedRef< class SWidget > FJavascriptCustomCellPresenter::ConstructEditModeDropDownWidget()
{
	return SNullWidget::NullWidget;
}

TSharedRef<SWidget> FJavascriptCustomCellPresenter::ConstructEditModeCellWidget()
{
	return ConstructDisplayWidget();
}

TSharedRef< class SWidget > FJavascriptCustomCellPresenter::WidgetToFocusOnEdit()
{
	return SNullWidget::NullWidget;
}

FString FJavascriptCustomCellPresenter::GetValueAsString()
{
	return FString();
}

FText FJavascriptCustomCellPresenter::GetValueAsText()
{
	return FText();
}
