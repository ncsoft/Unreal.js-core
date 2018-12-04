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
	auto ListView = MyListView.Pin();
	if (ListView.IsValid())
	{
		ListView->RequestListRefresh();
	}	
}

bool UJavascriptListView::GetSelectedItems_Implementation(TArray<UObject*>& OutItems)
{
	auto ListView = MyListView.Pin();
	if (ListView.IsValid())
	{
		return ListView->GetSelectedItems(OutItems) > 0;
	}
	return false;
}

void UJavascriptListView::SetSelection_Implementation(UObject* SoleSelectedItem)
{
	auto ListView = MyListView.Pin();
	if (ListView.IsValid())
	{
		ListView->SetSelection(SoleSelectedItem);
	}
}

TSharedRef<ITableRow> UJavascriptListView::CreateItemRow(UWidget* Widget, const TSharedRef<STableViewBase>& OwnerTable)
{
	auto GeneratedWidget = Widget->TakeWidget();
	CachedRows.Add(Widget, GeneratedWidget);
	return SNew(STableRow<UObject*>, OwnerTable)[GeneratedWidget];
}

TSharedRef<ITableRow> UJavascriptListView::CreateDefaultRow(UObject* Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<UObject*>, OwnerTable)
		[
			SNew(STextBlock).Text(Item ? FText::FromString(Item->GetName()) : FText::FromName(FName()))
		];
}