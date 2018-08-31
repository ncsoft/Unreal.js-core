#include "JavascriptListView.h"
#include "JavascriptContext.h"

UJavascriptListView::UJavascriptListView(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{	
}

TSharedRef<STableViewBase> UJavascriptListView::RebuildListWidget()
{
	return SAssignNew(MyListView, SListView< UObject* >)
		.SelectionMode(SelectionMode)
		.ListItemsSource(&Items)
		.ItemHeight(ItemHeight)
		.OnGenerateRow(BIND_UOBJECT_DELEGATE(SListView< UObject* >::FOnGenerateRow, HandleOnGenerateRow))
		.OnSelectionChanged_Lambda([this](UObject* Object, ESelectInfo::Type SelectInfo) {
		OnSelectionChanged(Object, SelectInfo);
		})
		.OnMouseButtonClick_Lambda([this](UObject* Object) {
			OnClick(Object);
		})
		.OnMouseButtonDoubleClick_Lambda([this](UObject* Object) {
			OnDoubleClick(Object);
		})
		.HeaderRow(GetHeaderRowWidget())
		.ExternalScrollbar(SNew(SScrollBar).Style(&ScrollBarStyle))
		.OnContextMenuOpening_Lambda([this]() {
			if (OnContextMenuOpening.IsBound())
			{
				auto Widget = OnContextMenuOpening.Execute(this);
				if (Widget)
				{
					return Widget->TakeWidget();
				}
			}
			return SNullWidget::NullWidget;
		});
		//.OnContextMenuOpening(this, &SSocketManager::OnContextMenuOpening)
		//.OnItemScrolledIntoView(this, &SSocketManager::OnItemScrolledIntoView)
		//	.HeaderRow
		//	(
		//		SNew(SHeaderRow)
		//		.Visibility(EVisibility::Collapsed)
		//		+ SHeaderRow::Column(TEXT("Socket"))
		//	);
}

void UJavascriptListView::RequestListRefresh()
{
	if (MyListView.IsValid())
	{
		MyListView->RequestListRefresh();
	}	
}

bool UJavascriptListView::GetSelectedItems_Implementation(TArray<UObject*>& OutItems)
{
	if (MyListView.IsValid())
	{
		return MyListView->GetSelectedItems(OutItems) > 0;
	}
	return false;
}

void UJavascriptListView::SetSelection_Implementation(UObject* SoleSelectedItem)
{
	if (MyListView.IsValid())
	{
		MyListView->SetSelection(SoleSelectedItem);
	}
}
