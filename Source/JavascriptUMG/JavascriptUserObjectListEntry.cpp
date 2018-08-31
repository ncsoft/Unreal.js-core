#include "JavascriptUserObjectListEntry.h"

UJavascriptUserObjectListEntry::UJavascriptUserObjectListEntry(const FObjectInitializer& Initializer)
	: Super(Initializer)
{
}

UObject* UJavascriptUserObjectListEntry::GetListItemObject_Implementation() const
{
	return Item;
}