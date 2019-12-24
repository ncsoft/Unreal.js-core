#include "JavascriptInvalidCellPresenter.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "Widgets/SBoxPanel.h"

FJavascriptInvalidCellPresenter::FJavascriptInvalidCellPresenter(UJavascriptPropertyTable* InJavascriptPropertyTable)
	: JavascriptPropertyTable(InJavascriptPropertyTable)
{
}

TSharedRef< class SWidget > FJavascriptInvalidCellPresenter::ConstructDisplayWidget()
{
	if (IsValid(JavascriptPropertyTable) && JavascriptPropertyTable->OnGenerateInvalidCellWidget.IsBound())
	{
		auto Widget = JavascriptPropertyTable->OnGenerateInvalidCellWidget.Execute().Widget;
		if (Widget.IsValid())
		{
			return Widget.ToSharedRef();
		}
	}

	return SNew(SHorizontalBox);
}

bool FJavascriptInvalidCellPresenter::RequiresDropDown()
{
	return false;
}

TSharedRef< class SWidget > FJavascriptInvalidCellPresenter::ConstructEditModeDropDownWidget()
{
	return SNullWidget::NullWidget;
}

TSharedRef<SWidget> FJavascriptInvalidCellPresenter::ConstructEditModeCellWidget()
{
	return ConstructDisplayWidget();
}

TSharedRef< class SWidget > FJavascriptInvalidCellPresenter::WidgetToFocusOnEdit()
{
	return SNullWidget::NullWidget;
}

FString FJavascriptInvalidCellPresenter::GetValueAsString()
{
	return FString();
}

FText FJavascriptInvalidCellPresenter::GetValueAsText()
{
	return FText();
}
