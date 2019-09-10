#include "JavascriptGeneratedClass.h"
#include "JavascriptGeneratedClass_Native.h"
#include "UObject/UnrealType.h"

void UJavascriptGeneratedClass::InitPropertiesFromCustomList(uint8* DataPtr, const uint8* DefaultDataPtr)
{
	if (const FCustomPropertyListNode* CustomPropertyList = GetCustomPropertyListForPostConstruction())
	{
		UBlueprintGeneratedClass::InitPropertiesFromCustomList(CustomPropertyList, this, DataPtr, DefaultDataPtr);
	}
}

void UJavascriptGeneratedClass::PostInitInstance(UObject* InObj)
{
	Super::PostInitInstance(InObj);

	auto Context = JavascriptContext.Pin();
	if (Context.IsValid())
	{
		Context->CallProxyFunction(this, InObj, TEXT("ctor"), nullptr);
	}
}

void UJavascriptGeneratedClass_Native::PostInitInstance(UObject* InObj)
{
	Super::PostInitInstance(InObj);

	auto Context = JavascriptContext.Pin();
	if (Context.IsValid())
	{
		Context->CallProxyFunction(this, InObj, TEXT("ctor"), nullptr);
	}
}