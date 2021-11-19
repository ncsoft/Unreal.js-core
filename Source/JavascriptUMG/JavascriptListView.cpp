#include "JavascriptListView.h"
#include "JavascriptContext.h"

UJavascriptListView::UJavascriptListView(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{	
}

TSharedRef<STableViewBase> UJavascriptListView::RebuildListWidget()
{
	SAssignNew(MyListView, SJavascriptListView)
		.SelectionMode(SelectionMode)
		.ListItemsSource(&Items)
		.ItemHeight(ItemHeight)
		.OnGenerateRow(BIND_UOBJECT_DELEGATE(SListView< UObject* >::FOnGenerateRow, HandleOnGenerateRow))
		.OnSelectionChanged_Lambda([this](UObject* Object, ESelectInfo::Type SelectInfo)
		{
			OnSelectionChanged(Object, SelectInfo);
		})
		.OnMouseButtonClick_Lambda([this](UObject* Object)
		{
			OnClick(Object);
		})
		.OnMouseButtonDoubleClick_Lambda([this](UObject* Object)
		{
			OnDoubleClick(Object);
		})
		.HeaderRow(GetHeaderRowWidget())
		.ExternalScrollbar(SNew(SScrollBar).Style(&ScrollBarStyle));

	if (OnContextMenuOpening.IsBound())
	{
		MyListView->SetOnContextMenuOpening(::FOnContextMenuOpening::CreateLambda([this]()
		{
			if (OnContextMenuOpening.IsBound())
			{
				auto Widget = OnContextMenuOpening.Execute(this);
				if (Widget)
				{
					return Widget->TakeWidget();
				}
			}
			return SNullWidget::NullWidget;
		}));
	}

	return MyListView.ToSharedRef();
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

void UJavascriptListView::ProcessEvent(UFunction* Function, void* Parms)
{
	if (JavascriptContext && JavascriptContext->CallProxyFunction(this, this, Function, Parms))
	{
		return;
	}

	Super::ProcessEvent(Function, Parms);
}

void UJavascriptListView::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyListView.Reset();
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
