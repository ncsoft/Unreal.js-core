#include "JavascriptPropertyTable.h"
#include "IPropertyTableColumn.h"
#include "PropertyEditorModule.h"

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
			for (TFieldIterator<UProperty> PropertyIterator(Class); PropertyIterator; ++PropertyIterator)
			{
				TWeakObjectPtr< UProperty > Property = *PropertyIterator;
				if (!Property->HasMetaData(TEXT("Hidden")))
				{
					PropertyTable->AddColumn(Property);
				}
			}
		}

		return PropertyEditorModule.CreatePropertyTableWidget(PropertyTable.ToSharedRef());
	}
}

#undef LOCTEXT_NAMESPACE
