#pragma once

#include "JavascriptTreeView.h"
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

/**
* Allows thousands of items to be displayed in a list.  Generates widgets dynamically for each item.
*/
UCLASS(Experimental)
class JAVASCRIPTUMG_API UJavascriptListView : public UJavascriptTreeView
{
	GENERATED_UCLASS_BODY()

public:	
	/** The height of each widget */
	UPROPERTY(EditAnywhere, Category = Content)
	float ItemHeight;

	/** Event fired when a tutorial stage ends */
	UFUNCTION(BlueprintImplementableEvent, Category = "Javascript")
	void OnClick(UObject* Object);

	/** Refreshes the list */
	UFUNCTION(BlueprintCallable, Category = "Behavior")
	void RequestListRefresh();

	virtual void SetSelection_Implementation(UObject* SoleSelectedItem) override;
	virtual bool GetSelectedItems_Implementation(TArray<UObject*>& OutItems) override;

	// UWidget interface
	virtual TSharedRef<STableViewBase> RebuildListWidget() override;
	// End of UWidget interface

	// UObject interface
	virtual void ProcessEvent(UFunction* Function, void* Parms) override;
	// End of UObject interface

	//~ Begin UVisual Interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	//~ End UVisual Interface

	TSharedPtr<SJavascriptListView> MyListView;

	// Overriden functions from UJavascriptTreeView
	virtual TSharedRef<ITableRow> CreateItemRow(UWidget* Widget, const TSharedRef<STableViewBase>& OwnerTable) override;
	virtual TSharedRef<ITableRow> CreateDefaultRow(UObject* Item, const TSharedRef<STableViewBase>& OwnerTable) override;
};
