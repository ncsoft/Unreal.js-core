#pragma once

#include "V8PCH.h"

enum class EPropertyOwner
{
	None,
	Object,
	Memory
};

struct IPropertyOwner
{
	EPropertyOwner Owner;
	virtual void* GetOwnerInstancePtr() const { return nullptr; }
};

struct FNoPropertyOwner : IPropertyOwner
{
	FNoPropertyOwner()
	{
		Owner = EPropertyOwner::None;
	}

	void* GetOwnerInstancePtr() const override { return nullptr; }
};

struct FObjectPropertyOwner : IPropertyOwner
{
	UObject* Object;

	FObjectPropertyOwner(UObject* InObject)
		: Object(InObject)
	{
		Owner = EPropertyOwner::Object;
	}
	void* GetOwnerInstancePtr() const override { return (void*)Object; }
};

struct FStructMemoryInstance;
struct FStructMemoryPropertyOwner : IPropertyOwner
{
	FStructMemoryInstance* Memory;

	FStructMemoryPropertyOwner(FStructMemoryInstance* InMemory)
		: Memory(InMemory)
	{
		Owner = EPropertyOwner::Memory;
	}
	void* GetOwnerInstancePtr() const override { return (void*)Memory; }
};

struct FPropertyAccessorFlags
{
	bool Alternative = false;
};

namespace v8
{
	Local<Value> ReadProperty(Isolate* isolate, UProperty* Property, uint8* Buffer, const IPropertyOwner& Owner, const FPropertyAccessorFlags& Flags = FPropertyAccessorFlags());
	void WriteProperty(Isolate* isolate, UProperty* Property, uint8* Buffer, Local<Value> value, const IPropertyOwner& Owner, const FPropertyAccessorFlags& Flags = FPropertyAccessorFlags());
	void ReportException(Isolate* isolate, TryCatch& try_catch);
	Local<String> V8_String(Isolate* isolate, const FString& String);
	Local<String> V8_String(Isolate* isolate, const char* String);
	Local<String> V8_KeywordString(Isolate* isolate, const FString& String);
	Local<String> V8_KeywordString(Isolate* isolate, const char* String);
	FString StringFromV8(Isolate* isolate, Local<Value> Value);
	void CallJavascriptFunction(Handle<Context> context, Handle<Value> This, UFunction* SignatureFunction, Handle<Function> func, void* Parms);
	UClass* UClassFromV8(Isolate* isolate_, Local<Value> Value);
	UObject* UObjectFromV8(Local<Context> context, Local<Value> Value);
	uint8* RawMemoryFromV8(Local<Context> context, Local<Value> Value);
	FString StringFromArgs(const FunctionCallbackInfo<v8::Value>& args, int StartIndex = 0);
	FString PropertyNameToString(UProperty* Property, bool bConvertComparisionIndex = true);
	bool MatchPropertyName(UProperty* Property, FName NameToMatch);
}
