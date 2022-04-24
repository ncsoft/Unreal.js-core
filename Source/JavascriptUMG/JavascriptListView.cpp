#include "JavascriptListView.h"
#include "JavascriptContext.h"

FTableRowStyle _TableRowStyle;

UJavascriptListView::UJavascriptListView(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	WidgetStyle = FUMGCoreStyle::Get().GetWidgetStyle<FTableViewStyle>("ListView");
	ScrollBarStyle = FUMGCoreStyle::Get().GetWidgetStyle<FScrollBarStyle>("Scrollbar");

	// 清除RowItem的默认Style
	FSlateBrush Brush;
	Brush.DrawAs = ESlateBrushDrawType::NoDrawType;
	_TableRowStyle.ActiveBrush = Brush;
	_TableRowStyle.ActiveHighlightedBrush = Brush;
	_TableRowStyle.ActiveHoveredBrush = Brush;
	_TableRowStyle.DropIndicator_Above = Brush;
	_TableRowStyle.DropIndicator_Below = Brush;
	_TableRowStyle.DropIndicator_Onto = Brush;
	_TableRowStyle.EvenRowBackgroundBrush = Brush;
	_TableRowStyle.EvenRowBackgroundHoveredBrush = Brush;
	_TableRowStyle.InactiveBrush = Brush;
	_TableRowStyle.InactiveHighlightedBrush = Brush;
	_TableRowStyle.InactiveHoveredBrush = Brush;
	_TableRowStyle.OddRowBackgroundBrush = Brush;
	_TableRowStyle.OddRowBackgroundHoveredBrush = Brush;
	_TableRowStyle.ParentRowBackgroundBrush = Brush;
	_TableRowStyle.ParentRowBackgroundHoveredBrush = Brush;
	_TableRowStyle.SelectorFocusedBrush = Brush;
}

void UJavascriptListView::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	auto This = static_cast<UJavascriptListView*>(InThis);

	if (This->MyListView.IsValid())
	{
		for (auto It = This->CachedRows.CreateIterator(); It; ++It)
		{
			auto Key = It->Key;
			auto Value = It->Value;

			if (Value.IsValid())
			{
				Collector.AddReferencedObject(Key, This);
			}
			else
			{
				It.RemoveCurrent();
			}
		}
	}
	else
	{
		This->CachedRows.Empty();
	}

	Super::AddReferencedObjects(This, Collector);
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

TSharedRef<STableViewBase> UJavascriptListView::RebuildListWidget()
{
	SAssignNew(MyListView, SJavascriptListView)
		.ListViewStyle(&WidgetStyle)
		.ScrollBarStyle(&ScrollBarStyle)
		.Orientation(Orientation)
		.SelectionMode(SelectionMode)
		.ConsumeMouseWheel(ConsumeMouseWheel)
		.ClearSelectionOnClick(bClearSelectionOnClick)
		.IsFocusable(bIsFocusable)
		.ReturnFocusToSelection(bReturnFocusToSelection)
		.ListItemsSource(&ListItems)
		.OnGenerateRow(BIND_UOBJECT_DELEGATE(SListView<UObject*>::FOnGenerateRow, HandleOnGenerateRow))
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
			});

	// 支持右键打开菜单，但目前右键菜单会默认带有一个背景层
	if (OnContextMenuOpening.IsBound())
	{
		MyListView->SetOnContextMenuOpening(FOnContextMenuOpening::CreateLambda([this]()
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

TSharedRef<ITableRow> UJavascriptListView::HandleOnGenerateRow(UObject* Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	// Call the user's delegate to see if they want to generate a custom widget bound to the data source.
	if (OnGenerateRow.IsBound())
	{
		UWidget* Widget = OnGenerateRow.Execute(Item, this);
		if (Widget != NULL)
		{
			return CreateItemRow(Widget, OwnerTable);
		}
	}

	// If a row wasn't generated just create the default one, a simple text block of the item's name.
	return CreateDefaultRow(Item, OwnerTable);
}

TSharedRef<ITableRow> UJavascriptListView::CreateDefaultRow(UObject* Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<UObject*>, OwnerTable)
		[
			SNew(STextBlock).Text(Item ? FText::FromString(Item->GetName()) : FText::FromName(FName()))
		]
	.Style(&_TableRowStyle);
}

TSharedRef<ITableRow> UJavascriptListView::CreateItemRow(UWidget* Widget, const TSharedRef<STableViewBase>& OwnerTable)
{
	auto GeneratedWidget = Widget->TakeWidget();
	CachedRows.Add(Widget, GeneratedWidget);
	return SNew(STableRow<UObject*>, OwnerTable)[GeneratedWidget]
		.Style(&_TableRowStyle);
}

void UJavascriptListView::SetItems(const TArray<UObject*>& InListItems)
{
	ListItems = InListItems;
	RequestRefresh();
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

void UJavascriptListView::SetItemSelection(TArray<UObject*> MultiSelectedItems, bool bIsSelected)
{
	if (MyListView.IsValid())
	{
		MyListView->SetItemSelection(MultiSelectedItems, bIsSelected);
	}
}

void UJavascriptListView::ClearSelection()
{
	if (MyListView.IsValid())
	{
		MyListView->ClearSelection();
	}
}

void UJavascriptListView::RequestNavigateToItem(UObject* Item)
{
	if (MyListView.IsValid())
	{
		MyListView->RequestNavigateToItem(Item);
	}
}