#include "JavascriptListView.h"
#include "JavascriptContext.h"

UJavascriptListView::UJavascriptListView(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{	
}

TSharedRef<SWidget> UJavascriptListView::RebuildWidget()
{
	TSharedRef<SScrollBar> ExternalScrollbar = SNew(SScrollBar).Style(&ScrollBarStyle);
	return StaticCastSharedRef<SWidget>
	(
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.FillWidth(1)
		[
			SAssignNew(MyListView, SListView< UObject* >)
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
			.ExternalScrollbar(ExternalScrollbar)
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
			})
			//.OnContextMenuOpening(this, &SSocketManager::OnContextMenuOpening)
			//.OnItemScrolledIntoView(this, &SSocketManager::OnItemScrolledIntoView)
			//	.HeaderRow
			//	(
			//		SNew(SHeaderRow)
			//		.Visibility(EVisibility::Collapsed)
			//		+ SHeaderRow::Column(TEXT("Socket"))
			//	);
		]
		+SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.WidthOverride(FOptionalSize(16))
			[
				ExternalScrollbar
			]
		]
	);
}

void UJavascriptListView::RequestListRefresh()
{
	if (MyListView.IsValid())
	{
		MyListView->RequestListRefresh();
	}	
}

void UJavascriptListView::GetSelectedItems(TArray<UObject*>& OutItems)
{
	if (MyListView.IsValid())
	{
		OutItems = MyListView->GetSelectedItems();
	}
}

void UJavascriptListView::SetSelection(UObject* SoleSelectedItem)
{
	if (MyListView.IsValid())
	{
		MyListView->SetSelection(SoleSelectedItem);
	}
}
