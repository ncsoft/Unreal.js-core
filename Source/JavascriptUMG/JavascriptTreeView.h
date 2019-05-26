#pragma once

#include "Widgets/Views/STreeView.h"
#include "Components/ListViewBase.h"
#include "JavascriptTreeView.generated.h"

class SJavascriptTreeView : public STreeView<UObject*>
{
public:
	SJavascriptTreeView()
		: STreeView<UObject*>()
	{
	}

public:
	void SetDoubleClickItemSelection(UObject* TheItem, bool bShouldBeSelected)
	{
		if (TableViewMode != ETableViewMode::Tree)
		{
			return;
		}

		if (bShouldBeSelected)
		{
			DoubleClickedItems.Add(TheItem);
		}
		else
		{
			DoubleClickedItems.Remove(TheItem);
		}
	}

	void ClearDoubleClickSelection()
	{
		if (TableViewMode != ETableViewMode::Tree)
		{
			return;
		}

		DoubleClickedItems.Empty();
	}

	bool IsDoubleClickSelection(UObject* TheItem)
	{
		if (TableViewMode != ETableViewMode::Tree)
		{
			return false;
		}

		if (TheItem == nullptr)
		{
			return false;
		}

		return (DoubleClickedItems.Find(TheItem) != nullptr);
	}

	TArray<UObject*> GetDoubleClickedItems() const
	{
		TArray<UObject*> SelectedItemArray;

		if (TableViewMode != ETableViewMode::Tree)
		{
			return SelectedItemArray;
		}

		SelectedItemArray.Empty(DoubleClickedItems.Num());
		for (typename TSet<UObject*>::TConstIterator SelectedItemIt(DoubleClickedItems); SelectedItemIt; ++SelectedItemIt)
		{
			SelectedItemArray.Add(*SelectedItemIt);
		}
		return SelectedItemArray;
	}

protected:
	TSet<UObject*> DoubleClickedItems;
};

class UJavascriptContext;

USTRUCT(BlueprintType)
struct FJavascriptColumn
{
	GENERATED_BODY()

	UPROPERTY()
	FString Id;

	UPROPERTY()
	float Width;

	UPROPERTY(Transient)
	UWidget* Widget;
};

/**
* Allows thousands of items to be displayed in a list.  Generates widgets dynamically for each item.
*/
UCLASS(Experimental)
class JAVASCRIPTUMG_API UJavascriptTreeView : public UListViewBase
{
	GENERATED_UCLASS_BODY()

public:	
	/** Delegate for constructing a UWidget based on a UObject */
	DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnGetChildrenUObject, UObject*, Item, UJavascriptTreeView*, Instance);

	/** Delegate for constructing a UWidget based on a UObject */
	DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnExpansionChanged, UObject*, Item, bool, bExpanded, UJavascriptTreeView*, Instance);

	/** Delegate for constructing a UWidget based on a UObject */
	DECLARE_DYNAMIC_DELEGATE_RetVal_ThreeParams(UWidget*, FOnGenerateRow, UObject*, Object, FName, Id, UJavascriptTreeView*, Instance);

	/** Delegate for constructing a UWidget based on a UObject */
	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(UWidget*, FOnContextMenuOpening, UJavascriptTreeView*, Instance);

	/** Called when a widget needs to be generated */
	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGenerateRow OnGenerateRowEvent;
	
	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnExpansionChanged OnExpansionChanged;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnContextMenuOpening OnContextMenuOpening;

	/** Called when a widget needs to be generated */
	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGetChildrenUObject OnGetChildren;	

	UPROPERTY(BlueprintReadWrite, Category = "Javascript")
	UJavascriptContext* JavascriptContext;

	/** The list of items to generate widgets for */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Content)
	TArray<UObject*> Items;

	UPROPERTY(EditAnywhere, BlueprintInternalUseOnly, Category = "Javascript")
	FHeaderRowStyle HeaderRowStyle;

	UPROPERTY(EditAnywhere, BlueprintInternalUseOnly, Category = "Javascript")
	FTableRowStyle TableRowStyle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Javascript")
	FScrollBarStyle ScrollBarStyle;

	/** The selection method for the list */
	UPROPERTY(EditAnywhere, Category = Content)
	TEnumAsByte<ESelectionMode::Type> SelectionMode;

	UPROPERTY(BlueprintReadWrite, Transient, Category = "Javascript")
	mutable TArray<UObject*> Children;

	UPROPERTY(BlueprintReadWrite, Transient, Category = "Javascript")
	TArray<FJavascriptColumn> Columns;

	/** Refreshes the list */
	UFUNCTION(BlueprintCallable, Category = "Behavior")
	void RequestTreeRefresh();
	
	/** Event fired when a tutorial stage ends */
	UFUNCTION(BlueprintImplementableEvent, Category = "Javascript")
	void OnDoubleClick(UObject* Object);

	/** Event fired when a tutorial stage ends */
	UFUNCTION(BlueprintImplementableEvent, Category = "Javascript")
	void OnSelectionChanged(UObject* Object, ESelectInfo::Type Type);

	UFUNCTION(BlueprintCallable, Category = "Javascript")
	void SetItemExpansion(UObject* InItem, bool InShouldExpandItem);

	UFUNCTION(BlueprintCallable, Category = "Javascript")
	void SetSingleExpandedItem(UObject* InItem);

	UFUNCTION(BlueprintCallable, Category = "Javascript")
	bool IsItemExpanded(UObject* InItem);

	UFUNCTION(BlueprintNativeEvent, Category = "Behavior")
	void SetSelection(UObject* SoleSelectedItem);

	UFUNCTION(BlueprintNativeEvent, Category = "Behavior")
	bool GetSelectedItems(TArray<UObject*>& OutItems);

	UFUNCTION(BlueprintCallable, Category = "Javascript")
	void ClearDoubleClickSelection();

	UFUNCTION(BlueprintCallable, Category = "Javascript")
	void SetDoubleClickSelection(UObject* SelectedItem);

	UFUNCTION(BlueprintCallable, Category = "Javascript")
	bool IsDoubleClickSelection(UObject* SelectedItem);

	UFUNCTION(BlueprintCallable, Category = "Javascript")
	void GetDoubleClickedItems(TArray<UObject*>& OutItems);

	TSharedRef<ITableRow> HandleOnGenerateRow(UObject* Item, const TSharedRef< STableViewBase >& OwnerTable);

	void HandleOnGetChildren(UObject* Item, TArray<UObject*>& OutChildItems);
	void HandleOnExpansionChanged(UObject* Item, bool bExpanded);

	// UWidget interface
	virtual TSharedRef<STableViewBase> RebuildListWidget() override;
	// End of UWidget interface

	// UObject interface
	virtual void ProcessEvent(UFunction* Function, void* Parms) override;
	// End of UObject interface

	//~ Begin UVisual Interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ End UVisual Interface

	TSharedPtr<SJavascriptTreeView> MyTreeView;

	TSharedPtr<SHeaderRow> GetHeaderRowWidget();

	UPROPERTY(Transient)
	TArray<UWidget*> ColumnWidgets;

	// UObject interface.
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);

	TMultiMap<UObject*, TWeakPtr<SWidget>> CachedRows;


	// Virtual functions for derived classes
	virtual TSharedRef<ITableRow> CreateTableRow(UObject* Item, const TSharedRef<STableViewBase>& OwnerTable);
	virtual TSharedRef<ITableRow> CreateItemRow(UWidget* Widget, const TSharedRef<STableViewBase>& OwnerTable);
	virtual TSharedRef<ITableRow> CreateDefaultRow(UObject* Item, const TSharedRef<STableViewBase>& OwnerTable);
};
