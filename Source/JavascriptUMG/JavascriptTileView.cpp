#include "JavascriptTileView.h"
#include "JavascriptContext.h"

UJavascriptTileView::UJavascriptTileView(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{	
}

TSharedRef<STableViewBase> UJavascriptTileView::RebuildListWidget()
{
	MyTileView = SNew(STileView< UObject* >)
		.SelectionMode(SelectionMode)
		.ListItemsSource(&TileListItems)
		.ItemHeight(EntryHeight)
		.OnSelectionChanged_Lambda([this](UObject* Object, ESelectInfo::Type SelectInfo){
			OnSelectionChanged(Object, SelectInfo);
		})
		.OnMouseButtonDoubleClick_Lambda([this](UObject* Object){
			OnDoubleClick(Object);
		})
		//.OnContextMenuOpening(this, &SSocketManager::OnContextMenuOpening)
		//.OnItemScrolledIntoView(this, &SSocketManager::OnItemScrolledIntoView)
		//	.HeaderRow
		//	(
		//		SNew(SHeaderRow)
		//		.Visibility(EVisibility::Collapsed)
		//		+ SHeaderRow::Column(TEXT("Socket"))
		//	);
		;

	return MyTileView.ToSharedRef();
}

void UJavascriptTileView::ProcessEvent(UFunction* Function, void* Parms)
{
	if (JavascriptContext && JavascriptContext->CallProxyFunction(this, this, Function, Parms))
	{
		return;
	}

	Super::ProcessEvent(Function, Parms);
}
