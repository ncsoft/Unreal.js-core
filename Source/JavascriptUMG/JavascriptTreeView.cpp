#include "JavascriptUMG.h"
#include "JavascriptTreeView.h"
#include "JavascriptContext.h"

UJavascriptTreeView::UJavascriptTreeView(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{	
	bIsVariable = true;

	SelectionMode = ESelectionMode::Single;
}

TSharedPtr<SHeaderRow> UJavascriptTreeView::GetHeaderRowWidget()
{
	TSharedPtr<SHeaderRow> HeaderRowWidget;
	if (Columns.Num())
	{
		HeaderRowWidget = SNew(SHeaderRow);

		for (auto& Column : Columns)
		{
			if (!Column.Widget)
			{
				if (OnGenerateRowEvent.IsBound())
				{
					Column.Widget = OnGenerateRowEvent.Execute(nullptr, Column.Id, this);
				}
			}

			if (!Column.Widget)
			{
				continue;
			}

			HeaderRowWidget->AddColumn(
				SHeaderRow::Column(Column.Id)
				.FillWidth(Column.Width)
				[
					Column.Widget->TakeWidget()
				]
			);
		}
	}
	return HeaderRowWidget;
}

TSharedRef<SWidget> UJavascriptTreeView::RebuildWidget()
{
	MyTreeView = SNew(STreeView< UObject* >)
		.SelectionMode(SelectionMode)
		.TreeItemsSource(&Items)
		.OnGenerateRow(BIND_UOBJECT_DELEGATE(STreeView< UObject* >::FOnGenerateRow, HandleOnGenerateRow))
		.OnGetChildren(BIND_UOBJECT_DELEGATE(STreeView< UObject* >::FOnGetChildren, HandleOnGetChildren))
		.OnSelectionChanged_Lambda([this](UObject* Object, ESelectInfo::Type SelectInfo) {
			OnSelectionChanged(Object, SelectInfo);
		})
		.OnMouseButtonDoubleClick_Lambda([this](UObject* Object) {
			OnDoubleClick(Object);
		})
		.HeaderRow(GetHeaderRowWidget())
		//.OnContextMenuOpening(this, &SSocketManager::OnContextMenuOpening)
		//.OnItemScrolledIntoView(this, &SSocketManager::OnItemScrolledIntoView)
		//	.HeaderRow
		//	(
		//		SNew(SHeaderRow)
		//		.Visibility(EVisibility::Collapsed)
		//		+ SHeaderRow::Column(TEXT("Socket"))
		//	);
		;

	return BuildDesignTimeWidget(MyTreeView.ToSharedRef());
}

void UJavascriptTreeView::ProcessEvent(UFunction* Function, void* Parms)
{
	if (JavascriptContext && JavascriptContext->CallProxyFunction(this, this, Function, Parms))
	{
		return;
	}

	Super::ProcessEvent(Function, Parms);
}

void UJavascriptTreeView::RequestTreeRefresh()
{
	if (MyTreeView.IsValid())
	{
		MyTreeView->RequestTreeRefresh();
	}	
}

/**
* Implements a row widget for the session console log.
*/
class SJavascriptTableRow
	: public SMultiColumnTableRow<UObject*>
{
public:
	SLATE_BEGIN_ARGS(SJavascriptTableRow) { }
		SLATE_ARGUMENT(UObject*, Object)
		SLATE_ARGUMENT(UJavascriptTreeView*, TreeView)
	SLATE_END_ARGS()

public:

	/**
	* Constructs the widget.
	*
	* @param InArgs The construction arguments.
	* @param InOwnerTableView The table view that owns this row.
	*/
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
	{
		Object = InArgs._Object;
		TreeView = InArgs._TreeView;

		SMultiColumnTableRow<UObject*>::Construct(FSuperRowType::FArguments(), InOwnerTableView);
	}

public:

	// SMultiColumnTableRow interface

	BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override
	{
		auto ColumnWidget = SNullWidget::NullWidget;

		if (TreeView->OnGenerateRowEvent.IsBound())
		{
			UWidget* Widget = TreeView->OnGenerateRowEvent.Execute(Object, ColumnName, TreeView);

			if (Widget)
			{
				ColumnWidget = Widget->TakeWidget();
			}
		}

		if (TreeView->IsA(UJavascriptTreeView::StaticClass()) && ColumnName == TreeView->Columns[0].Id)
		{
			// The first column gets the tree expansion arrow for this row
			return
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SExpanderArrow, SharedThis(this))
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					ColumnWidget
				];
		}
		else
		{
			return ColumnWidget;
		}
	}
	END_SLATE_FUNCTION_BUILD_OPTIMIZATION

private:
	UObject* Object;
	UJavascriptTreeView* TreeView;
};

TSharedRef<ITableRow> UJavascriptTreeView::HandleOnGenerateRow(UObject* Item, const TSharedRef< STableViewBase >& OwnerTable)
{
	// Call the user's delegate to see if they want to generate a custom widget bound to the data source.
	if (OnGenerateRowEvent.IsBound())
	{
		if (Columns.Num())
		{
			return SNew(SJavascriptTableRow, OwnerTable).Object(Item).TreeView(this);
		}
		else
		{
			UWidget* Widget = OnGenerateRowEvent.Execute(Item, FName(), this);
			if (Widget != NULL)
			{
				return SNew(STableRow< UObject* >, OwnerTable)
					[
						Widget->TakeWidget()
					];
			}
		}		
	}

	// If a row wasn't generated just create the default one, a simple text block of the item's name.
	return SNew(STableRow< UObject* >, OwnerTable)
		[
			SNew(STextBlock).Text(Item ? FText::FromString(Item->GetName()) : FText::FromName(FName()))
		];
}


void UJavascriptTreeView::HandleOnGetChildren(UObject* Item, TArray<UObject*>& OutChildItems)
{
	if (OnGetChildren.IsBound())
	{
		Children.Empty();

		OnGetChildren.Execute(Item,this);
		
		OutChildItems.Append(Children);

		Children.Empty();
	}
}


void UJavascriptTreeView::GetSelectedItems(TArray<UObject*>& OutItems)
{
	if (MyTreeView.IsValid())
	{
		OutItems = MyTreeView->GetSelectedItems();
	}
}