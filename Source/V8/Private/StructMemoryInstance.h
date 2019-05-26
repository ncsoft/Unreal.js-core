#pragma once

struct FStructMemoryInstance
	: public TSharedFromThis<FStructMemoryInstance>
{
	FStructMemoryInstance(UScriptStruct* InStruct, const IPropertyOwner& InOwner, void* InSource)
	: Struct(InStruct), Source(InSource)
	{
		Owner = InOwner.Owner;
		if (Owner == EPropertyOwner::Object)
		{
			Object = ((const FObjectPropertyOwner&)InOwner).Object;
		}
		else if (Owner == EPropertyOwner::Memory && Source)
		{
			auto OwnerInstance = ((const FStructMemoryPropertyOwner&)InOwner).Memory;
			if (OwnerInstance->Owner == EPropertyOwner::None)
			{
				Parent = OwnerInstance->AsShared();
			}
			else if (OwnerInstance->Parent.IsValid())
			{
				Parent = OwnerInstance->Parent;
			}
			else
			{
				Object = OwnerInstance->Object.Get();
			}
		}

		if (Owner == EPropertyOwner::None)
		{
			Buffer.AddUninitialized(Struct->GetStructureSize());
			Struct->InitializeStruct(GetMemory());

			if (Source)
			{
				Struct->CopyScriptStruct(GetMemory(), Source);
				Source = nullptr;
			}
		}
	}

	~FStructMemoryInstance()
	{
		if (Owner == EPropertyOwner::None)
		{
			Struct->DestroyStruct(GetMemory());
		}
	}

	UObject* GetNearestOwnerObject()
	{
		auto* StructMemoryPtr = this;
		while (StructMemoryPtr)
		{
			if (StructMemoryPtr->Owner == EPropertyOwner::None)
				return nullptr;
			if (StructMemoryPtr->Object.IsValid())
				return StructMemoryPtr->Object.Get();
			StructMemoryPtr = StructMemoryPtr->Parent.Get();
		}

		return nullptr;
	}

	// Struct 
	UScriptStruct* Struct;

	// Type
	EPropertyOwner Owner;

	// Object which this instance depends on
	FWeakObjectPtr Object;

	// Memory which this instance refers
	void* Source;

	// Parent instance
	TSharedPtr<FStructMemoryInstance> Parent;

	// Independent memory buffer
	TArray<uint8, TAlignedHeapAllocator<16>> Buffer;

	uint8* GetMemory()
	{
		if (Owner == EPropertyOwner::None)
		{
			return Buffer.GetData();
		}
		else if (Object.IsValid())
		{
			return (uint8*)Source;
		}
		else if (Parent.IsValid())
		{
			return (uint8*)Source;
		}
		else
		{
			return nullptr;
		}
	}

	static TSharedRef<FStructMemoryInstance> Create(UScriptStruct* Struct, const IPropertyOwner& InOwner, void* Source = nullptr)
	{
		return TSharedRef<FStructMemoryInstance>(new FStructMemoryInstance(Struct, InOwner, Source));
	}

	static FStructMemoryInstance* FromV8(v8::Local<v8::Context> context, v8::Local<v8::Value> Value)
	{
		auto Memory = RawMemoryFromV8(context, Value);
		return reinterpret_cast<FStructMemoryInstance*>(Memory);
	}	
};