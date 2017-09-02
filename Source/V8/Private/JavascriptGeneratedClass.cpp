#include "JavascriptGeneratedClass.h"
#include "UObject/UnrealType.h"

void UJavascriptGeneratedClass::InitPropertiesFromCustomList(uint8* DataPtr, const uint8* DefaultDataPtr)
{
	if (const FCustomPropertyListNode* CustomPropertyList = GetCustomPropertyListForPostConstruction())
	{
		UBlueprintGeneratedClass::InitPropertiesFromCustomList(CustomPropertyList, this, DataPtr, DefaultDataPtr);
	}
}