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

#if WITH_EDITOR
template <typename Type>
static void SetMetaData(Type* Object, const FString& Key, const FString& Value)
{
	if (Key.Compare(TEXT("None"), ESearchCase::IgnoreCase) == 0 || Key.Len() == 0) return;

	if (Value.Len() == 0)
	{
		Object->SetMetaData(*Key, TEXT("true"));
	}
	else
	{
		Object->SetMetaData(*Key, *Value);
	}
}
#endif

static void SetEnumFlags(UEnum* Enum, const TArray<FString>& Flags)
{
	for (const auto& Flag : Flags)
	{
		FString Left, Right;
		if (!Flag.Split(TEXT(":"), &Left, &Right))
		{
			Left = Flag;
		}

#if WITH_EDITOR
		SetMetaData(Enum, Left, Right);
#endif
	}
}

namespace v8
{
	Local<Value> ReadProperty(Isolate* isolate, FProperty* Property, uint8* Buffer, const IPropertyOwner& Owner, const FPropertyAccessorFlags& Flags = FPropertyAccessorFlags());
	void WriteProperty(Isolate* isolate, FProperty* Property, uint8* Buffer, Local<Value> value, const IPropertyOwner& Owner, const FPropertyAccessorFlags& Flags = FPropertyAccessorFlags());
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
	FString PropertyNameToString(FProperty* Property, bool bConvertComparisionIndex = true);
	bool MatchPropertyName(FProperty* Property, FName NameToMatch);
}
