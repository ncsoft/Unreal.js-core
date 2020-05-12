#include "JavascriptPropertyTable.h"
#include "IPropertyTableColumn.h"
#include "IPropertyTableCustomColumn.h"
#include "PropertyEditorModule.h"
#include "JavascriptCustomColumn.h"

#define LOCTEXT_NAMESPACE "JavascriptPropertyTable"

UJavascriptPropertyTable::UJavascriptPropertyTable(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UJavascriptPropertyTable::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	PropertyTable.Reset();
}


void UJavascriptPropertyTable::SetEditingObjects(TArray<UObject*> InEditingObjects)
{
	EditingObjects = InEditingObjects;

	if (PropertyTable.IsValid())
	{
		TSet<TSharedRef<IPropertyTableRow>> empty;
		PropertyTable->SetSelectedRows(empty);

		PropertyTable->SetObjects(EditingObjects);

		if (EditingObjects.Num() > 0)
		{
			UObject* Object = EditingObjects[0];
			UClass* Class = Object->GetClass();
			for (TFieldIterator<FProperty> PropertyIterator(Class); PropertyIterator; ++PropertyIterator)
			{
				TWeakFieldPtr< FProperty > Property = *PropertyIterator;
				if (!Property->HasMetaData(TEXT("Hidden")))
				{
					PropertyTable->AddColumn(Property);
				}
			}
		}

		for(auto object : EditingObjects)
		{
			PropertyTable->AddRow(object);
		}
		
		PropertyTable->RequestRefresh();
	}
}

TArray<UObject*> UJavascriptPropertyTable::GetSelectedTableObjects()
{
	TArray<UObject*> objects;

	if (PropertyTable.IsValid())
	{
		TArray<TWeakObjectPtr<UObject>> SelectedObjects;
		PropertyTable->GetSelectedTableObjects(SelectedObjects);

		for (auto object : SelectedObjects)
		{
			objects.Add(object.Get());
		}
	}

	return objects;
}

TSharedRef<SWidget> UJavascriptPropertyTable::RebuildWidget()
{
	if (IsDesignTime())
	{
		return RebuildDesignWidget(SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("JavascriptPropertyTable", "JavascriptPropertyTable"))
			]);
	}
	else
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyTable = PropertyEditorModule.CreatePropertyTable();
		PropertyTable->SetShowObjectName(false);
		PropertyTable->SetIsUserAllowedToChangeRoot(false);
		PropertyTable->SetObjects(EditingObjects);

		if (EditingObjects.Num() > 0)
		{
			UObject* Object = EditingObjects[0];
			UClass* Class = Object->GetClass();
			for (TFieldIterator<FProperty> PropertyIterator(Class); PropertyIterator; ++PropertyIterator)
			{
				TWeakFieldPtr< FProperty > Property = *PropertyIterator;
				if (!Property->HasMetaData(TEXT("Hidden")))
				{
					PropertyTable->AddColumn(Property);
				}
			}
		}

		if (bUseCustomColumns)
		{
			TArray< TSharedRef<class IPropertyTableCustomColumn>> CustomColumns;
			CustomColumns.Add(MakeShareable(new FJavascriptCustomColumn(this)));

			return PropertyEditorModule.CreatePropertyTableWidget(PropertyTable.ToSharedRef(), CustomColumns);
		}
		else
		{
			return PropertyEditorModule.CreatePropertyTableWidget(PropertyTable.ToSharedRef());			
		}
	}
}

#undef LOCTEXT_NAMESPACE
