#include "JavascriptGeneratedClass.h"
#include "JavascriptGeneratedClass_Native.h"
#include "JavascriptContext_Private.h"
#include "UObject/UnrealType.h"

void UJavascriptGeneratedClass::InitPropertiesFromCustomList(uint8* DataPtr, const uint8* DefaultDataPtr)
{
	if (const FCustomPropertyListNode* CustomPropertyList = GetCustomPropertyListForPostConstruction())
	{
		UBlueprintGeneratedClass::InitPropertiesFromCustomList(CustomPropertyList, this, DataPtr, DefaultDataPtr);
	}
}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
void UJavascriptGeneratedClass::PostInitInstance(UObject* InObj, FObjectInstancingGraph* InstanceGraph)
{
	Super::PostInitInstance(InObj, InstanceGraph);
#else
void UJavascriptGeneratedClass::PostInitInstance(UObject* InObj)
{
	Super::PostInitInstance(InObj);
#endif

	auto Context = JavascriptContext.Pin();
	if (Context.IsValid())
	{
		Context->CallProxyFunction(this, InObj, TEXT("ctor"), nullptr);
	}
}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
void UJavascriptGeneratedClass_Native::PostInitInstance(UObject* InObj, FObjectInstancingGraph* InstanceGraph)
{
	Super::PostInitInstance(InObj, InstanceGraph);
#else
void UJavascriptGeneratedClass_Native::PostInitInstance(UObject* InObj)
{
	Super::PostInitInstance(InObj);
#endif
	
	auto Context = JavascriptContext.Pin();
	if (Context.IsValid())
	{
		Context->CallProxyFunction(this, InObj, TEXT("ctor"), nullptr);
	}
}
