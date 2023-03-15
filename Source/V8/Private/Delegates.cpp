#include "Delegates.h"
#include "JavascriptDelegate.h"
#include "Translator.h"
#include "JavascriptStats.h"
#include "UObject/GCObject.h"
#include "../../Launch/Resources/Version.h"
#include "v8-version.h"

PRAGMA_DISABLE_SHADOW_VARIABLE_WARNINGS

using namespace v8;

class FJavascriptDelegate : public FGCObject, public TSharedFromThis<FJavascriptDelegate>
{
public:
	FWeakObjectPtr WeakObject;
	FProperty* Property;
	Persistent<Context> context_;
	TMap<int32, UniquePersistent<Function>> functions;
	Persistent<Object> WrappedObject;
	Isolate* isolate_;
	int32 NextUniqueId{ 0 };
	bool bAbandoned{ false };

	bool IsValid() const
	{
		return WeakObject.IsValid();
	}

	virtual FString GetReferencerName() const
	{
		return "FJavascriptDelegate";
	}

	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		Collector.AddReferencedObjects(DelegateObjects);
	}

	FJavascriptDelegate(UObject* InObject, FProperty* InProperty)
		: WeakObject(InObject), Property(InProperty)
	{}

	~FJavascriptDelegate()
	{
		Purge();
	}

	void Purge()
	{
		if (!bAbandoned)
		{
			bAbandoned = true;

			WrappedObject.Reset();

			ClearDelegateObjects();

			context_.Reset();
		}
	}

	Local<Object> Initialize(Local<Context> context)
	{
		isolate_ = context->GetIsolate();
		context_.Reset(isolate_, context);

		auto out = Object::New(isolate_);

		auto add = [](const FunctionCallbackInfo<Value>& info) {
			for (;;)
			{
				auto payload = reinterpret_cast<FJavascriptDelegate*>(Local<External>::Cast(info.Data())->Value());
				if (info.Length() == 1)
				{
					auto func = Local<Function>::Cast(info[0]);
					if (!func.IsEmpty())
					{
						payload->Add(func);
						break;
					}
				}

				UE_LOG(LogJavascript, Log, TEXT("Invalid argument for delegate"));
				break;
			}
		};

		auto remove = [](const FunctionCallbackInfo<Value>& info) {
			for (;;)
			{
				auto payload = reinterpret_cast<FJavascriptDelegate*>(Local<External>::Cast(info.Data())->Value());
				if (info.Length() == 1)
				{
					auto func = Local<Function>::Cast(info[0]);
					if (!func.IsEmpty())
					{
						payload->Remove(func);
						break;
					}
				}

				UE_LOG(LogJavascript, Log, TEXT("Invalid argument for delegate"));
				break;
			}
		};

		auto clear = [](const FunctionCallbackInfo<Value>& info) {
			auto payload = reinterpret_cast<FJavascriptDelegate*>(Local<External>::Cast(info.Data())->Value());
			payload->Clear();			
		};

		auto toJSON = [](const FunctionCallbackInfo<Value>& info) {
			auto payload = reinterpret_cast<FJavascriptDelegate*>(Local<External>::Cast(info.Data())->Value());

			uint32_t Index = 0;		
			auto isolate_ = info.GetIsolate();
			auto context_ = isolate_->GetCurrentContext();
			auto arr = Array::New(isolate_, payload->DelegateObjects.Num());
			const bool bIsMulticastDelegate = payload->Property->IsA(FMulticastDelegateProperty::StaticClass());

			for (auto DelegateObject : payload->DelegateObjects)
			{
				auto JavascriptFunction = payload->functions.Find(DelegateObject->UniqueId);
				if (JavascriptFunction)
				{
					auto function = Local<Function>::New(isolate_, *JavascriptFunction);
					if (!bIsMulticastDelegate)
					{
						info.GetReturnValue().Set(function);
						return;
					}
					
					(void)arr->Set(context_, Index++, function);
				}
			}

			if (!bIsMulticastDelegate)
			{
				info.GetReturnValue().Set(Null(isolate_));
			}
			else
			{
				info.GetReturnValue().Set(arr);
			}			
		};

		auto data = External::New(isolate_, this);

		(void)out->Set(context, V8_KeywordString(isolate_, "Add"), Function::New(context, add, data).ToLocalChecked());
		(void)out->Set(context, V8_KeywordString(isolate_, "Remove"), Function::New(context, remove, data).ToLocalChecked());
		(void)out->Set(context, V8_KeywordString(isolate_, "Clear"), Function::New(context, clear, data).ToLocalChecked());
		(void)out->Set(context, V8_KeywordString(isolate_, "toJSON"), Function::New(context, toJSON, data).ToLocalChecked());

		WrappedObject.Reset(isolate_, out);

		return out;
	}

	TArray<UJavascriptDelegate*> DelegateObjects;

	void ClearDelegateObjects()
	{
		for (auto obj : DelegateObjects)
		{
			obj->RemoveFromRoot();
		}
		DelegateObjects.Empty();
		functions.Empty();
	}

	void Add(Local<Function> function)
	{
		auto DelegateObject = NewObject<UJavascriptDelegate>();

		DelegateObject->UniqueId = NextUniqueId++;

		Bind(DelegateObject, function);
	}

	UJavascriptDelegate* FindJavascriptDelegateByFunction(Local<Context> context, Local<Function> function)
	{
		HandleScope handle_scope(isolate_);

		bool bWasSuccessful = false;
		for (auto it = functions.CreateIterator(); it; ++it)
		{
			if (Local<Function>::New(isolate_, it.Value())->Equals(context, function).ToChecked())
			{
				for (auto obj : DelegateObjects)
				{
					if (obj->UniqueId == it.Key())
					{
						return obj;
					}
				}
			}
		}

		return nullptr;
	}

	void Remove(Local<Function> function)
	{
		auto obj = FindJavascriptDelegateByFunction(isolate_->GetCurrentContext(), function);

		if (obj)
		{
			Unbind(obj);
		}
		else
		{
			UE_LOG(LogJavascript, Log, TEXT("No match for removing delegate"));
		}
	}

	void Clear()
	{
		while (DelegateObjects.Num())
		{
			Unbind(DelegateObjects[0]);
		}
	}

	void Bind(UJavascriptDelegate* DelegateObject, Local<Function> function)
	{
		static FName NAME_Fire("Fire");

		if (WeakObject.IsValid())
		{
			if (auto p = CastField<FMulticastDelegateProperty>(Property))
			{
				FScriptDelegate Delegate;
				Delegate.BindUFunction(DelegateObject, NAME_Fire);

#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION > 22) || ENGINE_MAJOR_VERSION > 4
				p->AddDelegate(Delegate, WeakObject.Get());
#else
				auto Target = p->GetPropertyValuePtr_InContainer(WeakObject.Get());
				Target->Add(Delegate);
#endif
			}
			else if (auto p = CastField<FDelegateProperty>(Property))
			{
				auto Target = p->GetPropertyValuePtr_InContainer(WeakObject.Get());
				Target->BindUFunction(DelegateObject, NAME_Fire);
			}
		}

		DelegateObject->JavascriptDelegate = AsShared();
		DelegateObjects.Add(DelegateObject);

		functions.Add( DelegateObject->UniqueId, UniquePersistent<Function>(isolate_, function) );
	}

	void Unbind(UJavascriptDelegate* DelegateObject)
	{
		static FName NAME_Fire("Fire");

		if (WeakObject.IsValid())
		{
			if (auto p = CastField<FMulticastDelegateProperty>(Property))
			{
				FScriptDelegate Delegate;
				Delegate.BindUFunction(DelegateObject, NAME_Fire);
#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION > 22) || ENGINE_MAJOR_VERSION > 4
				p->RemoveDelegate(Delegate, WeakObject.Get());
#else
				auto Target = p->GetPropertyValuePtr_InContainer(WeakObject.Get());
				Target->Remove(Delegate);
#endif
			}
			else if (auto p = CastField<FDelegateProperty>(Property))
			{
				auto Target = p->GetPropertyValuePtr_InContainer(WeakObject.Get());
				Target->Clear();
			}
		}

		DelegateObject->JavascriptDelegate.Reset();
		DelegateObjects.Remove(DelegateObject);

		if (!bAbandoned)
		{
			functions.Remove(DelegateObject->UniqueId);
		}
	}

	UFunction* GetSignatureFunction()
	{
		if (auto p = CastField<FMulticastDelegateProperty>(Property))
		{
			return p->SignatureFunction;
		}
		else if (auto p = CastField<FDelegateProperty>(Property))
		{
			return p->SignatureFunction;
		}
		else
		{
			return nullptr;
		}
	}

	void Fire(void* Parms, UJavascriptDelegate* Delegate)
	{
		SCOPE_CYCLE_COUNTER(STAT_JavascriptDelegate);

		if (!Delegate->IsValidLowLevelFast())
		{
			return;
		}

		auto Buffer = reinterpret_cast<uint8*>(Parms);		

		auto it = functions.Find(Delegate->UniqueId);
		if (WeakObject.IsValid() && it)
		{
			Isolate::Scope isolate_scope(isolate_);
			HandleScope handle_scope(isolate_);

			auto func = Local<Function>::New(isolate_, *it);
			if (!func.IsEmpty())
			{
				auto context = Local<Context>::New(isolate_, context_);

				Context::Scope context_scope(context);

				CallJavascriptFunction(context, context->Global(), GetSignatureFunction(), func, Parms);
			}
		}
	}
};

struct FDelegateManager : IDelegateManager
{
	Isolate* isolate_;

	FDelegateManager(Isolate* isolate)
		: isolate_(isolate)
	{
		FCoreUObjectDelegates::GetPostGarbageCollect().AddRaw(this, &FDelegateManager::OnPostGarbageCollect);
	}

	virtual ~FDelegateManager()
	{
		FCoreUObjectDelegates::GetPostGarbageCollect().RemoveAll(this);

		PurgeAllDelegates();
	}

	virtual void Destroy() override
	{
		delete this;
	}

	TSet<TSharedPtr<FJavascriptDelegate>> Delegates;
	bool bGarbageCollected = false;

	void OnPostGarbageCollect()
	{
		bGarbageCollected = true;
	}

	void CollectGarbageDelegates()
	{
		for (auto it = Delegates.CreateIterator(); it; ++it)
		{
			auto d = *it;
			if (!d->IsValid())
			{
				it.RemoveCurrent();
				d.Reset();
			}
		}
	}

	void PurgeAllDelegates()
	{
		for (auto& d : Delegates)
		{
			d.Reset();
		}
		Delegates.Empty();
	}

	Local<Object> CreateDelegate(UObject* Object, FProperty* Property)
	{
		if (bGarbageCollected)
		{
			//@HACK
			CollectGarbageDelegates();
			bGarbageCollected = false;
		}

		TSharedPtr<FJavascriptDelegate> payload = MakeShareable(new FJavascriptDelegate(Object, Property));
		auto created = payload->Initialize(isolate_->GetCurrentContext());

		Delegates.Add(payload);

		return created;
	}

	virtual Local<Value> GetProxy(Local<Object> This, UObject* Object, FProperty* Property) override
	{
		auto cache_id = V8_KeywordString(isolate_, FString::Printf(TEXT("$internal_%s"), *(Property->GetName())));
		auto context_ = isolate_->GetCurrentContext();
		auto maybe_cached = This->Get(context_, cache_id);
		if (maybe_cached.IsEmpty() || maybe_cached.ToLocalChecked()->IsUndefined())
		{
			auto created = CreateDelegate(Object, Property);

			(void)This->Set(context_, cache_id, created);
			return created;
		}
		else
		{
			return maybe_cached.ToLocalChecked();
		}
	}
};

namespace v8
{
	IDelegateManager* IDelegateManager::Create(Isolate* isolate)
	{
		return new FDelegateManager(isolate);
	}
}

void UJavascriptDelegate::BeginDestroy()
{
	const bool bIsClassDefaultObject = IsTemplate(RF_ClassDefaultObject);
	if (!bIsClassDefaultObject)
	{
		JavascriptDelegate.Reset();
	}

	Super::BeginDestroy();
}

void UJavascriptDelegate::Fire()
{}

void UJavascriptDelegate::ProcessEvent(UFunction* Function, void* Parms)
{
	if (JavascriptDelegate.IsValid())
	{
		JavascriptDelegate.Pin()->Fire(Parms, this);
	}
}

PRAGMA_ENABLE_SHADOW_VARIABLE_WARNINGS