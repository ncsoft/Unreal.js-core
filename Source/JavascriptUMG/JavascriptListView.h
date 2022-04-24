#pragma once

#include "Widgets/Views/SListView.h"
#include "Components/ListViewBase.h"
#include "JavascriptListView.generated.h"

class SJavascriptListView : public SListView<UObject*>
{
public:
	SJavascriptListView()
	: SListView<UObject*>()
	{
	}

public:
	void SetOnContextMenuOpening(FOnContextMenuOpening InOnContextMenuOpening)
	{
		OnContextMenuOpening = InOnContextMenuOpening;
	}
};

class UJavascriptContext;

/** Delegate for constructing a UWidget based on a UObject */
DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(UWidget*, FOnGenerateRowSignature, UObject*, Object, UJavascriptListView*, Instance);

/** Delegate for constructing a UWidget based on a UObject */
DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(UWidget*, FOnContextMenuOpeningSignature, UJavascriptListView*, Instance);

/**
* Allows thousands of items to be displayed in a list.  Generates widgets dynamically for each item.
*/
UCLASS(Experimental)
class JAVASCRIPTUMG_API UJavascriptListView : public UListViewBase
{
	GENERATED_UCLASS_BODY()

public:
	// UObject interface.
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);

	// UObject interface
	virtual void ProcessEvent(UFunction* Function, void* Parms) override;
	// End of UObject interface

	//~ Begin UVisual Interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ End UVisual Interface

	// UListViewBase interface
	virtual TSharedRef<STableViewBase> RebuildListWidget() override;
	// End of UListViewBase interface

	TSharedRef<ITableRow> HandleOnGenerateRow(UObject* Item, const TSharedRef< STableViewBase >& OwnerTable);

	virtual TSharedRef<ITableRow> CreateDefaultRow(UObject* Item, const TSharedRef<STableViewBase>& OwnerTable);
	virtual TSharedRef<ITableRow> CreateItemRow(UWidget* Widget, const TSharedRef<STableViewBase>& OwnerTable);

	/** Refreshes the list */
	UFUNCTION(BlueprintCallable, Category = "Behavior")
	void SetItems(const TArray<UObject*>& InListItems);

	/** Event fired when a tutorial stage ends */
	UFUNCTION(BlueprintImplementableEvent, Category = "Javascript")
	void OnSelectionChanged(UObject* Object, ESelectInfo::Type Type);

	UFUNCTION(BlueprintCallable, Category = "Javascript")
	void SetItemSelection(TArray<UObject*> MultiSelectedItems, bool bIsSelected);

	UFUNCTION(BlueprintNativeEvent, Category = "Behavior")
	void SetSelection(UObject* SoleSelectedItem);

	UFUNCTION(BlueprintNativeEvent, Category = "Behavior")
	bool GetSelectedItems(TArray<UObject*>& OutItems);

	UFUNCTION(BlueprintCallable, Category = "Javascript")
	void ClearSelection();

	UFUNCTION(BlueprintCallable, Category = "Javascript")
	void RequestNavigateToItem(UObject* Item);

	/** Event fired when a tutorial stage ends */
	UFUNCTION(BlueprintImplementableEvent, Category = "Javascript")
	void OnClick(UObject* Object);

	/** Event fired when a tutorial stage ends */
	UFUNCTION(BlueprintImplementableEvent, Category = "Javascript")
	void OnDoubleClick(UObject* Object);

protected:
	TSharedPtr<SJavascriptListView> MyListView;

	TMultiMap<UObject*, TWeakPtr<SWidget>> CachedRows;

	UPROPERTY(BlueprintReadWrite, Category = Javascript)
	UJavascriptContext* JavascriptContext;

	/** The list of items to generate widgets for */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ListView)
	TArray<UObject*> ListItems;

	/** Called when a widget needs to be generated */
	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGenerateRowSignature OnGenerateRow;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnContextMenuOpeningSignature OnContextMenuOpening;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ListView)
	FTableViewStyle WidgetStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ListView)
	FScrollBarStyle ScrollBarStyle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ListView)
	TEnumAsByte<EOrientation> Orientation = EOrientation::Orient_Vertical;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ListView)
	TEnumAsByte<ESelectionMode::Type> SelectionMode = ESelectionMode::Single;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ListView)
	EConsumeMouseWheel ConsumeMouseWheel = EConsumeMouseWheel::WhenScrollingPossible;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ListView)
	bool bClearSelectionOnClick = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ListView)
	bool bIsFocusable = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ListView)
	bool bReturnFocusToSelection = false;
};