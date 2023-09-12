PRAGMA_DISABLE_SHADOW_VARIABLE_WARNINGS

#ifndef THIRD_PARTY_INCLUDES_START
#	define THIRD_PARTY_INCLUDES_START
#	define THIRD_PARTY_INCLUDES_END
#endif

#include "JavascriptIsolate_Private.h"
#include "Templates/Tuple.h"
#include "Config.h"
#include "MallocArrayBufferAllocator.h"
#include "Translator.h"
#include "ScopedArguments.h"
#include "Exception.h"
#include "Delegates.h"
#include "JavascriptIsolate.h"
#include "JavascriptContext_Private.h"
#include "JavascriptContext.h"
#include "Helpers.h"
#include "JavascriptGeneratedClass.h"
#include "JavascriptGeneratedClass_Native.h"
#include "StructMemoryInstance.h"
#include "JavascriptMemoryObject.h"
#include "Containers/Ticker.h"
#include "UObject/UObjectIterator.h"
#include "UObject/TextProperty.h"
#include "JavascriptLibrary.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Internationalization/TextNamespaceUtil.h"
#include "Internationalization/StringTableRegistry.h"
#include "Misc/MessageDialog.h"
#include "Serialization/PropertyLocalizationDataGathering.h"
#include "HAL/FileManager.h"

#if WITH_EDITOR
#include "ScopedTransaction.h"
#endif
#include "JavascriptStats.h"
#include "ConsoleDelegate.h"

THIRD_PARTY_INCLUDES_START
#include <libplatform/libplatform.h>
THIRD_PARTY_INCLUDES_END

using namespace v8;

struct FPrivateJavascriptFunction
{
	~FPrivateJavascriptFunction()
	{
		if (UnrealJSContext.IsValid())
		{
			UnrealJSContext.Reset();

			context.Reset();
			Function.Reset();
		}
		else
		{
			// v8 context is already destroyed.
			context.Empty();
			Function.Empty();
		}
	}

	Isolate* isolate;
	TWeakPtr<FJavascriptContext> UnrealJSContext;
	UniquePersistent<Context> context;
	UniquePersistent<Function> Function;
};

struct FPrivateJavascriptRef
{
	~FPrivateJavascriptRef()
	{
		if (UnrealJSContext.IsValid())
		{
			UnrealJSContext.Reset();
			Object.Reset();
		}
		else
		{
			// v8 context is already destroyed.
			Object.Empty();
		}
	}

	TWeakPtr<FJavascriptContext> UnrealJSContext;
	UniquePersistent<Object> Object;
};

template <typename CppType>
struct TStructReader
{
	UScriptStruct* ScriptStruct;

	TStructReader(UScriptStruct* InScriptStruct)
		: ScriptStruct(InScriptStruct)
	{}

	bool Read(Isolate* isolate, Local<Value> Value, CppType& Target) const;
};
#if V8_MAJOR_VERSION < 9
static v8::ArrayBuffer::Contents GCurrentContents;
#else
static std::shared_ptr<v8::BackingStore> GCurrentBackingStore;
#endif

int32 FArrayBufferAccessor::GetSize()
{
#if V8_MAJOR_VERSION < 9
	return GCurrentContents.ByteLength();
#else
	return GCurrentBackingStore->ByteLength();
#endif
}

void* FArrayBufferAccessor::GetData()
{
#if V8_MAJOR_VERSION < 9
	return GCurrentContents.Data();
#else
	return GCurrentBackingStore->Data();
#endif
}

void FArrayBufferAccessor::Discard()
{
#if V8_MAJOR_VERSION < 9
	GCurrentContents = v8::ArrayBuffer::Contents();
#else
	GCurrentBackingStore = v8::ArrayBuffer::NewBackingStore(nullptr, 0, v8::BackingStore::EmptyDeleter, nullptr);
#endif
}

const FName FJavascriptIsolateConstant::MD_BitmaskEnum(TEXT("BitmaskEnum"));

class FJavascriptIsolateImplementation : public FJavascriptIsolate
{
public:
	FJavascriptContext* GetContext()
	{
		return FJavascriptContext::FromV8(isolate_->GetCurrentContext());
	}

	Persistent<ObjectTemplate> GlobalTemplate;

	// Allocator instance should be set for V8's ArrayBuffer's
	FMallocArrayBufferAllocator AllocatorInstance;

	IDelegateManager* Delegates;

	FTickerDelegate TickDelegate;
	FTSTicker::FDelegateHandle TickHandle;
	bool bIsEditor;
	UnrealConsoleDelegate* _UnrealConsoleDelegate = nullptr;
	const uint8 ZeroMemory[100] = {0,};

	struct FObjectPropertyAccessors
	{
		static void* This(Local<Context> context, Local<Value> self)
		{
			return UObjectFromV8(context, self);
		}

		static Local<Value> Get(Isolate* isolate, Local<Object> self, FProperty* Property, const FPropertyAccessorFlags& Flags = FPropertyAccessorFlags())
		{
			auto Object = UObjectFromV8(isolate->GetCurrentContext(), self);

			if (IsValid(Object))
			{
				FScopeCycleCounterUObject ContextScope(Object);

				SCOPE_CYCLE_COUNTER(STAT_JavascriptPropertyGet);

				if (auto p = CastField<FMulticastDelegateProperty>(Property))
				{
					return GetSelf(isolate)->Delegates->GetProxy(self, Object, p);
				}
				else if (auto p = CastField<FDelegateProperty>(Property))
				{
					return GetSelf(isolate)->Delegates->GetProxy(self, Object, p);
				}
				else
				{
					return ReadProperty(isolate, Property, (uint8*)Object, FObjectPropertyOwner(Object), Flags);
				}
			}
			else
			{
				return v8::Undefined(isolate);
			}
		}

		//@TODO : Property-type 'routing' is not necessary!
		static void Set(Isolate* isolate, Local<Object> self, FProperty* Property, Local<Value> value, const FPropertyAccessorFlags& Flags = FPropertyAccessorFlags())
		{
			FIsolateHelper I(isolate);

			auto context = isolate->GetCurrentContext();
			auto Object = UObjectFromV8(context, self);
			// Direct access to delegate
			auto SetDelegate = [&](Local<Value> proxy) {
				if (!proxy->IsObject())
				{
					I.Throw(TEXT("Set delegate on invalid instance"));
					return;
				}

				if (!(value->IsFunction() || value->IsNull() || value->IsArray()))
				{
					I.Throw(TEXT("Only [function] or null allowed to set delegate"));
					return;
				}

				auto ProxyObject = proxy->ToObject(context).ToLocalChecked();
				{
					auto clear_fn = v8::Handle<Function>::Cast(ProxyObject->Get(context, I.Keyword("Clear")).ToLocalChecked());
					(void)clear_fn->Call(context, ProxyObject, 0, nullptr);
				}

				auto add_fn = v8::Handle<Function>::Cast(ProxyObject->Get(context, I.Keyword("Add")).ToLocalChecked());

				// "whole array" can be set
				if (value->IsArray())
				{
					auto arr = v8::Handle<Array>::Cast(value);
					auto Length = arr->Length();
					for (decltype(Length) Index = 0; Index < Length; ++Index)
					{
						auto elem = arr->Get(context, Index);
						if (elem.IsEmpty()) continue;
						v8::Handle<Value> args[] = { elem.ToLocalChecked() };
						(void)add_fn->Call(context, ProxyObject, 1, args);
					}
				}
				// only one delegate
				else if (!value->IsNull())
				{
					v8::Handle<Value> args[] = {value};
					(void)add_fn->Call(context, ProxyObject, 1, args);
				}
			};

			if (IsValid(Object))
			{
				FScopeCycleCounterUObject ContextScope(Object);

				SCOPE_CYCLE_COUNTER(STAT_JavascriptPropertySet);

				// Multicast delegate
				if (auto p = CastField<FMulticastDelegateProperty>(Property))
				{
					auto proxy = GetSelf(isolate)->Delegates->GetProxy(self, Object, p);
					SetDelegate(proxy);
				}
				// delegate
				else if (auto p = CastField<FDelegateProperty>(Property))
				{
					auto proxy = GetSelf(isolate)->Delegates->GetProxy(self, Object, p);
					SetDelegate(proxy);
				}
				else
				{
					WriteProperty(isolate, Property, (uint8*)Object, value, FObjectPropertyOwner(Object), Flags);
				}
			}
		}
	};

	struct FStructPropertyAccessors
	{
		static void* This(Local<Context> context, Local<Value> self)
		{
			return FStructMemoryInstance::FromV8(context, self)->GetMemory();
		}

		static Local<Value> Get(Isolate* isolate, Local<Object> self, FProperty* Property, const FPropertyAccessorFlags& Flags = FPropertyAccessorFlags())
		{
			auto Instance = FStructMemoryInstance::FromV8(isolate->GetCurrentContext(), self);
			if (Instance)
			{
				return ReadProperty(isolate, Property, Instance->GetMemory(), FStructMemoryPropertyOwner(Instance), Flags);
			}
			else
			{
				return v8::Undefined(isolate);
			}
		}

		static void Set(Isolate* isolate, Local<Object> self, FProperty* Property, Local<Value> value, const FPropertyAccessorFlags& Flags = FPropertyAccessorFlags())
		{
			FIsolateHelper I(isolate);

			auto Instance = FStructMemoryInstance::FromV8(isolate->GetCurrentContext(), self);
			if (Instance)
			{
				WriteProperty(isolate, Property, Instance->GetMemory(), value, FStructMemoryPropertyOwner(Instance), Flags);
			}
			else
			{
				I.Throw(TEXT("Null struct"));
			}
		}
	};

	void RegisterSelf(Isolate* isolate)
	{
		isolate_ = isolate;
		isolate->SetData(0, this);
		Delegates = IDelegateManager::Create(isolate);
		_UnrealConsoleDelegate = new UnrealConsoleDelegate(isolate_);
		v8::debug::SetConsoleDelegate(isolate_, _UnrealConsoleDelegate);

#if STATS
		SetupCallbacks();
#endif
	}

	virtual void ResetUnrealConsoleDelegate() override
	{
		v8::debug::SetConsoleDelegate(isolate_, _UnrealConsoleDelegate);
	}

#if STATS
	FCycleCounter Counter[4];

	void OnGCEvent(bool bStart, GCType type, GCCallbackFlags flags)
	{
		auto GetStatId = [](int Index) -> TStatId {
			switch (Index)
			{
			case 0: return GET_STATID(STAT_Scavenge);
			case 1: return GET_STATID(STAT_MarkSweepCompact);
			case 2: return GET_STATID(STAT_IncrementalMarking);
			default: return GET_STATID(STAT_ProcessWeakCallbacks);
			}
		};

		auto GCEvent = [&](int Index) {
			if (bStart)
			{
				Counter[Index].Start(GetStatId(Index));
			}
			else
			{
				Counter[Index].Stop();
			}
		};

		for (int32 Index = 0; Index < UE_ARRAY_COUNT(Counter); ++Index)
		{
			if (type & (1 << Index))
			{
				GCEvent(Index);
			}
		}
	}

	void SetupCallbacks()
	{
		isolate_->AddGCEpilogueCallback([](Isolate* isolate, GCType type, GCCallbackFlags flags) {
			GetSelf(isolate)->OnGCEvent(false, type, flags);
		});

		isolate_->AddGCPrologueCallback([](Isolate* isolate, GCType type, GCCallbackFlags flags) {
			GetSelf(isolate)->OnGCEvent(true, type, flags);
		});
	}
#endif

	FJavascriptIsolateImplementation(bool bIsEditorIsolate)
	{
		bIsEditor = bIsEditorIsolate;
		Isolate::CreateParams params;

		// Set our array buffer allocator instance
		params.array_buffer_allocator = &AllocatorInstance;

		// Bind this instance to newly created V8 isolate
		RegisterSelf(Isolate::New(params));

		GenerateBlueprintFunctionLibraryMapping();

		InitializeGlobalTemplate();

		OnWorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddRaw(this, &FJavascriptIsolateImplementation::OnWorldCleanup);
		TickDelegate = FTickerDelegate::CreateRaw(this, &FJavascriptIsolateImplementation::HandleTicker);
		TickHandle = FTSTicker::GetCoreTicker().AddTicker(TickDelegate);
	}

	void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources)
	{
		if (World->IsGameWorld())
			return;

		for (auto It = ClassToFunctionTemplateMap.CreateIterator(); It; ++It)
		{
			UClass* Class = It.Key();
			//UE_LOG(LogJavascript, Log, TEXT("JavascriptIsolate referencing %s / %s %s (gen by %s %s)"), *(Class->GetOuter()->GetName()), *(Class->GetClass()->GetName()), *(Class->GetName()), Class->ClassGeneratedBy ? *(Class->ClassGeneratedBy->GetClass()->GetName()) : TEXT("none"), Class->ClassGeneratedBy ? *(Class->ClassGeneratedBy->GetName()) : TEXT("none"));
			int ValidIndex = IsExcludeGCUClassTarget(Class);

			if (ValidIndex == 0)
			{
				It.RemoveCurrent();
			}
		}
	}

	void InitializeGlobalTemplate()
	{
		// Declares isolate/handle scope
		Isolate::Scope isolate_scope(isolate_);
		HandleScope handle_scope(isolate_);

		v8::Handle<Context> context = Context::New(isolate_);
		Context::Scope ContextScope(context);

		// Create a new object template
		auto ObjectTemplate = ObjectTemplate::New(isolate_);

		// Save it into the persistant handle
		GlobalTemplate.Reset(isolate_, ObjectTemplate);

		// Export all structs
		for (TObjectIterator<UScriptStruct> It; It; ++It)
		{
			ExportStruct(*It);
		}

		// Export all classes
		for (TObjectIterator<UClass> It; It; ++It)
		{
			ExportUClass(*It);
		}

		// Export all enums
		for (TObjectIterator<UEnum> It; It; ++It)
		{
			ExportEnum(*It);
		}

		// ExportConsole();

		ExportMemory(ObjectTemplate);

		ExportMisc(ObjectTemplate);
	}

	~FJavascriptIsolateImplementation()
	{
		ReleaseAllPersistentHandles();

		Delegates->Destroy();
		Delegates = nullptr;

		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
		FWorldDelegates::OnWorldCleanup.Remove(OnWorldCleanupHandle);
		v8::debug::SetConsoleDelegate(isolate_, nullptr);

#if V8_MAJOR_VERSION > 8
		auto platform = reinterpret_cast<v8::Platform*>(IV8::Get().GetV8Platform());
		v8::platform::NotifyIsolateShutdown(platform, isolate_);
#endif
		isolate_->Dispose();
	}

	bool HandleTicker(float DeltaTime)
	{
		auto platform = reinterpret_cast<v8::Platform*>(IV8::Get().GetV8Platform());
		v8::platform::PumpMessageLoop(platform, isolate_);
#if V8_MAJOR_VERSION > 8
		v8::platform::RunIdleTasks(platform, isolate_, DeltaTime);
#endif
		return true;
	}

	void ReleaseAllPersistentHandles()
	{
		// Release all exported classes
		ClassToFunctionTemplateMap.Empty();

		// Release all exported structs(non-class)
		ScriptStructToFunctionTemplateMap.Empty();

		// Release global template
		GlobalTemplate.Reset();
	}

	void GenerateBlueprintFunctionLibraryMapping()
	{
		for (TObjectIterator<UClass> It; It; ++It)
		{
			UClass* Class = *It;

			// Blueprint function library only
			if (Class->IsChildOf(UBlueprintFunctionLibrary::StaticClass()))
			{
				// Iterate over all functions
				for (TFieldIterator<UFunction> FuncIt(Class, EFieldIteratorFlags::ExcludeSuper); FuncIt; ++FuncIt)
				{
					auto Function = *FuncIt;
					TFieldIterator<FProperty> It(Function);

					if (Function->GetName() == TEXT("GeneratedClass"))
					{
						continue;
					}

					// It should be a static function
					if ((Function->FunctionFlags & FUNC_Static) && It)
					{
						// and have first argument to bind with.
						if ((It->PropertyFlags & (CPF_Parm | CPF_ReturnParm)) == CPF_Parm)
						{
							// The first argument should be type of object
							if (auto p = CastField<FObjectPropertyBase>(*It))
							{
								auto TargetClass = p->PropertyClass;

								// GetWorld() may fail and crash, so target class is bound to UWorld
								if (TargetClass == UObject::StaticClass() && (p->GetName() == TEXT("WorldContextObject") || p->GetName() == TEXT("WorldContext")))
								{
									TargetClass = UWorld::StaticClass();
								}

								BlueprintFunctionLibraryMapping.Add(TargetClass, Function);
								continue;
							}
							else if (auto p = CastField<FStructProperty>(*It))
							{
								BlueprintFunctionLibraryMapping.Add(p->Struct, Function);
								continue;
							}
						}

						// Factory function?
						for (auto It2 = It; It2; ++It2)
						{
							if ((It2->PropertyFlags & (CPF_Parm | CPF_ReturnParm)) == (CPF_Parm | CPF_ReturnParm))
							{
								if (auto p = CastField<FStructProperty>(*It2))
								{
									BlueprintFunctionLibraryFactoryMapping.Add(p->Struct, Function);
									break;
								}
							}
						}
					}
				}
			}
		}
	}

	// To tell Unreal engine's GC not to destroy these objects!
	virtual void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector) override
	{
		// All classes
		for (auto It = ClassToFunctionTemplateMap.CreateIterator(); It; ++It)
		{
			UClass* Class = It.Key();
			//UE_LOG(LogJavascript, Log, TEXT("JavascriptIsolate referencing %s / %s %s (gen by %s %s)"), *(Class->GetOuter()->GetName()), *(Class->GetClass()->GetName()), *(Class->GetName()), Class->ClassGeneratedBy ? *(Class->ClassGeneratedBy->GetClass()->GetName()) : TEXT("none"), Class->ClassGeneratedBy ? *(Class->ClassGeneratedBy->GetName()) : TEXT("none"));
			if (!::IsValid(Class) || !Class->IsValidLowLevel())
			{
				It.RemoveCurrent();
			}
			else
			{
				Collector.AddReferencedObject(Class, InThis);
			}
		}

		// All structs
		for (auto It = ScriptStructToFunctionTemplateMap.CreateIterator(); It; ++It)
		{
			Collector.AddReferencedObject(It.Key(), InThis);
		}
	}

	Local<Value> InternalReadProperty(FProperty* Property, uint8* Buffer, const IPropertyOwner& Owner, const FPropertyAccessorFlags& Flags)
	{
		FIsolateHelper I(isolate_);

		if (!Buffer)
		{
			I.Throw(TEXT("Read property from invalid memory"));
			return v8::Undefined(isolate_);
		}

#if WITH_EDITOR
		const FString& BitmaskEnumName = Property->GetMetaData(FJavascriptIsolateConstant::MD_BitmaskEnum);
		if (!BitmaskEnumName.IsEmpty())
		{
			if (auto p = CastField<FNumericProperty>(Property))
			{
				if (p->IsInteger())
				{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
					const UEnum* BitmaskEnum = FindFirstObject<UEnum>(*BitmaskEnumName);
#else
					const UEnum* BitmaskEnum = FindObject<UEnum>(ANY_PACKAGE, *BitmaskEnumName);
#endif
					if (::IsValid(BitmaskEnum))
					{
						int32 EnumCount = BitmaskEnum->NumEnums();

						auto Value = p->GetSignedIntPropertyValue(Property->ContainerPtrToValuePtr<int64>(Buffer));

						TArray<FString> EnumStrings;

						for (int32 i = 0; i < EnumCount - 1; ++i)
						{
							if (Value & BitmaskEnum->GetValueByIndex(i))
							{
								EnumStrings.Add(BitmaskEnum->GetNameStringByIndex(i));
							}
						}

						return I.Keyword(FString::Join(EnumStrings, TEXT(",")));
					}
				}
			}
		}
#endif
		if (auto p = CastField<FIntProperty>(Property))
		{
			return Int32::New(isolate_, p->GetPropertyValue_InContainer(Buffer));
		}
		else if (auto p = CastField<FUInt32Property>(Property))
		{
			return Uint32::New(isolate_, p->GetPropertyValue_InContainer(Buffer));
		}
		else if (auto p = CastField<FInt64Property>(Property))
		{
			return Integer::New(isolate_, p->GetPropertyValue_InContainer(Buffer));
		}
		else if (auto p = CastField<FUInt64Property>(Property))
		{
			return BigInt::New(isolate_, p->GetPropertyValue_InContainer(Buffer));
		}
		else if (auto p = CastField<FInt8Property>(Property))
		{
			return Int32::New(isolate_, p->GetPropertyValue_InContainer(Buffer));
		}
		else if (auto p = CastField<FInt16Property>(Property))
		{
			return Int32::New(isolate_, p->GetPropertyValue_InContainer(Buffer));
		}
		else if (auto p = CastField<FUInt16Property>(Property))
		{
			return Uint32::New(isolate_, p->GetPropertyValue_InContainer(Buffer));
		}
		else if (auto p = CastField<FFloatProperty>(Property))
		{
			return Number::New(isolate_, p->GetPropertyValue_InContainer(Buffer));
		}
		else if (auto p = CastField<FDoubleProperty>(Property))
		{
			return Number::New(isolate_, p->GetPropertyValue_InContainer(Buffer));
		}
		else if (auto p = CastField<FBoolProperty>(Property))
		{
            return v8::Boolean::New(isolate_, p->GetPropertyValue_InContainer(Buffer));
		}
		else if (auto p = CastField<FNameProperty>(Property))
		{
			auto name = p->GetPropertyValue_InContainer(Buffer);
			return I.Keyword(name.ToString());
		}
		else if (auto p = CastField<FStrProperty>(Property))
		{
			const FString& Data = p->GetPropertyValue_InContainer(Buffer);
			return V8_String(isolate_, Data);
		}
		else if (auto p = CastField<FTextProperty>(Property))
		{
			const FText& Data = p->GetPropertyValue_InContainer(Buffer);
			if (!Flags.Alternative)
			{
				///@hack: Support uninitialized ftext (e.g. function prototype parameter properties)
				const uint8* buf = p->ContainerPtrToValuePtr<uint8>(Buffer);
				if (!FMemory::Memcmp(buf, (const void*)&ZeroMemory, sizeof(FText)))
				{
					return V8_String(isolate_, "EmptyString");
				}

				return V8_String(isolate_, Data.ToString());
			}
			else
			{
				// Wraps the FText to FJavascriptText for supports to js.
				FString Namespace;
				FString Key;
				FName TableId;
				if (Data.IsFromStringTable())
				{
					FTextInspector::GetTableIdAndKey(Data, TableId, Key);
				}
				FPropertyLocalizationDataGatherer::ExtractTextIdentity(Data, Namespace, Key, false);
				FJavascriptText wrapper = { Data.ToString(), TextNamespaceUtil::StripPackageNamespace(Namespace), Key, TableId, Data };
				auto Memory = FStructMemoryInstance::Create(FJavascriptText::StaticStruct(), FNoPropertyOwner(), (void*)&wrapper);
				// set FJavascriptText's lifetime to Owner's;
				GetSelf(isolate_)->RegisterScriptStructInstance(Memory, v8::External::New(isolate_, Owner.GetOwnerInstancePtr()));
				return ExportStructInstance(FJavascriptText::StaticStruct(), (uint8*)Memory->GetMemory(), FNoPropertyOwner());
			}
		}
		else if (auto p = CastField<FClassProperty>(Property))
		{
			auto Class = Cast<UClass>(p->GetPropertyValue_InContainer(Buffer));

			if (Class)
			{
				return ExportUClass(Class)->GetFunction(isolate_->GetCurrentContext()).ToLocalChecked();
			}
			else
			{
				return Null(isolate_);
			}
		}
		else if (auto p = CastField<FStructProperty>(Property))
		{
			if (auto ScriptStruct = Cast<UScriptStruct>(p->Struct))
			{
				return ExportStructInstance(ScriptStruct, p->ContainerPtrToValuePtr<uint8>(Buffer), Owner);
			}
			else
			{
				UE_LOG(LogJavascript, Warning, TEXT("Non ScriptStruct found : %s"), *p->Struct->GetName());

				return v8::Undefined(isolate_);
			}
		}
		else if (auto p = CastField<FArrayProperty>(Property))
		{
			FScriptArrayHelper_InContainer helper(p, Buffer);
			auto len = (uint32_t)(helper.Num());
			auto arr = Array::New(isolate_, len);
			auto context = isolate_->GetCurrentContext();

			auto Inner = p->Inner;

			if (Inner->IsA(FStructProperty::StaticClass()) && (Flags.Alternative == false))
			{
				uint8* ElementBuffer = (uint8*)FMemory_Alloca(Inner->GetSize());
				for (decltype(len) Index = 0; Index < len; ++Index)
				{
					Inner->InitializeValue(ElementBuffer);
					Inner->CopyCompleteValueFromScriptVM(ElementBuffer, helper.GetRawPtr(Index));
					if (arr->Set(context, Index, ReadProperty(isolate_, Inner, ElementBuffer, FNoPropertyOwner(), Flags)).FromMaybe(true)) {} // V8_WARN_UNUSED_RESULT;
					Inner->DestroyValue(ElementBuffer);
				}
			}
			else
			{
				for (decltype(len) Index = 0; Index < len; ++Index)
				{
					if (arr->Set(context, Index, ReadProperty(isolate_, Inner, helper.GetRawPtr(Index), Owner, Flags)).FromMaybe(true)) {} // V8_WARN_UNUSED_RESULT;
				}
			}

			return arr;
		}
		else if (auto p = CastField<FSoftObjectProperty>(Property))
		{
			// string only
// 			auto* Data = p->GetObjectPropertyValue_InContainer(Buffer);
// 			if (Data)
// 			{
// 				return ExportObject(Data);
// 			}
// 			else
// 			{
				auto Value = p->GetPropertyValue_InContainer(Buffer);
				return V8_String(isolate_, Value.ToString());
// 			}
		}
		else if (auto p = CastField<FObjectPropertyBase>(Property))
		{
			return ExportObject(p->GetObjectPropertyValue_InContainer(Buffer));
		}
		else if (auto p = CastField<FByteProperty>(Property))
		{
			auto Value = p->GetPropertyValue_InContainer(Buffer);

			if (p->Enum)
			{
				return I.Keyword(p->Enum->GetNameStringByIndex(Value));
			}
			else
			{
				return Int32::New(isolate_, Value);
			}
		}
		else if (auto p = CastField<FEnumProperty>(Property))
		{
			int32 Value = p->GetUnderlyingProperty()->GetValueTypeHash(Buffer);
			return I.Keyword(p->GetEnum()->GetNameStringByIndex(Value));
		}
		else if (auto p = CastField<FSetProperty>(Property))
		{
			FScriptSetHelper_InContainer SetHelper(p, Buffer);

			auto Out = Array::New(isolate_);

			auto Num = SetHelper.Num();
			auto context = isolate_->GetCurrentContext();
			for (int Index = 0; Index < Num; ++Index)
			{
				auto PairPtr = SetHelper.GetElementPtr(Index);

				(void)Out->Set(context, Index, InternalReadProperty(p->ElementProp, SetHelper.GetElementPtr(Index), Owner, Flags));
			}

			return Out;
		}
		else if (auto p = CastField<FMapProperty>(Property))
		{
			FScriptMapHelper_InContainer MapHelper(p, Buffer);

			auto Out = Object::New(isolate_);

			auto Num = MapHelper.Num();
			auto context = isolate_->GetCurrentContext();
			for (int Index = 0; Index < Num; ++Index)
			{
				uint8* PairPtr = MapHelper.GetPairPtr(Index);
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 22
				auto Key = InternalReadProperty(p->KeyProp, PairPtr + p->MapLayout.KeyOffset, Owner, Flags);
#else
				auto Key = InternalReadProperty(p->KeyProp, PairPtr, Owner, Flags);
#endif
				auto Value = InternalReadProperty(p->ValueProp, PairPtr, Owner, Flags);

				(void)Out->Set(context, Key, Value);
			}

			return Out;
		}

		return I.Keyword("<Unsupported type>");
	}

	void ReadOffStruct(Local<Object> v8_obj, UStruct* Struct, uint8* struct_buffer)
	{
		SCOPE_CYCLE_COUNTER(STAT_JavascriptReadOffStruct);
		FScopeCycleCounterUObject StructContext(Struct);

		FIsolateHelper I(isolate_);

		MaybeLocal<Array> _arr = v8_obj->GetOwnPropertyNames(isolate_->GetCurrentContext());
		if (_arr.IsEmpty()) return;

		auto arr = _arr.ToLocalChecked();

		auto len = arr->Length();
		auto context = isolate_->GetCurrentContext();
		for (TFieldIterator<FProperty> PropertyIt(Struct, EFieldIteratorFlags::IncludeSuper); PropertyIt && len; ++PropertyIt)
		{
			auto Property = *PropertyIt;
			auto PropertyName = PropertyNameToString(Property, !bIsEditor);

			auto name = I.Keyword(PropertyName);
			auto maybe_value = v8_obj->Get(context, name);

			if (maybe_value.IsEmpty()) continue;
			auto value = maybe_value.ToLocalChecked();

			if (!value->IsUndefined())
			{
				len--;
				InternalWriteProperty(Property, struct_buffer, value, FNoPropertyOwner(), FPropertyAccessorFlags());
			}
		}
	}

	void InternalWriteProperty(FProperty* Property, uint8* Buffer, v8::Handle<Value> Value, const IPropertyOwner& Owner, const FPropertyAccessorFlags& Flags)
	{
		FIsolateHelper I(isolate_);

		if (!Buffer)
		{
			I.Throw(TEXT("Write property on invalid memory"));
			return;
		}

		if (Value.IsEmpty() || Value->IsUndefined()) return;

#if WITH_EDITOR
		const FString& BitmaskEnumName = Property->GetMetaData(FJavascriptIsolateConstant::MD_BitmaskEnum);
		if (!BitmaskEnumName.IsEmpty())
		{
			if (auto p = CastField<FNumericProperty>(Property))
			{
				if (p->IsInteger())
				{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
					const UEnum* BitmaskEnum = FindFirstObject<UEnum>(*BitmaskEnumName);
#else
					const UEnum* BitmaskEnum = FindObject<UEnum>(ANY_PACKAGE, *BitmaskEnumName);
#endif

					if (::IsValid(BitmaskEnum))
					{
						auto Str = StringFromV8(isolate_, Value);
						TArray<FString> EnumStrings;
						Str.ParseIntoArray(EnumStrings, TEXT(","));
						int64 EnumValue = 0;

						for (int32 i = 0; i < EnumStrings.Num(); ++i)
						{
							EnumValue |= BitmaskEnum->GetValueByNameString(*EnumStrings[i], EGetByNameFlags::None);
						}

						p->SetIntPropertyValue(Property->ContainerPtrToValuePtr<int64>(Buffer), EnumValue);
						return;
					}
				}
			}
		}
#endif
		if (auto p = CastField<FIntProperty>(Property))
		{
			p->SetPropertyValue_InContainer(Buffer, Value->Int32Value(isolate_->GetCurrentContext()).ToChecked());
		}
		else if (auto p = CastField<FInt64Property>(Property))
		{
			p->SetPropertyValue_InContainer(Buffer, Value->IntegerValue(isolate_->GetCurrentContext()).ToChecked());
		}
		else if (auto p = CastField<FUInt32Property>(Property))
		{
			p->SetPropertyValue_InContainer(Buffer, Value->Uint32Value(isolate_->GetCurrentContext()).ToChecked());
		}
		else if (auto p = CastField<FInt8Property>(Property))
		{
			p->SetPropertyValue_InContainer(Buffer, Value->Int32Value(isolate_->GetCurrentContext()).ToChecked());
		}
		else if (auto p = CastField<FInt16Property>(Property))
		{
			p->SetPropertyValue_InContainer(Buffer, Value->Int32Value(isolate_->GetCurrentContext()).ToChecked());
		}
		else if (auto p = CastField<FUInt16Property>(Property))
		{
			p->SetPropertyValue_InContainer(Buffer, Value->Uint32Value(isolate_->GetCurrentContext()).ToChecked());
		}
		else if (auto p = CastField<FUInt64Property>(Property))
		{
			auto v = Value->ToBigInt(isolate_->GetCurrentContext()).ToLocalChecked();
			p->SetPropertyValue_InContainer(Buffer, v->Uint64Value());
		}
		else if (auto p = CastField<FFloatProperty>(Property))
		{
			p->SetPropertyValue_InContainer(Buffer, Value->NumberValue(isolate_->GetCurrentContext()).ToChecked());
		}
		else if (auto p = CastField<FDoubleProperty>(Property))
		{
			p->SetPropertyValue_InContainer(Buffer, Value->NumberValue(isolate_->GetCurrentContext()).ToChecked());
		}
		else if (auto p = CastField<FBoolProperty>(Property))
		{
			p->SetPropertyValue_InContainer(Buffer, Value->BooleanValue(isolate_));
		}
		else if (auto p = CastField<FNameProperty>(Property))
		{
			p->SetPropertyValue_InContainer(Buffer, FName(*StringFromV8(isolate_, Value)));
		}
		else if (auto p = CastField<FStrProperty>(Property))
		{
			p->SetPropertyValue_InContainer(Buffer, StringFromV8(isolate_, Value));
		}
		else if (auto p = CastField<FTextProperty>(Property))
		{
			if (!Flags.Alternative)
			{
				p->SetPropertyValue_InContainer(Buffer, FText::FromString(StringFromV8(isolate_, Value)));
			}
			else
			{
				const FText& Data = p->GetPropertyValue_InContainer(Buffer);

				auto Instance = FStructMemoryInstance::FromV8(isolate_->GetCurrentContext(), Value);
				if (Instance)
				{
					if (Instance->Struct->IsChildOf(FJavascriptText::StaticStruct()))
					{
						if (Instance->GetMemory())
						{
							auto JText = reinterpret_cast<FJavascriptText*>(Instance->GetMemory());
							p->SetPropertyValue_InContainer(Buffer, UJavascriptLibrary::UpdateLocalizationText(*JText, Owner));
						}
					}
					else
						I.Throw(FString::Printf(TEXT("TextProperty needed JavascriptText struct")));
				}
				else
					I.Throw(FString::Printf(TEXT("Needed JavascriptText struct data")));
			}
		}
		else if (auto p = CastField<FClassProperty>(Property))
		{
			if (Value->IsString())
			{
				auto UString = StringFromV8(isolate_, Value);
				if (UString == TEXT("null"))
				{
					p->SetPropertyValue_InContainer(Buffer, nullptr);
				}
				else
				{
					auto Object = StaticLoadObject(UObject::StaticClass(), nullptr, *UString);
					if (auto Class = Cast<UClass>(Object))
					{
						p->SetPropertyValue_InContainer(Buffer, Class);
					}
					else if (auto BP = Cast<UBlueprint>(Object))
					{
						auto BPGC = BP->GeneratedClass;
						p->SetPropertyValue_InContainer(Buffer, BPGC);
					}
					else
					{
						p->SetPropertyValue_InContainer(Buffer, Object);
					}
				}
			}
			else
			{
				p->SetPropertyValue_InContainer(Buffer, UClassFromV8(isolate_, Value));
			}
		}
		else if (auto p = CastField<FStructProperty>(Property))
		{
			if (auto ScriptStruct = Cast<UScriptStruct>(p->Struct))
			{
				auto Instance = FStructMemoryInstance::FromV8(isolate_->GetCurrentContext(), Value);

				// If given value is an exported struct memory instance
				if (Instance != nullptr && GetContext()->MemoryToObjectMap.Contains(Instance))
				{
					auto GivenStruct = Instance->Struct;

					// Type-checking needed
					if (GivenStruct->IsChildOf(ScriptStruct))
					{
						p->CopyCompleteValue(p->ContainerPtrToValuePtr<void>(Buffer), Instance->GetMemory());
					}
					else
					{
						I.Throw(FString::Printf(TEXT("Wrong struct type (given:%s), (expected:%s)"), *GivenStruct->GetName(), *p->Struct->GetName()));
					}
				}
				else if (p->Struct->IsChildOf(FJavascriptFunction::StaticStruct()))
				{
					auto struct_buffer = p->ContainerPtrToValuePtr<uint8>(Buffer);
					FJavascriptFunction func;
					if (Value->IsFunction())
					{
						auto jsfunc = Value.As<Function>();
						func.Handle = MakeShareable(new FPrivateJavascriptFunction);
						func.Handle->isolate = isolate_;
						func.Handle->UnrealJSContext = GetContext()->AsShared();
						func.Handle->context.Reset(isolate_, isolate_->GetCurrentContext());
						func.Handle->Function.Reset(isolate_, jsfunc);
					}
					p->Struct->CopyScriptStruct(struct_buffer, &func);
				}
				else if (p->Struct->IsChildOf(FJavascriptRef::StaticStruct()))
				{
					auto struct_buffer = p->ContainerPtrToValuePtr<uint8>(Buffer);
					FJavascriptRef ref;

					if (Value->IsObject())
					{
						auto jsobj = Value.As<Object>();
						ref.Handle = MakeShareable(new FPrivateJavascriptRef);
						ref.Handle->UnrealJSContext = GetContext()->AsShared();
						ref.Handle->Object.Reset(isolate_, jsobj);
					}

					p->Struct->CopyScriptStruct(struct_buffer, &ref);
				}
				// If raw javascript object has been passed,
				else if (Value->IsObject())
				{
					auto v8_obj = Value->ToObject(isolate_->GetCurrentContext());
					auto struct_buffer = p->ContainerPtrToValuePtr<uint8>(Buffer);

					auto Struct = p->Struct;

					if (!v8_obj.IsEmpty())
					{
						ReadOffStruct(v8_obj.ToLocalChecked(), Struct, struct_buffer);
					}
				}
				else
				{
					if (nullptr == p->Struct->GetOwnerClass())
						I.Throw(FString::Printf(TEXT("Needed struct data : [null] %s"), *p->Struct->GetName()));
					else
						I.Throw(FString::Printf(TEXT("Needed struct data : %s %s"), *p->Struct->GetOwnerClass()->GetName(), *p->Struct->GetName()));
				}
			}
			else
			{
				I.Throw(FString::Printf(TEXT("No ScriptStruct found : %s"), *p->Struct->GetName()));
			}
		}
		else if (auto p = CastField<FArrayProperty>(Property))
		{
			if (Value->IsArray())
			{
				auto arr = v8::Handle<Array>::Cast(Value);
				auto len = arr->Length();

				FScriptArrayHelper_InContainer helper(p, Buffer);

				// synchronize the length
				auto CurSize = (uint32_t)helper.Num();
				if (CurSize < len)
				{
					helper.AddValues(len - CurSize);
				}
				else if (CurSize > len)
				{
					helper.RemoveValues(len, CurSize - len);
				}

				auto context = isolate_->GetCurrentContext();
				for (decltype(len) Index = 0; Index < len; ++Index)
				{
					auto maybe_value = arr->Get(context, Index);
					if (!maybe_value.IsEmpty())
					{
						WriteProperty(isolate_, p->Inner, helper.GetRawPtr(Index), maybe_value.ToLocalChecked(), Owner, Flags);
					}
				}
			}
			else
			{
				I.Throw(TEXT("Should write into array by passing an array instance"));
			}
		}
		else if (auto p = CastField<FByteProperty>(Property))
		{
			if (p->Enum)
			{
				auto Str = StringFromV8(isolate_, Value);
				auto EnumValue = p->Enum->GetIndexByName(FName(*Str), EGetByNameFlags::None);
				if (EnumValue == INDEX_NONE)
				{
					I.Throw(FString::Printf(TEXT("Enum Text %s for Enum %s failed to resolve to any value"), *Str, *p->Enum->GetName()));
				}
				else
				{
					p->SetPropertyValue_InContainer(Buffer, EnumValue);
				}
			}
			else
			{
				p->SetPropertyValue_InContainer(Buffer, Value->Int32Value(isolate_->GetCurrentContext()).ToChecked());
			}
		}
		else if (auto p = CastField<FEnumProperty>(Property))
		{
			auto Str = StringFromV8(isolate_, Value);
			auto EnumValue = p->GetEnum()->GetIndexByName(FName(*Str), EGetByNameFlags::None);
			if (EnumValue == INDEX_NONE)
			{
				I.Throw(FString::Printf(TEXT("Enum Text %s for Enum %s failed to resolve to any value"), *Str, *p->GetName()));
			}
			else
			{
				uint8* PropData = p->ContainerPtrToValuePtr<uint8>(Buffer);
				p->GetUnderlyingProperty()->SetIntPropertyValue(PropData, (int64)EnumValue);
			}
		}
		else if (auto p = CastField<FObjectPropertyBase>(Property))
		{
			p->SetObjectPropertyValue_InContainer(Buffer, UObjectFromV8(isolate_->GetCurrentContext(), Value));
		}
		else if (auto p = CastField<FSetProperty>(Property))
		{
			if (Value->IsArray())
			{
				auto arr = v8::Handle<Array>::Cast(Value);
				auto len = arr->Length();

				FScriptSetHelper_InContainer SetHelper(p, Buffer);

				auto Num = SetHelper.Num();
				auto context = isolate_->GetCurrentContext();
				for (int Index = 0; Index < Num; ++Index)
				{
					const int32 ElementIndex = SetHelper.AddDefaultValue_Invalid_NeedsRehash();
					uint8* ElementPtr = SetHelper.GetElementPtr(Index);
					auto maybe_value = arr->Get(context, Index);
					if (!maybe_value.IsEmpty())
					{
						InternalWriteProperty(p->ElementProp, ElementPtr, maybe_value.ToLocalChecked(), Owner, Flags);
					}
				}

				SetHelper.Rehash();
			}
		}
		else if (auto p = CastField<FMapProperty>(Property))
		{
			if (Value->IsObject())
			{
				auto context = isolate_->GetCurrentContext();
				auto v = Value->ToObject(context).ToLocalChecked();

				FScriptMapHelper_InContainer MapHelper(p, Buffer);

				auto PropertyNames = v->GetOwnPropertyNames(context).ToLocalChecked();
				auto Num = PropertyNames->Length();
				MapHelper.EmptyValues(Num);
				for (decltype(Num) Index = 0; Index < Num; ++Index) {
					auto Key = PropertyNames->Get(context, Index).ToLocalChecked();
					auto Value = v->Get(context, Key).ToLocalChecked();

					auto ElementIndex = MapHelper.AddDefaultValue_Invalid_NeedsRehash();
					MapHelper.Rehash();

					uint8* PairPtr = MapHelper.GetPairPtr(ElementIndex);
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 22
					InternalWriteProperty(p->KeyProp, PairPtr + p->MapLayout.KeyOffset, Key, Owner, Flags);
#else
					InternalWriteProperty(p->KeyProp, PairPtr, Key, Owner, Flags);
#endif
					InternalWriteProperty(p->ValueProp, PairPtr, Value, Owner, Flags);
				}
			}
		}
	};

	virtual Local<ObjectTemplate> GetGlobalTemplate() override
	{
		return Local<ObjectTemplate>::New(isolate_, GlobalTemplate);
	}

	void ExportConsole()
	{

		//FIsolateHelper I(isolate_);

		//Local<FunctionTemplate> Template = I.FunctionTemplate();

		//auto add_fn = [&](const char* name, FunctionCallback fn) {
		//	Template->PrototypeTemplate()->Set(I.Keyword(name), I.FunctionTemplate(fn));
		//};

		//// console.log
		//add_fn("log", [](const FunctionCallbackInfo<Value>& info)
		//{
		//	UE_LOG(LogJavascript, Log, TEXT("%s"), *StringFromArgs(info));
		//	info.GetReturnValue().Set(info.Holder());
		//});

		//// console.warn
		//add_fn("warn", [](const FunctionCallbackInfo<Value>& info)
		//{
		//	UE_LOG(LogJavascript, Warning, TEXT("%s"), *StringFromArgs(info));
		//	info.GetReturnValue().Set(info.Holder());
		//});

		//// console.info
		//add_fn("info", [](const FunctionCallbackInfo<Value>& info)
		//{
		//	UE_LOG(LogJavascript, Display, TEXT("%s"), *StringFromArgs(info));
		//	info.GetReturnValue().Set(info.Holder());
		//});

		//// console.error
		//add_fn("error", [](const FunctionCallbackInfo<Value>& info)
		//{
		//	UE_LOG(LogJavascript, Error, TEXT("%s"), *StringFromArgs(info));
		//	info.GetReturnValue().Set(info.Holder());
		//});

		//// console.assert
		//add_fn("assert", [](const FunctionCallbackInfo<Value>& info)
		//{
		//	bool to_assert = info.Length() < 1 || info[0]->IsFalse();
		//	if (to_assert)
		//	{
		//		auto stack_frame = StackTrace::CurrentStackTrace(info.GetIsolate(), 1, StackTrace::kOverview)->GetFrame(0);
		//		auto filename = stack_frame->GetScriptName();
		//		auto line_number = stack_frame->GetLineNumber();

		//		UE_LOG(LogJavascript, Error, TEXT("Assertion:%s:%d %s"), *StringFromV8(filename), line_number, *StringFromArgs(info, 1));
		//	}

		//	info.GetReturnValue().Set(info.Holder());
		//});

		//// console.void
		//add_fn("void", [](const FunctionCallbackInfo<Value>& info)
		//{
		//	info.GetReturnValue().Set(info.Holder());
		//});

		//global_templ->Set(
		//	I.Keyword("console"),
		//	// Create an instance
		//	Template->GetFunction()->NewInstance(isolate_->GetCurrentContext()).ToLocalChecked()
		//	);
	}

	void ExportMisc(Local<ObjectTemplate> global_templ)
	{
		FIsolateHelper I(isolate_);

		auto fileManagerCwd = [](const FunctionCallbackInfo<Value>& info)
		{
			info.GetReturnValue().Set(V8_String(info.GetIsolate(), IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead((const TCHAR *)L"."))); // FPaths::ProjectDir()
        };
		global_templ->Set(I.Keyword("$cwd"), I.FunctionTemplate(FV8Exception::GuardLambda(fileManagerCwd)));

#if WITH_EDITOR
		auto exec_editor = [](const FunctionCallbackInfo<Value>& info)
		{
			FEditorScriptExecutionGuard Guard;
			if (info.Length() == 1)
			{
				auto function = info[0].As<Function>();
				if (!function.IsEmpty())
				{
					auto isolate = info.GetIsolate();
					(void)function->Call(isolate->GetCurrentContext(), info.This(), 0, nullptr);
				}
			}
		};
		global_templ->Set(I.Keyword("$execEditor"), I.FunctionTemplate(FV8Exception::GuardLambda(exec_editor)));

		auto exec_transaction = [](const FunctionCallbackInfo<Value>& info)
		{
			if (info.Length() == 2)
			{
				auto isolate = info.GetIsolate();
				auto String = StringFromV8(isolate, info[0]);
				FScopedTransaction Transaction(FText::FromString(String));

				auto function = info[1].As<Function>();
				if (!function.IsEmpty())
				{
					(void)function->Call(isolate->GetCurrentContext(), info.This(), 0, nullptr);
				}
			}
		};
		global_templ->Set(I.Keyword("$execTransaction"), I.FunctionTemplate(FV8Exception::GuardLambda(exec_transaction)));

		auto exec_profile = [](const FunctionCallbackInfo<Value>& info)
		{
			if (info.Length() == 2)
			{
				for (;;)
				{
					auto function = info[1].As<Function>();
					auto isolate = info.GetIsolate();
					if (function.IsEmpty())
					{
						break;
					}

					if (info[0]->IsObject())
					{
						auto Object = UObjectFromV8(isolate->GetCurrentContext(), info[0]);
						if (Object)
						{
							FScopeCycleCounterUObject ContextScope(Object);
							info.GetReturnValue().Set(function->Call(isolate->GetCurrentContext(), info.This(), 0, nullptr).ToLocalChecked());
							return;
						}
					}

					info.GetReturnValue().Set(function->Call(isolate->GetCurrentContext(), info.This(), 0, nullptr).ToLocalChecked());
				}
			}
		};
		global_templ->Set(I.Keyword("$profile"), I.FunctionTemplate(FV8Exception::GuardLambda(exec_profile)));
#endif
	}

	template <typename T>
	void BindFunction(FIsolateHelper I, Local<FunctionTemplate> Template, const char* name, T&& fn)
	{
		Template->PrototypeTemplate()->Set(I.Keyword(name), I.FunctionTemplate(FV8Exception::GuardLambda(fn)));
	}

	struct FunctionTemplateHelper
	{
		FIsolateHelper I;
		v8::Handle<FunctionTemplate> Template;

		template <typename T>
		void Set(const char* name, T&& fn)
		{
			Template->PrototypeTemplate()->Set(I.Keyword(name), I.FunctionTemplate(FV8Exception::GuardLambda(fn)));
		}
	};

	void ExportMemory(Local<ObjectTemplate> global_templ)
	{
		FIsolateHelper I(isolate_);

		Local<FunctionTemplate> Template = I.FunctionTemplate();
		FunctionTemplateHelper FnHelper{ I, Template };

		FnHelper.Set("access", [](const FunctionCallbackInfo<Value>& info)
		{
			auto isolate = info.GetIsolate();

			FIsolateHelper I(isolate);

			if (info.Length() == 1)
			{
				auto context = isolate->GetCurrentContext();
				auto Source = Cast<UJavascriptMemoryObject>(UObjectFromV8(context, info[0]));

				if (Source)
				{
#if V8_MAJOR_VERSION < 9
					auto ab = ArrayBuffer::New(isolate, Source->GetMemory(), Source->GetSize());
#else
					auto backing_store = ArrayBuffer::NewBackingStore(Source->GetMemory(), Source->GetSize(), v8::BackingStore::EmptyDeleter, nullptr);
					auto ab = ArrayBuffer::New(isolate, std::move(backing_store));
#endif
					(void)ab->Set(context, I.Keyword("$source"), info[0]);
					info.GetReturnValue().Set(ab);
					return;
				}
			}

			I.Throw(TEXT("memory.fork requires JavascriptMemoryObject"));
		});

		FnHelper.Set("exec", [](const FunctionCallbackInfo<Value>& info)
		{
			auto isolate = info.GetIsolate();
			FIsolateHelper I(isolate);

			if (info.Length() == 2 && info[0]->IsArrayBuffer() && info[1]->IsFunction())
			{
				auto arr = info[0].As<ArrayBuffer>();
				auto function = info[1].As<Function>();
#if V8_MAJOR_VERSION < 9
				GCurrentContents = v8::ArrayBuffer::Contents();
#else
				GCurrentBackingStore = arr->GetBackingStore();
#endif
				v8::Handle<Value> argv[1];
				argv[0] = arr;
				(void)function->Call(isolate->GetCurrentContext(), info.This(), 1, argv);

#if V8_MAJOR_VERSION < 9
				GCurrentContents = v8::ArrayBuffer::Contents();
#else
				GCurrentBackingStore = v8::ArrayBuffer::NewBackingStore(isolate, 0);
#endif
			}
			else
			{
#if V8_MAJOR_VERSION < 9
				GCurrentContents = v8::ArrayBuffer::Contents();
#else
				GCurrentBackingStore = v8::ArrayBuffer::NewBackingStore(isolate, 0);
#endif
			}

			info.GetReturnValue().Set(info.Holder());
		});

		// memory.bind
		FnHelper.Set("bind", [](const FunctionCallbackInfo<Value>& info)
		{
			UE_LOG(LogJavascript, Warning, TEXT("memory.bind is deprecated. use memory.exec(ab,fn) instead."));
			auto isolate = info.GetIsolate();
			FIsolateHelper I(isolate);

			if (info.Length() == 1 && info[0]->IsArrayBuffer())
			{
				auto arr = info[0].As<ArrayBuffer>();

#if V8_MAJOR_VERSION < 9
				GCurrentContents = arr->Externalize();
#else
				GCurrentBackingStore = arr->GetBackingStore();
#endif
			}
			else
			{
#if V8_MAJOR_VERSION < 9
				GCurrentContents = v8::ArrayBuffer::Contents();
#else
				GCurrentBackingStore = v8::ArrayBuffer::NewBackingStore(isolate, 0);
#endif
			}

			info.GetReturnValue().Set(info.Holder());
		});

		// memory.unbind
		FnHelper.Set("unbind", [](const FunctionCallbackInfo<Value>& info)
		{
			auto isolate = info.GetIsolate();
			FIsolateHelper I(isolate);

			if (info.Length() == 1 && info[0]->IsArrayBuffer())
			{
				auto arr = info[0].As<ArrayBuffer>();

#if V8_MAJOR_VERSION < 9
				if (arr->IsNeuterable())
				{
					arr->Neuter();
					GCurrentContents = v8::ArrayBuffer::Contents();
				}
#else
				if (arr->IsDetachable())
				{
					arr->Detach();
					GCurrentBackingStore = v8::ArrayBuffer::NewBackingStore(isolate, 0);
				}
#endif
				else
				{
					I.Throw(TEXT("ArrayBuffer is not neuterable"));
				}
			}

			info.GetReturnValue().Set(info.Holder());
		});

		// console.void
		FnHelper.Set("write", [](const FunctionCallbackInfo<Value>& info)
		{
			auto isolate = info.GetIsolate();
			FIsolateHelper I(isolate);

			if (info.Length() == 2)
			{
				auto filename = info[0];
				auto data = info[1];

				FArchive* Ar = IFileManager::Get().CreateFileWriter(*StringFromV8(isolate, info[0]), 0);
				if (Ar)
				{
					if (data->IsArrayBuffer())
					{
						auto arr = data.As<ArrayBuffer>();
#if V8_MAJOR_VERSION < 9
						auto Contents = arr->Externalize();
						Ar->Serialize(Contents.Data(), Contents.ByteLength());
#else
						auto Contents = arr->GetBackingStore();
						Ar->Serialize(Contents->Data(), Contents->ByteLength());
#endif
					}

					delete Ar;
				}
			}
			else
			{
				I.Throw(TEXT("Two arguments needed"));
			}

			info.GetReturnValue().Set(info.Holder());
		});

		FnHelper.Set("takeSnapshot", [](const FunctionCallbackInfo<Value>& info)
		{
			auto isolate = info.GetIsolate();
			FIsolateHelper I(isolate);
			class FileOutputStream : public OutputStream
			{
			public:
				FileOutputStream(FArchive* ar) : ar_(ar) {}

				virtual int GetChunkSize() {
					return 65536;  // big chunks == faster
				}

				virtual void EndOfStream() {}

				virtual WriteResult WriteAsciiChunk(char* data, int size) {
					ar_->Serialize(data, size);
					return ar_->IsError() ? kAbort : kContinue;
				}

			private:
				FArchive* ar_;
			};

			if (info.Length() == 1)
			{
				const HeapSnapshot* const snap = info.GetIsolate()->GetHeapProfiler()->TakeHeapSnapshot();
				FArchive* Ar = IFileManager::Get().CreateFileWriter(*StringFromV8(isolate, info[0]), 0);
				if (Ar)
				{
					FileOutputStream stream(Ar);
					snap->Serialize(&stream, HeapSnapshot::kJSON);
					delete Ar;
				}

				// Work around a deficiency in the API.  The HeapSnapshot object is const
				// but we cannot call HeapProfiler::DeleteAllHeapSnapshots() because that
				// invalidates _all_ snapshots, including those created by other tools.
				const_cast<HeapSnapshot*>(snap)->Delete();
			}
			else
			{
				I.Throw(TEXT("One argument needed"));
			}
		});

		global_templ->Set(
			I.Keyword("memory"),
			// Create an instance
			Template->GetFunction(isolate_->GetCurrentContext()).ToLocalChecked()->NewInstance(isolate_->GetCurrentContext()).ToLocalChecked(),
			// Do not modify!
			ReadOnly);
	}

	template <typename Fn>
	static Local<Value> CallFunction(Isolate* isolate, Local<Value> self, UFunction* Function, UObject* Object, Fn&& GetArg)
	{
		SCOPE_CYCLE_COUNTER(STAT_JavascriptFunctionCallToEngine);

		FIsolateHelper I(isolate);

		EscapableHandleScope handle_scope(isolate);

		// Allocate buffer(param size) in stack
		uint8* Buffer = (uint8*)FMemory_Alloca(Function->ParmsSize);

		// Arguments should construct and destruct along this scope
		FScopedArguments scoped_arguments(Function, Buffer);

		// Does this function return some parameters by reference?
		bool bHasAnyOutParams = false;

		// Argument index
		int ArgIndex = 0;

		// Intentionally declares iterator outside for-loop scope
		TFieldIterator<FProperty> It(Function);

		int32 NumArgs = 0;

		// Iterate over input parameters
		for (; It && (It->PropertyFlags & (CPF_Parm | CPF_ReturnParm)) == CPF_Parm; ++It)
		{
			auto Prop = *It;

			// Get argument from caller
			auto arg = GetArg(ArgIndex++);

			// Do we have valid argument?
			if (!arg.IsEmpty() && !arg->IsUndefined())
			{
				WriteProperty(isolate, Prop, Buffer, arg, FNoPropertyOwner());
			}

			// This is 'out ref'!
			if ((It->PropertyFlags & (CPF_ConstParm | CPF_OutParm)) == CPF_OutParm)
			{
				bHasAnyOutParams = true;
			}
		}

		NumArgs = ArgIndex;

		// Call regular native function.
		FScopeCycleCounterUObject ContextScope(Object);
		FScopeCycleCounterUObject FunctionScope(Function);

		if (!Object->HasAnyFlags(RF_BeginDestroyed | RF_FinishDestroyed))
		{
			Object->ProcessEvent(Function, Buffer);
		}

		auto FetchProperty = [&](FProperty* Param, int32 ArgIndex) -> Local<Value> {
			if (auto p = CastField<FStructProperty>(Param))
			{
				// Get argument from caller
				auto arg = GetArg(ArgIndex);

				// Do we have valid argument?
				if (!arg.IsEmpty() && !arg->IsUndefined())
				{
					auto Instance = FStructMemoryInstance::FromV8(isolate->GetCurrentContext(), arg);
					if (Instance)
					{
						p->Struct->CopyScriptStruct(Instance->GetMemory(), p->ContainerPtrToValuePtr<uint8>(Buffer));

						return arg;
					}
				}
			}

			return ReadProperty(isolate, Param, Buffer, FNoPropertyOwner());
		};

		// In case of 'out ref'
		if (bHasAnyOutParams)
		{
			ArgIndex = 0;

			// Allocate an object to pass return values within
			auto OutParameters = Object::New(isolate);

			auto context = isolate->GetCurrentContext();
			// Iterate over parameters again
			for (TFieldIterator<FProperty> It(Function); It; ++It, ArgIndex++)
			{
				FProperty* Param = *It;

				auto PropertyFlags = Param->GetPropertyFlags();

				// pass return parameter as '$'
				if (PropertyFlags & CPF_ReturnParm)
				{
					// value can be null if isolate is in trouble
					auto value = FetchProperty(Param, NumArgs);
					if (!value.IsEmpty())
					{
						(void)OutParameters->Set(
							context,
							// "$"
							I.Keyword("$"),
							// property value
							value
							);
					}
				}
				// rejects 'const T&' and pass 'T&' as its name
				else if ((PropertyFlags & (CPF_ConstParm | CPF_OutParm)) == CPF_OutParm)
				{
					auto value = FetchProperty(Param, ArgIndex);
					if (!value.IsEmpty())
					{
						(void)OutParameters->Set(
							context,
							// parameter name
							I.Keyword(Param->GetName()),
							// property value
							value
							);
					}
				}
			}

			// We're done
			return handle_scope.Escape(OutParameters);
		}
		else
		{
			// Iterate to fill out return parameter (if we have one)
			for (; It; ++It)
			{
				FProperty* Param = *It;
				if (Param->GetPropertyFlags() & CPF_ReturnParm)
				{
					return handle_scope.Escape(FetchProperty(Param, NumArgs));
				}
			}
		}

		// No return value available
		return handle_scope.Escape(v8::Undefined(isolate));
	}

	void ExportFunction(v8::Handle<FunctionTemplate> Template, UFunction* FunctionToExport)
	{
		FIsolateHelper I(isolate_);

		// Exposed function body (it doesn't capture anything)
		auto FunctionBody = [](const FunctionCallbackInfo<Value>& info)
		{
			auto isolate = info.GetIsolate();

			//FIsolateHelper I(isolate);

			auto self = info.Holder();

			// Retrieve "FUNCTION"
			auto Function = reinterpret_cast<UFunction*>((Local<External>::Cast(info.Data()))->Value());

			// Determine 'this'
			auto Object = (Function->FunctionFlags & FUNC_Static) ? Function->GetOwnerClass()->ClassDefaultObject.Get() : UObjectFromV8(isolate->GetCurrentContext(), self);

			// Check 'this' is valid
			if (!IsValid(Object))
			{
				//I.Throw(FString::Printf(TEXT("Invalid instance for calling a function %s"), *Function->GetName()));
				return;
			}

			info.GetReturnValue().Set(
				// Call unreal engine function!
				CallFunction(isolate, self, Function, Object, [&](int ArgIndex) -> Local<Value> {
					// pass an argument if we have
					if (ArgIndex < info.Length())
					{
						return info[ArgIndex];
					}
					// Otherwise, just return undefined.
					else
					{
						return v8::Undefined(isolate);
					}
				})
			);
		};

		auto function_name = I.Keyword(FunctionToExport->GetName());
		auto function = I.FunctionTemplate(FV8Exception::GuardLambda(FunctionBody), FunctionToExport);

		// In case of static function, you can also call this function by 'Class.Method()'.
		if (FunctionToExport->FunctionFlags & FUNC_Static)
		{
			Template->Set(function_name, function);
		}

		// Register the function to prototype template
		Template->PrototypeTemplate()->Set(function_name, function);
	}

	void ExportBlueprintLibraryFunction(v8::Handle<FunctionTemplate> Template, UFunction* FunctionToExport)
	{
		FIsolateHelper I(isolate_);

		// Exposed function body (it doesn't capture anything)
		auto FunctionBody = [](const FunctionCallbackInfo<Value>& info)
		{
			auto isolate = info.GetIsolate();

			auto self = info.Holder();

			// Retrieve "FUNCTION"
			auto Function = reinterpret_cast<UFunction*>((Local<External>::Cast(info.Data()))->Value());

			// 'this' should be CDO of owner class
			auto Object = Function->GetOwnerClass()->ClassDefaultObject;

			info.GetReturnValue().Set(
				// Call unreal engine function!
				CallFunction(isolate, self, Function, Object, [&](int ArgIndex) -> Local<Value> {
					// The first argument is bound automatically
					if (ArgIndex == 0)
					{
						return self;
					}
					// The rest arguments are being passed like normal function call.
					else if (ArgIndex - 1 < info.Length())
					{
						return info[ArgIndex - 1];
					}
					else
					{
						return v8::Undefined(isolate);
					}
				})
			);
		};

		auto function_name = I.Keyword(FunctionToExport->GetName());
		auto function = I.FunctionTemplate(FV8Exception::GuardLambda(FunctionBody), FunctionToExport);
		
		// Register the function to prototype template
		Template->PrototypeTemplate()->Set(function_name, function);
	}

	void ExportBlueprintLibraryFactoryFunction(v8::Handle<FunctionTemplate> Template, UFunction* FunctionToExport)
	{
		FIsolateHelper I(isolate_);

		// Exposed function body (it doesn't capture anything)
		auto FunctionBody = [](const FunctionCallbackInfo<Value>& info)
		{
			auto isolate = info.GetIsolate();

			auto self = info.Holder();

			// Retrieve "FUNCTION"
			auto Function = reinterpret_cast<UFunction*>((Local<External>::Cast(info.Data()))->Value());

			// 'this' should be CDO of owner class
			auto Object = Function->GetOwnerClass()->ClassDefaultObject;

			info.GetReturnValue().Set(
				// Call unreal engine function!
				CallFunction(isolate, self, Function, Object, [&](int ArgIndex) -> Local<Value> {
					// pass an argument if we have
					if (ArgIndex < info.Length())
					{
						return info[ArgIndex];
					}
					// Otherwise, just return undefined.
					else
					{
						return v8::Undefined(isolate);
					}
				})
			);
		};

		auto function_name = I.Keyword(FunctionToExport->GetName());
		auto function = I.FunctionTemplate(FV8Exception::GuardLambda(FunctionBody), FunctionToExport);

		// Register the function to prototype template
		Template->Set(function_name, function);
	}

	virtual void PublicExportUClass(UClass* ClassToExport) override
	{
		// Clear some template maps
		OnGarbageCollectedByV8(GetContext(), ClassToExport);

		ExportUClass(ClassToExport);
	}

	virtual void PublicExportStruct(UScriptStruct* StructToExport) override
	{
		v8::UniquePersistent<v8::FunctionTemplate> Template;
		if (ScriptStructToFunctionTemplateMap.RemoveAndCopyValue(StructToExport, Template))
		{
			Template.Reset();
		}

		ExportStruct(StructToExport);
	}

	virtual int IsExcludeGCUClassTarget(UClass* TargetUClass) override
	{
		UClass* Class = TargetUClass;

		if (!::IsValid(Class) || !Class->IsValidLowLevel())
		{
			return 0;
		}

#if WITH_EDITORONLY_DATA
		if (::IsValid(Class->ClassGeneratedBy) && Class->ClassGeneratedBy->IsValidLowLevelFast())
		{
			if (Cast<UBlueprint>(Class->ClassGeneratedBy)->BlueprintType == EBlueprintType::BPTYPE_LevelScript)
			{
				return 0;
			}
		}
#endif
		return INDEX_NONE;
	}

	template <typename PropertyAccessors>
	void ExportProperty(v8::Handle<FunctionTemplate> Template, FProperty* PropertyToExport, int32 PropertyIndex)
	{
		FIsolateHelper I(isolate_);

		// Property getter
		auto Getter = [](Local<Name> property, const PropertyCallbackInfo<Value>& info) {
			auto isolate = info.GetIsolate();

			auto data = info.Data();
			check(data->IsExternal());

			auto Flags = FPropertyAccessorFlags();
			Flags.Alternative = StringFromV8(isolate, property)[0] == '$';
			auto Property = reinterpret_cast<FProperty*>((Local<External>::Cast(data))->Value());
			info.GetReturnValue().Set(PropertyAccessors::Get(isolate, info.This(), Property, Flags));
		};

		// Property setter
		auto Setter = [](Local<Name> property, Local<Value> value, const PropertyCallbackInfo<void>& info) {
			auto isolate = info.GetIsolate();

			auto data = info.Data();
			check(data->IsExternal())

			auto Flags = FPropertyAccessorFlags();
			Flags.Alternative = StringFromV8(isolate, property)[0] == '$';
			auto Property = reinterpret_cast<FProperty*>((Local<External>::Cast(data))->Value());
			PropertyAccessors::Set(isolate, info.This(), Property, value, Flags);
		};

		auto Name = PropertyNameToString(PropertyToExport, !bIsEditor);

		EPropertyAccessorAvailability AccessorAvailability = FV8Config::GetPropertyAccessorAvailability(PropertyToExport);
		if (EnumHasAllFlags(AccessorAvailability, EPropertyAccessorAvailability::Default))
		{
			Template->PrototypeTemplate()->SetAccessor(
				I.Keyword(Name),
				Getter,
				Setter,
				I.External(PropertyToExport),
				DEFAULT,
				(PropertyAttribute)(DontDelete | (FV8Config::IsWriteDisabledProperty(PropertyToExport) ? ReadOnly : 0))
			);
		}

		if (EnumHasAnyFlags(AccessorAvailability, EPropertyAccessorAvailability::AltAccessor))
		{
			Template->PrototypeTemplate()->SetAccessor(
				I.Keyword("$" + Name),
				Getter,
				Setter,
				I.External(PropertyToExport),
				DEFAULT,
				(PropertyAttribute)(DontDelete | (FV8Config::IsWriteDisabledProperty(PropertyToExport) ? ReadOnly : 0))
			);
		}

		if (EnumHasAnyFlags(AccessorAvailability, EPropertyAccessorAvailability::StructRefArray))
		{
			AddMemberFunction_GetStructRefArray<PropertyAccessors>(Template, PropertyToExport);
		}
	}

	void ExportHelperFunctions(UStruct* ClassToExport, Local<FunctionTemplate> Template)
	{
		// Bind blue print library!
		TArray<UFunction*> Functions;
		BlueprintFunctionLibraryMapping.MultiFind(ClassToExport, Functions);

		for (auto Function : Functions)
		{
			ExportBlueprintLibraryFunction(Template, Function);
		}

		BlueprintFunctionLibraryFactoryMapping.MultiFind(ClassToExport, Functions);
		for (auto Function : Functions)
		{
			ExportBlueprintLibraryFactoryFunction(Template, Function);
		}
	}

	void AddMemberFunction_Class_GetClassObject(Local<FunctionTemplate> Template, UStruct* ClassToExport)
	{
		FIsolateHelper I(isolate_);

		auto fn = [](const FunctionCallbackInfo<Value>& info) {
			auto ClassToExport = reinterpret_cast<UClass*>((Local<External>::Cast(info.Data()))->Value());

			auto isolate = info.GetIsolate();

			info.GetReturnValue().Set(GetSelf(isolate)->ForceExportObject(ClassToExport));
		};

		Template->Set(I.Keyword("GetClassObject"), I.FunctionTemplate(FV8Exception::GuardLambda(fn), ClassToExport));
	}	

	void AddMemberFunction_Class_SetDefaultSubobjectClass(Local<FunctionTemplate> Template, UStruct* ClassToExport)
	{
		FIsolateHelper I(isolate_);

		auto fn = [](const FunctionCallbackInfo<Value>& info) {
			auto isolate = info.GetIsolate();
			HandleScope scope(isolate);

			FIsolateHelper I(isolate);

			auto ClassToExport = reinterpret_cast<UClass*>((Local<External>::Cast(info.Data()))->Value());

			auto ObjectInitializer = GetSelf(isolate)->GetContext()->GetObjectInitializer();

			if (!ObjectInitializer)
			{
				I.Throw(TEXT("SetDefaultSubobjectClass must be called within ctor"));
				return;
			}

			if (info.Length() < 1)
			{
				I.Throw(TEXT("Missing arg"));
				return;
			}

			auto Class = static_cast<UJavascriptGeneratedClass*>(ObjectInitializer->GetClass());
			if (!Class->JavascriptContext.IsValid())
			{
				I.Throw(TEXT("Fatal"));
				return;
			}

			auto Context = Class->JavascriptContext.Pin();
			auto Name = StringFromV8(isolate, info[0]);
			ObjectInitializer->SetDefaultSubobjectClass(*Name, ClassToExport);
		};

		Template->Set(I.Keyword("SetDefaultSubobjectClass"), I.FunctionTemplate(FV8Exception::GuardLambda(fn), ClassToExport));
	}

	void AddMemberFunction_Class_CreateDefaultSubobject(Local<FunctionTemplate> Template, UStruct* ClassToExport)
	{
		FIsolateHelper I(isolate_);

		auto fn = [](const FunctionCallbackInfo<Value>& info) {
			auto isolate = info.GetIsolate();
			HandleScope scope(isolate);

			FIsolateHelper I(isolate);

			auto ClassToExport = reinterpret_cast<UClass*>((Local<External>::Cast(info.Data()))->Value());

			auto ObjectInitializer = GetSelf(isolate)->GetContext()->GetObjectInitializer();

			if (!ObjectInitializer)
			{
				I.Throw(TEXT("CreateDefaultSubobject must be called within ctor"));
				return;
			}

			if (info.Length() < 1)
			{
				I.Throw(TEXT("Missing arg"));
				return;
			}

			auto Class = static_cast<UJavascriptGeneratedClass*>(ObjectInitializer->GetClass());
			if (!Class->JavascriptContext.IsValid())
			{
				I.Throw(TEXT("Fatal"));
				return;
			}

			auto Context = Class->JavascriptContext.Pin();
			auto ReturnType = ClassToExport;
			auto Name = StringFromV8(isolate, info[0]);
			bool bTransient = info.Length() > 1 ? info[1]->BooleanValue(isolate) : false;
			bool bIsRequired = info.Length() > 2 ? info[2]->BooleanValue(isolate) : true;
			bool bIsAbstract = info.Length() > 3 ? info[3]->BooleanValue(isolate) : false;
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 23
			auto Object = ObjectInitializer->CreateDefaultSubobject(ObjectInitializer->GetObj(), *Name, ReturnType, ReturnType, bIsRequired, bIsAbstract, bTransient);
#else
			auto Object = ObjectInitializer->CreateDefaultSubobject(ObjectInitializer->GetObj(), *Name, ReturnType, ReturnType, bIsRequired, bTransient);
#endif
			info.GetReturnValue().Set(Context->ExportObject(Object));
		};

		Template->Set(I.Keyword("CreateDefaultSubobject"), I.FunctionTemplate(FV8Exception::GuardLambda(fn), ClassToExport));
	}

	void AddMemberFunction_Class_GetDefaultSubobjectByName(Local<FunctionTemplate> Template, UStruct* ClassToExport)
	{
		FIsolateHelper I(isolate_);

		auto fn = [](const FunctionCallbackInfo<Value>& info) {
			auto ClassToExport = reinterpret_cast<UClass*>((Local<External>::Cast(info.Data()))->Value());

			auto isolate = info.GetIsolate();

			auto Name = StringFromV8(isolate, info[0]);

			info.GetReturnValue().Set(GetSelf(isolate)->ExportObject(ClassToExport->GetDefaultSubobjectByName(*Name)));
		};

		Template->Set(I.Keyword("GetDefaultSubobjectByName"), I.FunctionTemplate(FV8Exception::GuardLambda(fn), ClassToExport));
	}

	void AddMemberFunction_Class_GetDefaultObject(Local<FunctionTemplate> Template, UStruct* ClassToExport)
	{
		FIsolateHelper I(isolate_);

		auto fn = [](const FunctionCallbackInfo<Value>& info) {
			auto ClassToExport = reinterpret_cast<UClass*>((Local<External>::Cast(info.Data()))->Value());

			auto isolate = info.GetIsolate();

			info.GetReturnValue().Set(GetSelf(isolate)->ExportObject(ClassToExport->GetDefaultObject()));
		};

		Template->Set(I.Keyword("GetDefaultObject"), I.FunctionTemplate(FV8Exception::GuardLambda(fn), ClassToExport));
	}

	void AddMemberFunction_Class_Find(Local<FunctionTemplate> Template, UClass* ClassToExport)
	{
		FIsolateHelper I(isolate_);

		auto fn = [](const FunctionCallbackInfo<Value>& info) {
			auto ClassToExport = reinterpret_cast<UClass*>((Local<External>::Cast(info.Data()))->Value());

			auto isolate = info.GetIsolate();

			FIsolateHelper I(isolate);

			if (info.Length() == 2 && info[1]->IsString())
			{
				auto Outer = UObjectFromV8(isolate->GetCurrentContext(), info[0]);
				auto ObjectName = StringFromV8(isolate, info[1]->ToString(isolate->GetCurrentContext()).ToLocalChecked());
				UObject* obj = nullptr;
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
				if (Outer != nullptr)
				{
					obj = StaticFindObject(ClassToExport, Outer, *ObjectName);
				}
				else
				{
					obj = StaticFindFirstObject(ClassToExport, *ObjectName);
				}
#else
				obj = StaticFindObject(ClassToExport, Outer ? Outer : ANY_PACKAGE, *ObjectName);
#endif
				auto out = GetSelf(isolate)->ExportObject(obj);
				info.GetReturnValue().Set(out);
			}
			else
			{
				I.Throw(TEXT("Missing resource name to load"));
			}
		};

		Template->Set(I.Keyword("Find"), I.FunctionTemplate(FV8Exception::GuardLambda(fn), ClassToExport));
	}

	void AddMemberFunction_Class_Load(Local<FunctionTemplate> Template, UClass* ClassToExport)
	{
		FIsolateHelper I(isolate_);

		auto fn = [](const FunctionCallbackInfo<Value>& info) {
			auto ClassToExport = reinterpret_cast<UClass*>((Local<External>::Cast(info.Data()))->Value());

			auto isolate = info.GetIsolate();

			FIsolateHelper I(isolate);

			if (info.Length() == 1 && info[0]->IsString())
			{
				auto obj = StaticLoadObject(ClassToExport, nullptr, *StringFromV8(isolate, info[0]->ToString(isolate->GetCurrentContext()).ToLocalChecked()));
				auto out = GetSelf(isolate)->ExportObject(obj);
				info.GetReturnValue().Set(out);
			}
			else
			{
				I.Throw(TEXT("Missing resource name to load"));
			}
		};

		Template->Set(I.Keyword("Load"), I.FunctionTemplate(FV8Exception::GuardLambda(fn), ClassToExport));
	}

	Local<Value> C_Operator(UStruct* StructToExport, Local<Value> Value)
	{
		auto Instance = FStructMemoryInstance::FromV8(isolate_->GetCurrentContext(), Value);

		// If given value is an instance
		if (Instance)
		{
			auto GivenStruct = Instance->Struct;
			if (Instance->Struct->IsChildOf(StructToExport))
			{
				return Value;
			}
		}
		else if (auto ScriptStruct = Cast<UScriptStruct>(StructToExport))
		{
			if (Value->IsObject())
			{
				auto v = Value->ToObject(isolate_->GetCurrentContext()).ToLocalChecked();
				auto Size = ScriptStruct->GetStructureSize();
				auto Target = (uint8*)(FMemory_Alloca(Size));
				FMemory::Memzero(Target, Size);
				ReadOffStruct(v, ScriptStruct, Target);
				return ExportStructInstance(ScriptStruct, Target, FNoPropertyOwner());
			}
		}

		return Local<v8::Value>();
	}

	void AddMemberFunction_Struct_C(Local<FunctionTemplate> Template, UStruct* StructToExport)
	{
		FIsolateHelper I(isolate_);

		auto fn = [](const FunctionCallbackInfo<Value>& info) {
			auto StructToExport = reinterpret_cast<UStruct*>((Local<External>::Cast(info.Data()))->Value());

			auto isolate = info.GetIsolate();

			if (info.Length() == 1)
			{
				info.GetReturnValue().Set(GetSelf(isolate)->C_Operator(StructToExport,info[0]));
			}
		};

		Template->Set(I.Keyword("C"), I.FunctionTemplate(FV8Exception::GuardLambda(fn), StructToExport));
	}

	void AddMemberFunction_JavascriptRef_get(Local<FunctionTemplate> Template)
	{
		FIsolateHelper I(isolate_);

		auto fn = [](const FunctionCallbackInfo<Value>& info) {
			auto isolate = info.GetIsolate();

			auto self = info.This();

			if (self->IsUndefined())
			{
				return;
			}

			auto Instance = FStructMemoryInstance::FromV8(isolate->GetCurrentContext(), self);

			if (Instance->GetMemory())
			{
				auto Ref = reinterpret_cast<FJavascriptRef*>(Instance->GetMemory());
				if (Ref->Handle.IsValid())
				{
					FPrivateJavascriptRef* Handle = Ref->Handle.Get();
					auto object = Local<Object>::New(isolate, Handle->Object);

					info.GetReturnValue().Set(object);
				}
			}
		};

		Template->PrototypeTemplate()->Set(I.Keyword("get"), I.FunctionTemplate(FV8Exception::GuardLambda(fn), nullptr));
	}

	void AddMemberFunction_Struct_clone(Local<FunctionTemplate> Template, UStruct* StructToExport)
	{
		FIsolateHelper I(isolate_);

		auto fn = [](const FunctionCallbackInfo<Value>& info) {
			auto Struct = reinterpret_cast<UScriptStruct*>((Local<External>::Cast(info.Data()))->Value());

			auto isolate = info.GetIsolate();

			auto self = info.This();
			auto out = Object::New(isolate);

			auto Instance = FStructMemoryInstance::FromV8(isolate->GetCurrentContext(), self);

			if (Instance->GetMemory())
			{
				info.GetReturnValue().Set(GetSelf(isolate)->ExportStructInstance(Instance->Struct, Instance->GetMemory(), FNoPropertyOwner()));
			}
		};

		Template->PrototypeTemplate()->Set(I.Keyword("clone"), I.FunctionTemplate(FV8Exception::GuardLambda(fn), StructToExport));
	}

	template <typename PropertyAccessor>
	void AddMemberFunction_Struct_toJSON(Local<FunctionTemplate> Template, UStruct* ClassToExport)
	{
		FIsolateHelper I(isolate_);

		if (bIsEditor)
		{
			auto fn = [](const FunctionCallbackInfo<Value>& info) {
				auto Class = reinterpret_cast<UClass*>((Local<External>::Cast(info.Data()))->Value());

				auto isolate = info.GetIsolate();
				FIsolateHelper I(isolate);

				auto self = info.This();
				auto out = Object::New(isolate);

				auto Object_toJSON = [&](Local<Value> value) -> Local<Value>
				{
					auto Object = UObjectFromV8(isolate->GetCurrentContext(), value);
					if (Object == nullptr)
					{
						return Null(isolate);
					}
					else
					{
						return V8_String(isolate, Object->GetPathName());
					}
				};
				auto context = isolate->GetCurrentContext();

				for (TFieldIterator<FProperty> PropertyIt(Class, EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
				{
					auto Property = *PropertyIt;

					if (FV8Config::CanExportProperty(Class, Property))
					{
						auto PropertyName = PropertyNameToString(Property, false);

						auto name = I.Keyword(PropertyName);
						auto value = PropertyAccessor::Get(isolate, self, Property);
						if (auto p = CastField<FClassProperty>(Property))
						{
							auto Class = UClassFromV8(isolate, value);

							if (Class)
							{
								auto BPGC = Cast<UBlueprintGeneratedClass>(Class);
								if (BPGC)
								{
#if WITH_EDITORONLY_DATA
									auto BP = Cast<UBlueprint>(BPGC->ClassGeneratedBy);
									value = I.String(BP->GetPathName());
#else
									value = I.String(BPGC->GetPathName());
#endif
								}
								else
								{
									value = I.String(Class->GetPathName());
								}
							}
							else
							{
								value = I.Keyword("null");
							}

							(void)out->Set(context, name, value);
						}
						else if (auto p = CastField<FObjectPropertyBase>(Property))
						{
							(void)out->Set(context, name, Object_toJSON(value));
						}
						else if (auto p = CastField<FArrayProperty>(Property))
						{
							if (auto q = CastField<FObjectPropertyBase>(p->Inner))
							{
								auto arr = v8::Handle<Array>::Cast(value);
								auto len = arr->Length();

								auto out_arr = Array::New(isolate, len);
								(void)out->Set(context, name, out_arr);

								for (decltype(len) Index = 0; Index < len; ++Index)
								{
									auto maybe_value = arr->Get(context, Index);
									if (!maybe_value.IsEmpty())
									{
										(void)out_arr->Set(context, Index, Object_toJSON(maybe_value.ToLocalChecked()));
									}
								}
							}
							else
							{
								(void)out->Set(context, name, value);
							}
						}
						else
						{
							(void)out->Set(context, name, value);
						}
					}
				}

				info.GetReturnValue().Set(out);
			};
			Template->PrototypeTemplate()->Set(I.Keyword("toJSON"), I.FunctionTemplate(FV8Exception::GuardLambda(fn), ClassToExport));
		}
		else
		{
			auto fn = [](const FunctionCallbackInfo<Value>& info) {
				auto Class = reinterpret_cast<UClass*>((Local<External>::Cast(info.Data()))->Value());

				auto isolate = info.GetIsolate();
				FIsolateHelper I(isolate);

				auto self = info.This();
				auto out = Object::New(isolate);

				auto Object_toJSON = [&](Local<Value> value) -> Local<Value>
				{
					auto Object = UObjectFromV8(isolate->GetCurrentContext(), value);
					if (Object == nullptr)
					{
						return Null(isolate);
					}
					else
					{
						return V8_String(isolate, Object->GetPathName());
					}
				};

				auto context = isolate->GetCurrentContext();

				for (TFieldIterator<FProperty> PropertyIt(Class, EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
				{
					auto Property = *PropertyIt;

					if (FV8Config::CanExportProperty(Class, Property))
					{
						auto PropertyName = PropertyNameToString(Property, true);

						auto name = I.Keyword(PropertyName);
						auto value = PropertyAccessor::Get(isolate, self, Property);
						if (auto p = CastField<FClassProperty>(Property))
						{
							auto Class = UClassFromV8(isolate, value);

							if (Class)
							{
								auto BPGC = Cast<UBlueprintGeneratedClass>(Class);
								if (BPGC)
								{
#if WITH_EDITORONLY_DATA
									auto BP = Cast<UBlueprint>(BPGC->ClassGeneratedBy);
									value = I.String(BP->GetPathName());
#else
									value = I.String(BPGC->GetPathName());
#endif
								}
								else
								{
									value = I.String(Class->GetPathName());
								}
							}
							else
							{
								value = I.Keyword("null");
							}

							(void)out->Set(context, name, value);
						}
						else if (auto p = CastField<FObjectPropertyBase>(Property))
						{
							(void)out->Set(context, name, Object_toJSON(value));
						}
						else if (auto p = CastField<FArrayProperty>(Property))
						{
							if (auto q = CastField<FObjectPropertyBase>(p->Inner))
							{
								auto arr = v8::Handle<Array>::Cast(value);
								auto len = arr->Length();

								auto out_arr = Array::New(isolate, len);
								(void)out->Set(context, name, out_arr);

								for (decltype(len) Index = 0; Index < len; ++Index)
								{
									auto maybe_value = arr->Get(context, Index);
									if (!maybe_value.IsEmpty())
									{
										(void)out_arr->Set(context, Index, Object_toJSON(maybe_value.ToLocalChecked()));
									}
								}
							}
							else
							{
								(void)out->Set(context, name, value);
							}
						}
						else
						{
							(void)out->Set(context, name, value);
						}
					}
				}

				info.GetReturnValue().Set(out);
			};
			Template->PrototypeTemplate()->Set(I.Keyword("toJSON"), I.FunctionTemplate(FV8Exception::GuardLambda(fn), ClassToExport));
		}

	}

	template <typename PropertyAccessor>
	void AddMemberFunction_Struct_RawAccessor(Local<FunctionTemplate> Template, UStruct* ClassToExport)
	{
		FIsolateHelper I(isolate_);

		auto fn = [](const FunctionCallbackInfo<Value>& info) {
			auto Class = reinterpret_cast<UClass*>((Local<External>::Cast(info.Data()))->Value());

			auto isolate = info.GetIsolate();
			FIsolateHelper I(isolate);

			if (info.Length() != 2 || !info[1]->IsFunction()) return;

			HandleScope handle_scope(isolate);

			const FName PropertyNameToAccess(*StringFromV8(isolate, info[0]));
			auto function = info[1].As<Function>();

			auto self = info.This();
			auto Context = Context::New(isolate);
			auto Instance = PropertyAccessor::This(Context, self);
			for (TFieldIterator<FProperty> PropertyIt(Class, EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
			{
				auto Property = *PropertyIt;

				if (auto p = CastField<FArrayProperty>(Property))
				{
					FScriptArrayHelper_InContainer helper(p, Instance);

					if (FV8Config::CanExportProperty(Class, Property) && MatchPropertyName(Property,PropertyNameToAccess))
					{
						v8::Handle<Value> argv[1];
#if V8_MAJOR_VERSION < 9
						argv[0] = ArrayBuffer::New(isolate, helper.GetRawPtr(), helper.Num() * p->Inner->GetSize());
#else
						auto backing_store = ArrayBuffer::NewBackingStore(helper.GetRawPtr(), helper.Num() * p->Inner->GetSize(), v8::BackingStore::EmptyDeleter, nullptr);
						argv[0] = ArrayBuffer::New(isolate, std::move(backing_store));
#endif
						auto out = function->Call(Context, info.This(), 1, argv).ToLocalChecked();
						info.GetReturnValue().Set(out);
						break;
					}
				}
			}
		};

		Template->PrototypeTemplate()->Set(I.Keyword("$memaccess"), I.FunctionTemplate(FV8Exception::GuardLambda(fn), ClassToExport));
	}

	template <typename PropertyAccessor>
	void AddMemberFunction_GetStructRefArray(v8::Handle<FunctionTemplate> Template, FProperty* PropertyToExport)
	{
		auto fn = [](const FunctionCallbackInfo<Value>& info)
		{
			auto isolate = info.GetIsolate();
			FIsolateHelper I(isolate);

			auto self = info.This();
			auto Context = Context::New(isolate);
			auto Instance = PropertyAccessor::This(Context, self);

			auto PropertyToExport = reinterpret_cast<FProperty*>((Local<External>::Cast(info.Data()))->Value());

			// Depends on alternative implementation of InternalReadProperty for UArrayProperty containing UStructProperty.
			auto Flags = FPropertyAccessorFlags();
			Flags.Alternative = true;
			auto value = PropertyAccessor::Get(isolate, self, PropertyToExport, Flags);
			info.GetReturnValue().Set(value);
		};

		FIsolateHelper I(isolate_);
		FString StructRefArrayAccessorName = FString::Printf(TEXT("$getStructRefArray_%s"), *PropertyToExport->GetName());
		Template->PrototypeTemplate()->Set(I.Keyword(StructRefArrayAccessorName), I.FunctionTemplate(FV8Exception::GuardLambda(fn), PropertyToExport));
	}

	Local<FunctionTemplate> InternalExportUClass(UClass* ClassToExport)
	{
		FIsolateHelper I(isolate_);

		EscapableHandleScope handle_scope(isolate_);

		auto ConstructorBody = [](const FunctionCallbackInfo<Value>& info)
		{
			auto isolate = info.GetIsolate();

			FIsolateHelper I(isolate);

			auto ClassToExport = reinterpret_cast<UClass*>((Local<External>::Cast(info.Data()))->Value());

			if (info.IsConstructCall())
			{
				auto self = info.This();

				UObject* Associated = nullptr;

				// Called by system (via ExportObject)
				if (info.Length() == 1 && info[0]->IsExternal())
				{
					auto ext = Local<External>::Cast(info[0]);

					Associated = reinterpret_cast<UObject*>(ext->Value());

					if (!Associated->IsValidLowLevel())
					{
						Associated = nullptr;
					}
				}

				// Called by user (via 'new' operator)
				if (!Associated)
				{
					const bool bIsJavascriptClass =
						ClassToExport->GetClass()->IsChildOf(UJavascriptGeneratedClass::StaticClass()) ||
						ClassToExport->GetClass()->IsChildOf(UJavascriptGeneratedClass_Native::StaticClass());

					auto PreCreate = [&]() {
						if (bIsJavascriptClass)
						{
							GetSelf(isolate)->ObjectUnderConstructionStack.Push(FPendingClassConstruction(self, ClassToExport));
						}
					};

					// Custom constructors
					if (ClassToExport->IsChildOf(AActor::StaticClass()))
					{
						if (info.Length() == 0)
						{
							I.Throw(TEXT("Missing world to spawn"));
							return;
						}

						auto World = Cast<UWorld>(UObjectFromV8(isolate->GetCurrentContext(), info[0]));
						if (!World)
						{
							I.Throw(TEXT("Missing world to spawn"));
							return;
						}

						FVector Location(ForceInitToZero);
						FRotator Rotation(ForceInitToZero);

						UPackage* CoreUObjectPackage = UObject::StaticClass()->GetOutermost();
						static UScriptStruct* VectorStruct = FindObjectChecked<UScriptStruct>(CoreUObjectPackage, TEXT("Vector"));
						static UScriptStruct* RotatorStruct = FindObjectChecked<UScriptStruct>(CoreUObjectPackage, TEXT("Rotator"));
						static TStructReader<FVector> VectorReader(VectorStruct);
						static TStructReader<FRotator> RotatorReader(RotatorStruct);

						FActorSpawnParameters SpawnInfo;
						switch (FMath::Min(2, info.Length())) {
						case 2:
							if (!VectorReader.Read(isolate, info[1], Location)) return;
							if (info.Length() == 2) break;
						case 3:
							if (!RotatorReader.Read(isolate, info[2], Rotation)) return;
							if (info.Length() == 3) break;
						case 4:
							SpawnInfo.Name = FName(*StringFromV8(isolate, info[3]));
							if (info.Length() == 4) break;
						case 5:
							SpawnInfo.ObjectFlags = RF_Transient | RF_Transactional;
							if (info.Length() == 5) break;
						case 6:
							SpawnInfo.Template = Cast<AActor>(UObjectFromV8(isolate->GetCurrentContext(), info[5]));
							if (info.Length() == 6) break;
						default:
							break;
						}
						
						ULevel* CurrentLevel = World->GetCurrentLevel();
						if (StaticFindObjectFast(nullptr, CurrentLevel, SpawnInfo.Name))
						{
							FString Msg = FString::Printf(TEXT("An actor of name '%s' already exists in level '%s'."), *(SpawnInfo.Name.ToString()), *(CurrentLevel->GetFullName()));
							FMessageDialog::Open(EAppMsgType::Ok, FText::FromString(Msg));
							RequestEngineExit(TEXT("JavascriptIsolate_Private RequestExit"));
							I.Throw(TEXT("Failed to spawn"));
							return;
						}

						PreCreate();
						Associated = World->SpawnActor(ClassToExport, &Location, &Rotation, SpawnInfo);
#if WITH_EDITOR
						if (SpawnInfo.Name != NAME_None)
							(Cast<AActor>(Associated))->SetActorLabel(StringFromV8(isolate, info[3]));
#endif
					}
					else
					{
						UObject* Outer = GetTransientPackage();
						FName Name = NAME_None;
						EObjectFlags ObjectFlags = RF_NoFlags;
						UObject* Template = nullptr;

						if (info.Length() > 0)
						{
							if (auto value = UObjectFromV8(isolate->GetCurrentContext(), info[0]))
							{
								Outer = value;
							}
							if (info.Length() > 1)
							{
								if (StringFromV8(isolate, info[1]) == TEXT(""))
									Name = NAME_None;
								else
									Name = FName(*StringFromV8(isolate, info[1]));
							}
							if (info.Length() > 2)
							{
								ObjectFlags = (EObjectFlags)(info[2]->Int32Value(isolate->GetCurrentContext()).ToChecked());
							}
							if (info.Length() > 3)
							{
								Template = UObjectFromV8(isolate->GetCurrentContext(), info[3]);
							}
						}

						PreCreate();
						Associated = NewObject<UObject>(Outer, ClassToExport, Name, ObjectFlags, Template);
					}

					if (bIsJavascriptClass)
					{
						const auto& Last = GetSelf(isolate)->ObjectUnderConstructionStack.Last();

						bool bSafeToQuit = Last.bCatched;

						GetSelf(isolate)->ObjectUnderConstructionStack.Pop();

						if (bSafeToQuit)
						{
							return;
						}
					}

					if (!Associated)
					{
						I.Throw(TEXT("Failed to spawn"));
						return;
					}
				}

				FPendingClassConstruction(self, ClassToExport).Finalize(GetSelf(isolate), Associated);
			}
			else
			{
				info.GetReturnValue().Set(GetSelf(isolate)->C_Operator(ClassToExport, info[0]));
			}
		};

		auto Template = I.FunctionTemplate(ConstructorBody, ClassToExport);
		Template->InstanceTemplate()->SetInternalFieldCount(1);

		AddMemberFunction_Struct_C(Template, ClassToExport);

		// load
		if (!ClassToExport->IsChildOf(AActor::StaticClass()))
		{
			AddMemberFunction_Class_Load(Template, ClassToExport);
		}
		AddMemberFunction_Class_Find(Template, ClassToExport);

		AddMemberFunction_Class_GetClassObject(Template, ClassToExport);
		AddMemberFunction_Class_CreateDefaultSubobject(Template, ClassToExport);
		AddMemberFunction_Class_SetDefaultSubobjectClass(Template, ClassToExport);

		AddMemberFunction_Class_GetDefaultObject(Template, ClassToExport);
		AddMemberFunction_Class_GetDefaultSubobjectByName(Template, ClassToExport);

		AddMemberFunction_Struct_toJSON<FObjectPropertyAccessors>(Template, ClassToExport);
		AddMemberFunction_Struct_RawAccessor<FObjectPropertyAccessors>(Template, ClassToExport);

		Template->SetClassName(I.Keyword(ClassToExport->GetName()));

		auto static_class = I.Keyword("StaticClass");

		// access thru Class.prototype.StaticClass
		Template->PrototypeTemplate()->Set(static_class, I.External(ClassToExport));
		Template->Set(static_class, I.External(ClassToExport));

		for (TFieldIterator<UFunction> FuncIt(ClassToExport, EFieldIteratorFlags::ExcludeSuper); FuncIt; ++FuncIt)
		{
			UFunction* Function = *FuncIt;
			if (FV8Config::CanExportFunction(ClassToExport, Function))
			{
				ExportFunction(Template, Function);
			}
		}

		int32 PropertyIndex = 0;
		for (TFieldIterator<FProperty> PropertyIt(ClassToExport, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt, ++PropertyIndex)
		{
			FProperty* Property = *PropertyIt;
			if (FV8Config::CanExportProperty(ClassToExport, Property))
			{
				ExportProperty<FObjectPropertyAccessors>(Template, Property, PropertyIndex);
			}
		}

		return handle_scope.Escape(Template);
	}

	Local<FunctionTemplate> InternalExportStruct(UScriptStruct* StructToExport)
	{
		FIsolateHelper I(isolate_);

		EscapableHandleScope handle_scope(isolate_);

		auto fn = [](const FunctionCallbackInfo<Value>& info)
		{
			auto StructToExport = reinterpret_cast<UScriptStruct*>((Local<External>::Cast(info.Data()))->Value());

			auto isolate = info.GetIsolate();

			if (info.IsConstructCall())
			{
				auto self = info.This();

				TSharedPtr<FStructMemoryInstance> Memory;

				if (info.Length() == 2 && info[0]->IsExternal() && info[1]->IsExternal())
				{
					IPropertyOwner& Owner = *reinterpret_cast<IPropertyOwner*>(Local<External>::Cast(info[1])->Value());

					Memory = FStructMemoryInstance::Create(StructToExport, Owner, Local<External>::Cast(info[0])->Value());
				}
				else
				{
					Memory = FStructMemoryInstance::Create(StructToExport, FNoPropertyOwner());
				}

				GetSelf(isolate)->RegisterScriptStructInstance(Memory, self);

				self->SetAlignedPointerInInternalField(0, Memory.Get());
			}
			else
			{
				info.GetReturnValue().Set(GetSelf(isolate)->C_Operator(StructToExport, info[0]));
			}
		};

		auto Template = I.FunctionTemplate(fn, StructToExport);
		Template->InstanceTemplate()->SetInternalFieldCount(1);

		AddMemberFunction_Struct_C(Template, StructToExport);
		AddMemberFunction_Struct_clone(Template, StructToExport);
		AddMemberFunction_Struct_toJSON<FStructPropertyAccessors>(Template, StructToExport);
		AddMemberFunction_Struct_RawAccessor<FStructPropertyAccessors>(Template, StructToExport);

		if (StructToExport == FJavascriptRef::StaticStruct())
		{
			AddMemberFunction_JavascriptRef_get(Template);
		}

		Template->SetClassName(I.Keyword(StructToExport->GetName()));

		auto static_class = I.Keyword("StaticClass");

		// access thru Class.prototype.StaticClass
		Template->PrototypeTemplate()->Set(static_class, I.External(StructToExport));
		Template->Set(static_class, I.External(StructToExport));

		int32 PropertyIndex = 0;
		for (TFieldIterator<FProperty> PropertyIt(StructToExport, EFieldIteratorFlags::ExcludeSuper); PropertyIt; ++PropertyIt, ++PropertyIndex)
		{
			FProperty* Property = *PropertyIt;
			if (FV8Config::CanExportProperty(StructToExport, Property))
			{
				ExportProperty<FStructPropertyAccessors>(Template, Property, PropertyIndex);
			}
		}

		return handle_scope.Escape(Template);
	}

	virtual Local<FunctionTemplate> ExportStruct(UScriptStruct* ScriptStruct) override
	{
		auto ExportedFunctionTemplatePtr = ScriptStructToFunctionTemplateMap.Find(ScriptStruct);
		if (ExportedFunctionTemplatePtr == nullptr)
		{
			auto Template = InternalExportStruct(ScriptStruct);

			auto SuperStruct = Cast<UScriptStruct>(ScriptStruct->GetSuperStruct());
			if (SuperStruct)
			{
				Template->Inherit(ExportStruct(SuperStruct));
			}

			ExportHelperFunctions(ScriptStruct, Template);

			RegisterStruct(ScriptStructToFunctionTemplateMap, ScriptStruct, Template);

			return Template;
		}
		else
		{
			return Local<FunctionTemplate>::New(isolate_, *ExportedFunctionTemplatePtr);
		}
	}

	Local<Value> ExportEnum(UEnum* Enum)
	{
		FIsolateHelper I(isolate_);

		auto EnumLength = Enum->NumEnums();
		auto arr = Array::New(isolate_, EnumLength);
		auto context = isolate_->GetCurrentContext();
		for (decltype(EnumLength) Index = 0; Index < EnumLength; ++Index)
		{
			auto value = I.Keyword(Enum->GetNameStringByIndex(Index));
			(void)arr->Set(context, Index, value);
			(void)arr->Set(context, value, value);
		}

		// public name
		auto name = I.Keyword(FV8Config::Safeify(Enum->GetName()));
		GetGlobalTemplate()->Set(name, arr);

		return arr;
	}

	virtual Local<FunctionTemplate> ExportUClass(UClass* Class, bool bAutoRegister = true) override
	{
		auto ExportedFunctionTemplatePtr = ClassToFunctionTemplateMap.Find(Class);
		if (ExportedFunctionTemplatePtr == nullptr)
		{
			auto Template = InternalExportUClass(Class);

			auto SuperClass = Class->GetSuperClass();
			if (SuperClass)
			{
				Template->Inherit(ExportUClass(SuperClass));
			}

			ExportHelperFunctions(Class, Template);

			if (bAutoRegister)
			{
				RegisterUClass(Class, Template);
			}

			return Template;
		}
		else
		{
			return Local<FunctionTemplate>::New(isolate_, *ExportedFunctionTemplatePtr);
		}
	}

	Local<Value> ExportStructInstance(UScriptStruct* Struct, uint8* Buffer, const IPropertyOwner& Owner) override
	{
		FIsolateHelper I(isolate_);
		if (!Struct || !Buffer)
		{
			return v8::Undefined(isolate_);
		}

		auto v8_struct = ExportStruct(Struct);
		auto arg = I.External(Buffer);
		auto arg2 = I.External((void*)&Owner);
		v8::Handle<Value> args[] = { arg, arg2 };

		auto maybe_func = v8_struct->GetFunction(isolate_->GetCurrentContext());

		if (maybe_func.IsEmpty())
			return v8::Undefined(isolate_);

		auto obj = maybe_func.ToLocalChecked()->NewInstance(isolate_->GetCurrentContext(), 2, args);

		if (obj.IsEmpty())
			return v8::Undefined(isolate_);

		return obj.ToLocalChecked();
	}

	Local<Value> ForceExportObject(UObject* Object)
	{
		FIsolateHelper I(isolate_);

		if (!(::IsValid(Object)) || !Object->IsValidLowLevelFast())
		{
			return v8::Undefined(isolate_);
		}

		auto ObjectPtr = GetContext()->ObjectToObjectMap.Find(Object);
		if (ObjectPtr == nullptr)
		{
			auto v8_class = ExportUClass(Object->GetClass());
			auto arg = I.External(Object);
			v8::Handle<Value> args[] = { arg };

			auto maybe_func = v8_class->GetFunction(isolate_->GetCurrentContext());

			if (maybe_func.IsEmpty())
				return v8::Undefined(isolate_);

			auto maybe_value = maybe_func.ToLocalChecked()->NewInstance(isolate_->GetCurrentContext(), 1, args);

			if (maybe_value.IsEmpty())
				return v8::Undefined(isolate_);

			return maybe_value.ToLocalChecked();
		}
		else
		{
			return Local<Value>::New(isolate_, *ObjectPtr);
		}
	}

	Local<Value> ExportObject(UObject* Object, bool bForce = false) override
	{
		if (bForce) return ForceExportObject(Object);

		FIsolateHelper I(isolate_);

		auto Context = GetContext();
		if (!Context)
		{
			return v8::Undefined(isolate_);
		}

		if (!(::IsValid(Object)) || !Object->IsValidLowLevelFast())
		{
			return v8::Undefined(isolate_);
		}

		auto ObjectPtr = Context->ObjectToObjectMap.Find(Object);
		if (ObjectPtr == nullptr)
		{
			if (ObjectUnderConstructionStack.Num() > 0)
			{
				auto& Last = ObjectUnderConstructionStack.Last();
				if (!Last.bCatched)
				{
					if (!Object->HasAnyFlags(RF_ClassDefaultObject) && Object->IsA(Last.Class))
					{
						Last.bCatched = true;
						Last.Finalize(this, Object);
						return Last.Object;
					}
				}
			}
			Local<Value> value;

			if (auto Class = Cast<UClass>(Object))
			{
				auto maybe_value = ExportUClass(Class)->GetFunction(isolate_->GetCurrentContext());
				if (maybe_value.IsEmpty())
				{
					return v8::Undefined(isolate_);
				}
				value = maybe_value.ToLocalChecked();
			}
			else if (auto Struct = Cast<UScriptStruct>(Object))
			{
				auto maybe_value= ExportStruct(Struct)->GetFunction(isolate_->GetCurrentContext());
				if (maybe_value.IsEmpty())
				{
					return v8::Undefined(isolate_);
				}
				value = maybe_value.ToLocalChecked();
			}
			else
			{
				auto Class = Object->GetClass();

				auto v8_class = ExportUClass(Class);
				auto arg = I.External(Object);
				v8::Handle<Value> args[] = { arg };

				auto maybe_func = v8_class->GetFunction(isolate_->GetCurrentContext());

				if (maybe_func.IsEmpty())
				{
					return v8::Undefined(isolate_);
				}

				auto maybe_value = maybe_func.ToLocalChecked()->NewInstance(isolate_->GetCurrentContext(), 1, args);

				if (maybe_value.IsEmpty())
				{
					return v8::Undefined(isolate_);
				}
				value = maybe_value.ToLocalChecked();
			}

			return value;
		}
		else
		{
			return Local<Value>::New(isolate_, *ObjectPtr);
		}
	}

	// For tracking exported entities
	template <typename U, typename T>
	void SetWeak(UniquePersistent<U>& Handle, T* GarbageCollectedObject)
	{
		typedef TPair<FJavascriptContext*, T*> WeakData;
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
		typedef typename WeakData::KeyType WeakDataKeyInitType;
		typedef typename WeakData::ValueType WeakDataValueInitType;
		typedef TPairInitializer<WeakDataKeyInitType, WeakDataValueInitType> InitializerType;
#else
		typedef TPairInitializer<FJavascriptContext*, T*> InitializerType;
#endif

#if V8_MAJOR_VERSION == 5 && V8_MINOR_VERSION < 3
		Handle.template SetWeak<WeakData>(new WeakData(InitializerType(GetContext(), GarbageCollectedObject)), [](const WeakCallbackData<U, WeakData>& data) {
			auto Parameter = data.GetParameter();

			auto Context = Parameter->Key;
			auto Self = static_cast<FJavascriptIsolateImplementation*>(Context->Environment.Get());
			Self->OnGarbageCollectedByV8(Context,Parameter->Value);

			delete Parameter;
		});
#else
		Handle.template SetWeak<WeakData>(new WeakData(InitializerType(GetContext(), GarbageCollectedObject)), [](const WeakCallbackInfo<WeakData>& data) {
			auto Parameter = data.GetParameter();

			auto Context = Parameter->Key;
			auto Self = static_cast<FJavascriptIsolateImplementation*>(Context->Environment.Get());
			Self->OnGarbageCollectedByV8(Context, Parameter->Value);

			delete Parameter;
		}, WeakCallbackType::kParameter);
#endif
	}

	template <typename StructType>
	void RegisterStruct(TMap< StructType*, v8::UniquePersistent<v8::FunctionTemplate> >& TheMap, StructType* Class, Local<FunctionTemplate> Template)
	{
		FIsolateHelper I(isolate_);

		// public name
		auto name = I.Keyword(FV8Config::Safeify(Class->GetName()));

		// If we are running in a context, we also register this class to the context directly.
		auto Context = isolate_->GetCurrentContext();
		if (!Context.IsEmpty())
		{
			auto maybe_func = Template->GetFunction(Context);
			if (!maybe_func.IsEmpty())
			{
				(void)Context->Global()->Set(Context, name, maybe_func.ToLocalChecked());
			}
		}

		// Register this class to the global template so that any other contexts which will be created later have this function template.
		GetGlobalTemplate()->Set(name, Template);

		// Track this class from v8 gc.
		auto& result = TheMap.Add(Class, UniquePersistent<FunctionTemplate>(isolate_, Template));
		SetWeak(result, Class);
	}

	virtual void RegisterUClass(UClass* Class, v8::Local<v8::FunctionTemplate> Template) override
	{
		RegisterStruct(ClassToFunctionTemplateMap, Class, Template);
	}

	void RegisterScriptStruct(UScriptStruct* Struct, Local<FunctionTemplate> Template)
	{
		RegisterStruct(ScriptStructToFunctionTemplateMap, Struct, Template);
	}

	void RegisterObject(UObject* UnrealObject, Local<Value> value)
	{
		FJavascriptContext* JavaContextPtr = GetContext();

		auto& result = JavaContextPtr->ObjectToObjectMap.Add(UnrealObject, UniquePersistent<Value>(isolate_, value));
		SetWeak(result, UnrealObject);
	}

	void RegisterScriptStructInstance(TSharedPtr<FStructMemoryInstance> MemoryObject, Local<Value> value)
	{
		auto MemoryObjectPtr = MemoryObject.Get();
		auto Info = FJavascriptContext::FExportedStructMemoryInfo(MemoryObject, UniquePersistent<Value>(isolate_, value));
		auto& result = GetContext()->MemoryToObjectMap.Add(MemoryObjectPtr, MoveTemp(Info));
		SetWeak(result.Value, MemoryObjectPtr);
	}

	void OnGarbageCollectedByV8(FJavascriptContext* Context, FStructMemoryInstance* Memory)
	{
		// We should keep ourselves clean
		Context->MemoryToObjectMap.Remove(Memory);
	}

	void OnGarbageCollectedByV8(FJavascriptContext* Context, UObject* Object)
	{
		if (auto klass = Cast<UClass>(Object))
		{
			v8::UniquePersistent<v8::FunctionTemplate> Template;
			if (ClassToFunctionTemplateMap.RemoveAndCopyValue(klass, Template))
			{
				Template.Reset();
			}
		}

		v8::UniquePersistent<v8::Value> Persistant;
		if (Context->ObjectToObjectMap.RemoveAndCopyValue(Object, Persistant))
		{
			Persistant.Reset();
		}
	}

	static FJavascriptIsolateImplementation* GetSelf(Isolate* isolate)
	{
		return reinterpret_cast<FJavascriptIsolateImplementation*>(isolate->GetData(0));
	}
};

FJavascriptIsolate* FJavascriptIsolate::Create(bool bIsEditor)
{
	return new FJavascriptIsolateImplementation(bIsEditor);
}

Local<Value> FJavascriptIsolate::ReadProperty(Isolate* isolate, FProperty* Property, uint8* Buffer, const IPropertyOwner& Owner, const FPropertyAccessorFlags& Flags)
{
	return FJavascriptIsolateImplementation::GetSelf(isolate)->InternalReadProperty(Property, Buffer, Owner, Flags);
}

void FJavascriptIsolate::WriteProperty(Isolate* isolate, FProperty* Property, uint8* Buffer, v8::Handle<Value> Value, const IPropertyOwner& Owner, const FPropertyAccessorFlags& Flags)
{
	FJavascriptIsolateImplementation::GetSelf(isolate)->InternalWriteProperty(Property, Buffer, Value, Owner, Flags);
}

void FPendingClassConstruction::Finalize(FJavascriptIsolate* Isolate, UObject* UnrealObject)
{
	static_cast<FJavascriptIsolateImplementation*>(Isolate)->RegisterObject(UnrealObject, Object);
	Object->SetAlignedPointerInInternalField(0, UnrealObject);
}

Local<Value> FJavascriptIsolate::ExportStructInstance(Isolate* isolate, UScriptStruct* Struct, uint8* Buffer, const IPropertyOwner& Owner)
{
	return FJavascriptIsolateImplementation::GetSelf(isolate)->ExportStructInstance(Struct, Buffer, Owner);
}


template <typename CppType>
bool TStructReader<CppType>::Read(Isolate* isolate, Local<Value> Value, CppType& Target) const
{
	FIsolateHelper I(isolate);

	auto Instance = FStructMemoryInstance::FromV8(isolate->GetCurrentContext(), Value);
	if (Instance && Instance->Struct == ScriptStruct)
	{
		ScriptStruct->CopyScriptStruct(&Target, Instance->GetMemory());
	}
	else if (Value->IsObject())
	{
		auto v8_v1 = Value->ToObject(isolate->GetCurrentContext()).ToLocalChecked();

		FJavascriptIsolateImplementation::GetSelf(isolate)->ReadOffStruct(v8_v1, ScriptStruct, reinterpret_cast<uint8*>(&Target));
	}
	else
	{
		I.Throw(TEXT("couldn't read struct"));
		return false;
	}

	return true;
}

namespace v8
{
	Local<Value> ReadProperty(Isolate* isolate, FProperty* Property, uint8* Buffer, const IPropertyOwner& Owner, const FPropertyAccessorFlags& Flags)
	{
		return FJavascriptIsolate::ReadProperty(isolate, Property, Buffer, Owner, Flags);
	}

	void WriteProperty(Isolate* isolate, FProperty* Property, uint8* Buffer, Local<Value> Value, const IPropertyOwner& Owner, const FPropertyAccessorFlags& Flags)
	{
		FJavascriptIsolate::WriteProperty(isolate, Property, Buffer, Value, Owner, Flags);
	}
}

void FJavascriptFunction::Execute()
{
	if (!Handle.IsValid() || Handle->Function.IsEmpty()) return;

	{
		FPrivateJavascriptFunction* Handle = this->Handle.Get();

		auto isolate_ = Handle->isolate;

		Isolate::Scope isolate_scope(isolate_);
		HandleScope handle_scope(isolate_);

		auto function = Local<Function>::New(Handle->isolate, Handle->Function);
		if (!function.IsEmpty())
		{
			auto context = Local<Context>::New(isolate_, Handle->context);

			Context::Scope context_scope(context);

			(void)function->Call(context, function, 0, nullptr);
		}
	}
}

void FJavascriptFunction::Execute(UScriptStruct* Struct, void* Buffer)
{
	if (!Handle.IsValid() || Handle->Function.IsEmpty()) return;

	{
		FPrivateJavascriptFunction* Handle = this->Handle.Get();

		auto isolate_ = Handle->isolate;

		Isolate::Scope isolate_scope(isolate_);
		HandleScope handle_scope(isolate_);

		auto function = Local<Function>::New(Handle->isolate, Handle->Function);
		if (!function.IsEmpty())
		{
			auto context = Local<Context>::New(isolate_, Handle->context);

			Context::Scope context_scope(context);

			auto arg = FJavascriptIsolateImplementation::GetSelf(Handle->isolate)->ExportStructInstance(Struct, (uint8*)Buffer, FNoPropertyOwner());
			v8::Handle<Value> args[] = { arg };
			(void)function->Call(context, function, 1, args);
		}
	}
}

PRAGMA_ENABLE_SHADOW_VARIABLE_WARNINGS
