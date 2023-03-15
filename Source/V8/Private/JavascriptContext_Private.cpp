PRAGMA_DISABLE_SHADOW_VARIABLE_WARNINGS

#include "JavascriptContext_Private.h"
#include "CoreMinimal.h"
#include "JavascriptIsolate.h"
#include "HAL/FileManager.h"
#include "Config.h"
#include "Translator.h"
#include "Exception.h"
#include "Engine/Blueprint.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "JavascriptIsolate_Private.h"
#include "UObject/PropertyPortFlags.h"
#include "UObject/ScriptMacros.h"
#include "UObject/TextProperty.h"
#include "UObject/ObjectMacros.h"

#if WITH_EDITOR
#include "TypingGenerator.h"
#include "Kismet2/KismetReinstanceUtilities.h"
#endif

#include "Helpers.h"
#include "JavascriptGeneratedClass_Native.h"
#include "JavascriptGeneratedClass.h"
#include "JavascriptGeneratedFunction.h"
#include "StructMemoryInstance.h"

#include "JavascriptStats.h"

#include "../../Launch/Resources/Version.h"

using namespace v8;

static const int kContextEmbedderDataIndex = 0;
static const int32 MagicNumber = 0x2852abd3;
static const FString URL_FilePrefix(TEXT("file:///"));

static FString LocalPathToURL(FString Path)
{
	return URL_FilePrefix + Path.Replace(TEXT("\\"), TEXT("/")).Replace(TEXT(" "), TEXT("%20"));
}

static FString URLToLocalPath(FString URL)
{
	if (URL.StartsWith(*URL_FilePrefix))
	{
		URL = URL.Mid(URL_FilePrefix.Len()).Replace(TEXT("%20"), TEXT(" "));
	}
#if PLATFORM_WINDOWS
	URL = URL.Replace(TEXT("\\"), TEXT("/"));
#endif
	return URL;
}

static TArray<FString> StringArrayFromV8(Isolate* isolate, v8::Handle<Value> InArray)
{
	TArray<FString> OutArray;
	if (!InArray.IsEmpty() && InArray->IsArray())
	{
		auto arr = v8::Handle<Array>::Cast(InArray);
		auto len = arr->Length();
		auto context_ = isolate->GetCurrentContext();
		for (decltype(len) Index = 0; Index < len; ++Index)
		{
			auto maybe_value = arr->Get(context_, Index);
			if (!maybe_value.IsEmpty())
			{
				OutArray.Add(StringFromV8(isolate, maybe_value.ToLocalChecked()));
			}			
		}
	}
	return OutArray;
};

static void SetFunctionFlags(UFunction* Function, const TArray<FString>& Flags)
{
	static struct FKeyword {
		const TCHAR* Keyword;
		EFunctionFlags Flags;
	} Keywords[] = {
		{ TEXT("Exec"), FUNC_Exec },
		{ TEXT("Server"), FUNC_Net | FUNC_NetServer },
		{ TEXT("Client"), FUNC_Net | FUNC_NetClient },
		{ TEXT("NetMulticast"), FUNC_Net | FUNC_NetMulticast },
		{ TEXT("Event"), FUNC_Event },
		{ TEXT("Delegate"), FUNC_Delegate },
		{ TEXT("MulticastDelegate"), FUNC_MulticastDelegate | FUNC_Delegate },
		{ TEXT("Reliable"), FUNC_NetReliable },
		{ TEXT("Unreliable"), FUNC_NetResponse }
	};

	for (const auto& Flag : Flags)
	{
		FString Left, Right;
		if (!Flag.Split(TEXT(":"), &Left, &Right))
		{
			Left = Flag;
		}

		bool bHasMatch = false;
		for (const auto& Keyword : Keywords)
		{
			if (Left.Compare(Keyword.Keyword, ESearchCase::IgnoreCase) == 0)
			{
				Function->FunctionFlags |= Keyword.Flags;
				bHasMatch = true;
				break;
			}
		}

#if WITH_EDITOR
		if (!bHasMatch)
		{
			SetMetaData(Function, Left, Right);
		}
#endif
	}
}

static void SetClassFlags(UClass* Class, const TArray<FString>& Flags)
{
	static struct FKeyword {
		const TCHAR* Keyword;
		EClassFlags Flags;
	} Keywords[] = {
		{ TEXT("Abstract"), CLASS_Abstract },
		{ TEXT("DefaultConfig"), CLASS_DefaultConfig },
		{ TEXT("Transient"), CLASS_Transient },
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
		{ TEXT("AdvancedDisplay"), CLASS_AdvancedDisplay },
#endif
		{ TEXT("NotPlaceable"), CLASS_NotPlaceable },
		{ TEXT("PerObjectConfig"), CLASS_PerObjectConfig },
		{ TEXT("EditInlineNew"), CLASS_EditInlineNew },
		{ TEXT("CollapseCategories"), CLASS_CollapseCategories },
		{ TEXT("Const"), CLASS_Const },
		{ TEXT("DefaultToInstanced"), CLASS_DefaultToInstanced },
		{ TEXT("Hidden"), CLASS_Hidden },
		{ TEXT("HideDropDown"), CLASS_HideDropDown }
	};

	for (const auto& Flag : Flags)
	{
		FString Left, Right;
		if (!Flag.Split(TEXT(":"), &Left, &Right))
		{
			Left = Flag;
		}

		bool bHasMatch{ false };
		for (const auto& Keyword : Keywords)
		{
			if (Left.Compare(Keyword.Keyword, ESearchCase::IgnoreCase) == 0)
			{
				Class->ClassFlags |= Keyword.Flags;
				bHasMatch = true;
				break;
			}
			else if (Left.StartsWith(TEXT("Not")) && Left.Mid(3).Compare(Keyword.Keyword, ESearchCase::IgnoreCase) == 0)
			{
				Class->ClassFlags &= ~Keyword.Flags;
				bHasMatch = true;
				break;
			}
		}

#if WITH_EDITOR
		if (!bHasMatch)
		{
			SetMetaData(Class, Left, Right);
		}
#endif
	}
}

static void SetStructFlags(UScriptStruct* Struct, const TArray<FString>& Flags)
{
	static struct FKeyword {
		const TCHAR* Keyword;
		EStructFlags Flags;
	} Keywords[] = {
		{ TEXT("Atomic"), STRUCT_Atomic },
		{ TEXT("Immutable"), STRUCT_Immutable }
	};

	for (const auto& Flag : Flags)
	{
		FString Left, Right;
		if (!Flag.Split(TEXT(":"), &Left, &Right))
		{
			Left = Flag;
		}

		bool bHasMatch{ false };
		for (const auto& Keyword : Keywords)
		{
			if (Left.Compare(Keyword.Keyword, ESearchCase::IgnoreCase) == 0)
			{
				Struct->StructFlags = (EStructFlags)(Struct->StructFlags | Keyword.Flags);
				bHasMatch = true;
				break;
			}
		}


#if WITH_EDITOR
		if (!bHasMatch)
		{
			SetMetaData(Struct, Left, Right);
		}
#endif
	}
}

template<typename T>
static FProperty* CreateProperty(T* Outer, FName Name, const TArray<FString>& Decorators, FString Type, bool bIsArray, bool bIsSubclass, bool bIsMap)
{
	auto SetupProperty = [&](FProperty* NewProperty) {
		static struct FKeyword {
			const TCHAR* Keyword;
			uint64 Flags;
		} Keywords[] = {
			{ TEXT("Const"), CPF_ConstParm },
			{ TEXT("Return"), CPF_ReturnParm },
			{ TEXT("Out"), CPF_OutParm },
			{ TEXT("Replicated"), CPF_Net },
			{ TEXT("NotReplicated"), CPF_RepSkip },
			{ TEXT("ReplicatedUsing"), CPF_Net | CPF_RepNotify },
			{ TEXT("Transient"), CPF_Transient },
			{ TEXT("DuplicateTransient"), CPF_DuplicateTransient },
			{ TEXT("EditFixedSize"), CPF_EditFixedSize },
			{ TEXT("EditAnywhere"), CPF_Edit },
			{ TEXT("EditDefaultsOnly"), CPF_Edit | CPF_DisableEditOnInstance },
			{ TEXT("EditInstanceOnly"), CPF_Edit | CPF_DisableEditOnTemplate },
			{ TEXT("BlueprintReadOnly"), CPF_BlueprintVisible | CPF_BlueprintReadOnly },
			{ TEXT("BlueprintReadWrite"), CPF_BlueprintVisible },
			{ TEXT("Instanced"), CPF_PersistentInstance | CPF_ExportObject | CPF_InstancedReference },
			{ TEXT("GlobalConfig"), CPF_GlobalConfig | CPF_Config },
			{ TEXT("Config"), CPF_Config },
			{ TEXT("TextExportTransient"), CPF_TextExportTransient },
			{ TEXT("NonPIEDuplicateTransient"), CPF_NonPIEDuplicateTransient },
			{ TEXT("Export"), CPF_ExportObject },
			{ TEXT("EditFixedSize"), CPF_EditFixedSize },
			{ TEXT("NotReplicated"), CPF_RepSkip },
			{ TEXT("NonTransactional"), CPF_NonTransactional },
			{ TEXT("BlueprintAssignable"), CPF_BlueprintAssignable },
			{ TEXT("SimpleDisplay"), CPF_SimpleDisplay },
			{ TEXT("AdvancedDisplay"), CPF_AdvancedDisplay },
			{ TEXT("SaveGame"), CPF_SaveGame },
			{ TEXT("AssetRegistrySearchable"), CPF_AssetRegistrySearchable },
			{ TEXT("Interp"), CPF_Edit | CPF_Interp | CPF_BlueprintVisible },
			{ TEXT("NoClear"), CPF_NoClear },
			{ TEXT("VisibleAnywhere"), CPF_Edit | CPF_EditConst },
			{ TEXT("VisibleInstanceOnly"), CPF_Edit | CPF_EditConst | CPF_DisableEditOnTemplate },
			{ TEXT("VisibleDefaultsOnly"), CPF_Edit | CPF_EditConst | CPF_DisableEditOnInstance },
		};

		for (const auto& Flag : Decorators)
		{
			FString Left, Right;
			if (!Flag.Split(TEXT(":"), &Left, &Right))
			{
				Left = Flag;
			}


			bool bHasMatch{ false };

			for (const auto& Keyword : Keywords)
			{
				if (Left.Compare(Keyword.Keyword, ESearchCase::IgnoreCase) == 0)
				{
					NewProperty->SetPropertyFlags(static_cast<EPropertyFlags>(Keyword.Flags));

					if (Keyword.Flags & CPF_RepNotify)
					{
						NewProperty->RepNotifyFunc = FName(*Right);
					}

					bHasMatch = true;
					break;
				}
			}

#if WITH_EDITOR
			if (!bHasMatch)
			{
				SetMetaData(NewProperty, Left, Right);
			}
#endif
		}

		return NewProperty;
	};

	auto Create = [&]() -> FProperty* {
		auto ObjectFlags = EObjectFlags::RF_Public;
		auto Inner = [&](auto* Outer, const FString& Type) -> FProperty* {
			// Find TypeObject (to make UObjectHash happy)
			auto FindTypeObject = [](const TCHAR* ObjectName) -> UObject* {
				const TCHAR* PackagesToSearch[] = {
					TEXT("Engine"),
					TEXT("CoreUObject")
				};
				UObject* TypeObject = nullptr;
				for (auto PackageToSearch : PackagesToSearch)
				{
					TypeObject = StaticFindObject(UObject::StaticClass(), nullptr, *FString::Printf(TEXT("/Script/%s.%s"), PackageToSearch, ObjectName));
					if (TypeObject) return TypeObject;
				}

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
				TypeObject = StaticFindFirstObject(UObject::StaticClass(), ObjectName);
#else
				TypeObject = StaticFindObject(UObject::StaticClass(), (UObject*)ANY_PACKAGE, ObjectName);
#endif
				if (TypeObject) return TypeObject;

				TypeObject = StaticLoadObject(UObject::StaticClass(), nullptr, ObjectName);
				if (TypeObject) return TypeObject;

				return nullptr;
			};

			if (Type == FString("bool"))
			{
				auto q = new FBoolProperty(FFieldVariant(Outer), Name, ObjectFlags);
				return q;
			}
			else if (Type == FString("int"))
			{
				auto q = new FIntProperty(FFieldVariant(Outer), Name, ObjectFlags);
				return q;
			}
			else if (Type == FString("uint8"))
			{
				auto q = new FByteProperty(FFieldVariant(Outer), Name, ObjectFlags);
				return q;
			}
			else if (Type == FString("int64"))
			{
				auto q = new FInt64Property(FFieldVariant(Outer), Name, ObjectFlags);
				return q;
			}
			else if (Type == FString("string"))
			{
				auto q = new FStrProperty(FFieldVariant(Outer), Name, ObjectFlags);
				return q;
			}
			else if (Type == FString("float"))
			{
				auto q = new FFloatProperty(FFieldVariant(Outer), Name, ObjectFlags);
				return q;
			}
			else if (Type == FString("text"))
			{
				auto q = new FTextProperty(FFieldVariant(Outer), Name, ObjectFlags);
				return q;
			}
			else
			{
				UObject* TypeObject = FindTypeObject(*Type);

				if (auto p = Cast<UClass>(TypeObject))
				{
					if (bIsSubclass)
					{
						auto q = new FClassProperty(FFieldVariant(Outer), Name, ObjectFlags);
						q->SetPropertyClass(UClass::StaticClass());
						q->SetMetaClass(p);
						return q;
					}
					else
					{
						auto q = new FObjectProperty(FFieldVariant(Outer), Name, ObjectFlags);
						q->SetPropertyClass(p);
						return q;
					}
				}
				else  if (auto p = Cast<UBlueprint>(TypeObject))
				{
					if (bIsSubclass)
					{
						auto q = new FClassProperty(Outer, Name, ObjectFlags);
						q->SetPropertyClass(UClass::StaticClass());
						q->SetMetaClass(p->GeneratedClass);
						return q;
					}
					else
					{
						auto q = new FObjectProperty(FFieldVariant(Outer), Name, ObjectFlags);
						q->SetPropertyClass(p->GeneratedClass);
						return q;
					}
				}
				else if (auto p = Cast<UScriptStruct>(TypeObject))
				{
					auto q = new FStructProperty(FFieldVariant(Outer), Name, ObjectFlags);
					q->Struct = p;
					return q;
				}
				else if (auto p = Cast<UEnum>(TypeObject))
				{
					auto q = new FByteProperty(FFieldVariant(Outer), Name, ObjectFlags);
					q->Enum = p;
					return q;
				}
				else
				{
					auto q = new FInt64Property(FFieldVariant(Outer), Name, ObjectFlags);
					return q;
				}
			}
		};

		if (bIsMap)
		{
			FMapProperty* q = new FMapProperty(FFieldVariant(Outer), Name, ObjectFlags);
			FString Left, Right;
			if (Type.Split(TEXT("::"), &Left, &Right))
			{
				auto Key = FName(*Name.ToString().Append(TEXT("_Key")));
				TArray<FString> Empty;
				if (auto KeyProperty = CreateProperty(q, Key, Empty, Left, false, false, false))
				{
					q->KeyProp = KeyProperty;
					auto Value = FName(*Name.ToString().Append(TEXT("_Value")));
					if (auto ValueProperty = CreateProperty(q, Value, Decorators, Right, bIsArray, bIsSubclass, false))
					{
						q->ValueProp = ValueProperty;
					}
					//else
					//	q->MarkPendingKill();
				}
				//else
				//	q->MarkPendingKill();
			}

			return q;
		}
		else if (bIsArray)
		{
			FArrayProperty* q = new FArrayProperty(FFieldVariant(Outer), Name, ObjectFlags);
			q->Inner = SetupProperty(Inner(q, Type));
			return q;
		}
		else
		{
			return Inner(Outer, Type);
		}
	};

	return SetupProperty(Create());
}

static FProperty* CreatePropertyFromDecl(Local<Context> context, FIsolateHelper& I, UObject* Outer, v8::Handle<Value> PropertyDecl)
{
	auto Decl = PropertyDecl->ToObject(context).ToLocalChecked();
	auto Name = Decl->Get(context, I.Keyword("Name")).ToLocalChecked();
	auto Type = Decl->Get(context, I.Keyword("Type")).ToLocalChecked();
	auto Decorators = Decl->Get(context, I.Keyword("Decorators")).ToLocalChecked();
	auto IsArray = Decl->Get(context, I.Keyword("IsArray"));
	auto IsSubClass = Decl->Get(context, I.Keyword("IsSubclass"));
	auto IsMap = Decl->Get(context, I.Keyword("IsMap"));
	return CreateProperty(
		Outer,
		*StringFromV8(I.isolate_, Name),
		StringArrayFromV8(I.isolate_, Decorators),
		StringFromV8(I.isolate_, Type),
		!IsArray.IsEmpty() && IsArray.ToLocalChecked()->BooleanValue(I.isolate_),
		!IsSubClass.IsEmpty() && IsSubClass.ToLocalChecked()->BooleanValue(I.isolate_),
		!IsMap.IsEmpty() && IsMap.ToLocalChecked()->BooleanValue(I.isolate_)
		);
}

template<typename T>
static FProperty* DuplicateProperty(T* Outer, FProperty* Property, FName Name)
{
	auto SetupProperty = [&](FProperty* NewProperty) {
		NewProperty->SetPropertyFlags(Property->GetPropertyFlags());
		return NewProperty;
	};

	auto Clone = [&]() -> FProperty* {
		auto ObjectFlags = Property->GetFlags();
		if (auto p = CastField<FStructProperty>(Property))
		{
			auto q = new FStructProperty(Outer, Name, ObjectFlags);
			q->Struct = p->Struct;
			return q;
		}
		else if (auto p = CastField<FArrayProperty>(Property))
		{
			auto q = new FArrayProperty(Outer, Name, ObjectFlags);
			q->Inner = DuplicateProperty(q, p->Inner, p->Inner->GetFName());
			return q;
		}
		else if (auto p = CastField<FByteProperty>(Property))
		{
			auto q = new FByteProperty(Outer, Name, ObjectFlags);
			q->Enum = p->Enum;
			return q;
		}
		else if (auto p = CastField<FBoolProperty>(Property))
		{
			auto q = new FBoolProperty(Outer, Name, ObjectFlags);
			q->SetBoolSize(sizeof(bool), true);
			return q;
		}
		else if (auto p = CastField<FClassProperty>(Property))
		{
			auto q = new FClassProperty(Outer, Name, ObjectFlags);
			q->SetMetaClass(p->MetaClass);
			q->PropertyClass = UClass::StaticClass();
			return q;
		}
		else if (auto p = CastField<FObjectProperty>(Property))
		{
			auto q = new FObjectProperty(Outer, Name, ObjectFlags);
			q->SetPropertyClass(p->PropertyClass);
			return q;
		}
		else
		{
			//return static_cast<UProperty*>(StaticDuplicateObject(Property, Outer, *(Name.ToString())));
			return static_cast<FProperty*>(FProperty::Duplicate(Property, FFieldVariant(Outer), *(Name.ToString()), ObjectFlags));
		}
	};

	return SetupProperty(Clone());
};


namespace {
	UClass* CurrentClassUnderConstruction = nullptr;
	void CallClassConstructor(UClass* Class, const FObjectInitializer& ObjectInitializer)
	{
		if (Cast<UJavascriptGeneratedClass_Native>(Class) || Cast<UJavascriptGeneratedClass>(Class))
		{
			CurrentClassUnderConstruction = Class;
		}
		Class->ClassConstructor(ObjectInitializer);
		CurrentClassUnderConstruction = nullptr;
	};
}

class FJavascriptContextImplementation : public FJavascriptContext
{
	friend class UJavascriptContext;

	TArray<const FObjectInitializer*> ObjectInitializerStack;

	virtual const FObjectInitializer* GetObjectInitializer() override
	{
		return ObjectInitializerStack.Num() ? ObjectInitializerStack.Last(0) : nullptr;
	}

	int32 Magic{ MagicNumber };

	Persistent<Context> context_;
	IJavascriptInspector* inspector{ nullptr };

	TMap<FString, UObject*> WKOs;

public:
	Isolate* isolate() { return Environment.IsValid() ? Environment->isolate_ : nullptr; }
	Local<Context> context() { return Local<Context>::New(isolate(), context_); }
	bool IsValid() const { return Magic == MagicNumber; }

public:

	TMap<FString, UniquePersistent<Value>> Modules;
	TArray<FString>& Paths;

	bool IsDebugContext() const
	{
		return inspector != nullptr;
	}

	void CreateInspector(int32 Port)
	{
		if (inspector) return;

		Isolate::Scope isolate_scope(isolate());
		HandleScope handle_scope(isolate());

		inspector = IJavascriptInspector::Create(Port, context());
	}

	void DestroyInspector()
	{
		if (inspector)
		{
			Isolate::Scope isolate_scope(isolate());
			HandleScope handle_scope(isolate());

			inspector->Destroy();
			inspector = nullptr;
		}
	}

	FJavascriptContextImplementation(TSharedPtr<FJavascriptIsolate> InEnvironment, TArray<FString>& InPaths)
		: FJavascriptContext(InEnvironment), Paths(InPaths)
	{
		Isolate::Scope isolate_scope(isolate());
		HandleScope handle_scope(isolate());

		auto context = Context::New(isolate(), nullptr, Environment->GetGlobalTemplate());
		context->SetAlignedPointerInEmbedderData(kContextEmbedderDataIndex, this);

		context_.Reset(isolate(), context);

		ExposeGlobals();

		Paths = IV8::Get().GetGlobalScriptSearchPaths();

		PostWorldCleanupHandle = FWorldDelegates::OnPostWorldCleanup.AddRaw(this, &FJavascriptContextImplementation::OnWorldCleanup);
	}

	~FJavascriptContextImplementation()
	{
		PurgeModules();

		ReleaseAllPersistentHandles();

		DestroyInspector();

		context_.Reset();

		FWorldDelegates::OnPostWorldCleanup.Remove(PostWorldCleanupHandle);
	}

	void ReleaseAllPersistentHandles()
	{
		// Release all object instances
		ObjectToObjectMap.Empty();

		// Release all struct instances
		MemoryToObjectMap.Empty();
	}

	void ExposeGlobals()
	{
		HandleScope handle_scope(isolate());
		Context::Scope context_scope(context());

		ExposeRequire();
		ExportUnrealEngineClasses();
		ExportUnrealEngineStructs();

		ExposeMemory2();
		ExposeVersions();		
	}

	void PurgeModules()
	{
		Modules.Empty();
	}

	void ExposeVersions()
	{
		FIsolateHelper I(isolate());

		auto ctx = context();
		auto global = ctx->Global();

		(void)global->Set(ctx, I.Keyword("$engineVersion"), I.String(ENGINE_VERSION_STRING));
		(void)global->Set(ctx, I.Keyword("$engineMajorVersion"), I.String(VERSION_STRINGIFY(ENGINE_MAJOR_VERSION)));
		(void)global->Set(ctx, I.Keyword("$engineMinorVersion"), I.String(VERSION_STRINGIFY(ENGINE_MINOR_VERSION)));
		(void)global->Set(ctx, I.Keyword("$enginePatchVersion"), I.String(VERSION_STRINGIFY(ENGINE_PATCH_VERSION)));
	}

	void ExportUnrealEngineClasses()
	{
		auto fn = [](const FunctionCallbackInfo<Value>& info) {
			auto start = FPlatformTime::Seconds();

			auto Context = reinterpret_cast<FJavascriptContextImplementation*>((Local<External>::Cast(info.Data()))->Value());

			auto isolate = info.GetIsolate();

			FIsolateHelper I(isolate);

			HandleScope scope(isolate);
			auto context = Context->context();
			auto Name = StringFromV8(isolate, info[0]);
			auto Opts = info[1]->ToObject(context).ToLocalChecked();
			auto Outer = UObjectFromV8(context, Opts->Get(context, I.Keyword("Outer")).ToLocalChecked());
			auto Archetype = UObjectFromV8(context, Opts->Get(context, I.Keyword("Archetype")).ToLocalChecked());
			auto ParentClass = UClassFromV8(isolate, Opts->Get(context, I.Keyword("Parent")).ToLocalChecked());
			auto NonNative = Opts->Get(context, I.Keyword("NonNative")).ToLocalChecked()->BooleanValue(isolate);
			Outer = Outer ? Outer : GetTransientPackage();
			ParentClass = ParentClass ? ParentClass : UObject::StaticClass();

			UBlueprintGeneratedClass* Class = nullptr;
			if (NonNative || Cast<UBlueprintGeneratedClass>(ParentClass))
			{
				auto Klass = NewObject<UJavascriptGeneratedClass>(Outer, *Name, RF_Public);
				Klass->JavascriptContext = Context->AsShared();
				Class = Klass;
			}
			else
			{
				auto Klass = NewObject<UJavascriptGeneratedClass_Native>(Outer, *Name, RF_Public);
				Klass->JavascriptContext = Context->AsShared();
				Class = Klass;

				// This flag is necessary for proper initialization
				Class->ClassFlags |= CLASS_Native;
			}

			// Create a blueprint
			auto Blueprint = NewObject<UBlueprint>(Outer);
			Blueprint->GeneratedClass = Class;
#if WITH_EDITORONLY_DATA
			Class->ClassGeneratedBy = Blueprint;
#endif
			auto ClassConstructor = [](const FObjectInitializer& ObjectInitializer){
				auto Class = static_cast<UBlueprintGeneratedClass*>(CurrentClassUnderConstruction ? CurrentClassUnderConstruction : ObjectInitializer.GetClass());
				CurrentClassUnderConstruction = nullptr;

				FJavascriptContextImplementation* Context = nullptr;

				if (auto Klass = Cast<UJavascriptGeneratedClass_Native>(Class))
				{
					if (Klass->JavascriptContext.IsValid())
					{
						Context = static_cast<FJavascriptContextImplementation*>(Klass->JavascriptContext.Pin().Get());
					}
				}
				else if (auto Klass = Cast<UJavascriptGeneratedClass>(Class))
				{
					if (Klass->JavascriptContext.IsValid())
					{
						Context = static_cast<FJavascriptContextImplementation*>(Klass->JavascriptContext.Pin().Get());
					}
				}

				if (Context)
				{
					auto isolate = Context->isolate();
					auto Object = ObjectInitializer.GetObj();

					FIsolateHelper I(isolate);

					Isolate::Scope isolate_scope(isolate);
					HandleScope handle_scope(isolate);
					Context::Scope context_scope(Context->context());

					auto Holder = Context->ExportObject(Class);
					auto context = Context->context();

					auto v8_obj = Holder->ToObject(context).ToLocalChecked();
					auto maybe_proxy = v8_obj->Get(context, I.Keyword("proxy"));
					if (maybe_proxy.IsEmpty())
					{
						I.Throw(TEXT("Invalid proxy : construct class"));
						//@todo : assertion
						return;
					}

					auto proxy = maybe_proxy.ToLocalChecked();

					if (!proxy->IsObject())
					{
						I.Throw(TEXT("Invalid proxy : construct class"));
						//@todo : assertion
						return;
					}

					Context->ObjectInitializerStack.Add(&ObjectInitializer);

					auto This = Context->ExportObject(Object);

					{
						auto func = proxy->ToObject(context).ToLocalChecked()->Get(context, I.Keyword("prector")).ToLocalChecked();

						if (func->IsFunction())
						{
							CallJavascriptFunction(context, This, nullptr, Local<Function>::Cast(func), nullptr);
						}
					}

					CallClassConstructor(Class->GetSuperClass(), ObjectInitializer);

					// move to javascriptgeneratedclass_*
// 					{
// 						auto func = proxy->ToObject(context).ToLocalChecked()->Get(context, I.Keyword("ctor")).ToLocalChecked();
// 						if (func->IsFunction())
// 						{
// 							CallJavascriptFunction(context, This, nullptr, Local<Function>::Cast(func), nullptr);
// 						}
// 					}

					Context->ObjectInitializerStack.RemoveAt(Context->ObjectInitializerStack.Num() - 1, 1);
				}
				else
				{
					CallClassConstructor(Class->GetSuperClass(), ObjectInitializer);
				}
			};

			Class->ClassConstructor = ClassConstructor;

			// Set properties we need to regenerate the class with
			Class->PropertyLink = ParentClass->PropertyLink;
			Class->ClassWithin = ParentClass->ClassWithin;
			Class->ClassConfigName = ParentClass->ClassConfigName;

			Class->SetSuperStruct(ParentClass);
			Class->ClassFlags |= (ParentClass->ClassFlags & (CLASS_Inherit | CLASS_ScriptInherit | CLASS_CompiledFromBlueprint));
			Class->ClassCastFlags |= ParentClass->ClassCastFlags;

			auto AddFunction = [&](FName NewFunctionName, v8::Handle<Value> TheFunction) -> bool {
				UFunction* ParentFunction = ParentClass->FindFunctionByName(NewFunctionName);

				UJavascriptGeneratedFunction* Function{ nullptr };

				auto MakeFunction = [&]() {
					Function = NewObject<UJavascriptGeneratedFunction>(Class, NewFunctionName, RF_Public);
					Function->JavascriptContext = Context->AsShared();
					//Function->RepOffset = MAX_uint16;
					Function->ReturnValueOffset = MAX_uint16;
					Function->FirstPropertyToInit = nullptr;

					Function->Script.Add(EX_EndFunctionParms);
				};

				// Overridden function should have its parent function
				if (ParentFunction)
				{
					MakeFunction();

					Function->SetSuperStruct(ParentFunction);

					auto InitializeProperties = [](UFunction* Function, UFunction* ParentFunction) {
						FField** Storage = &Function->ChildProperties;
						FProperty** PropertyStorage = &Function->PropertyLink;

						for (TFieldIterator<FProperty> PropIt(ParentFunction, EFieldIteratorFlags::ExcludeSuper); PropIt; ++PropIt)
						{
							FProperty* Property = *PropIt;
							if (Property->HasAnyPropertyFlags(CPF_Parm))
							{
								FProperty* NewProperty = DuplicateProperty(Function, Property, Property->GetFName());

								*Storage = NewProperty;
								Storage = &NewProperty->Next;

								*PropertyStorage = NewProperty;
								PropertyStorage = &NewProperty->PropertyLinkNext;
							}
						}
					};

					if (ParentFunction)
					{
						InitializeProperties(Function, ParentFunction);
					}
				}
				else
				{
					auto FunctionObj = TheFunction->ToObject(context).ToLocalChecked();
					auto Maybe_IsUFUNCTION = FunctionObj->Get(context, I.Keyword("IsUFUNCTION"));
					if (Maybe_IsUFUNCTION.IsEmpty())
					{
						return false;
					}
					
					auto IsUFUNCTION = Maybe_IsUFUNCTION.ToLocalChecked();

					if (!IsUFUNCTION->BooleanValue(isolate))
					{
						return false;
					}

					MakeFunction();

					auto Decorators = FunctionObj->Get(context, I.Keyword("Decorators"));
					if (!Decorators.IsEmpty())
					{
						auto CheckedDecorators = Decorators.ToLocalChecked();
						if (CheckedDecorators->IsArray())
						{
							SetFunctionFlags(Function, StringArrayFromV8(isolate, CheckedDecorators));
						}
					}

					auto InitializeProperties = [&](UFunction* Function, v8::Handle<Value> Signature) {
						FField** Storage = &Function->ChildProperties;
						FProperty** PropertyStorage = &Function->PropertyLink;

						if (!Signature.IsEmpty() && Signature->IsArray())
						{
							auto arr = v8::Handle<Array>::Cast(Signature);
							auto len = arr->Length();

							for (decltype(len) Index = 0; Index < len; ++Index)
							{
								auto PropertyDecl = arr->Get(context, Index);
								FProperty* NewProperty = nullptr;
								if (!PropertyDecl.IsEmpty())
								{
									NewProperty = CreatePropertyFromDecl(context, I, Function, PropertyDecl.ToLocalChecked());
								}

								if (NewProperty)
								{
									NewProperty->SetPropertyFlags(CPF_Parm);

									*Storage = NewProperty;
									Storage = &NewProperty->Next;

									*PropertyStorage = NewProperty;
									PropertyStorage = &NewProperty->PropertyLinkNext;
								}
							}
						}
					};

					auto Signature = FunctionObj->Get(context, I.Keyword("Signature")).ToLocalChecked();

					InitializeProperties(Function, Signature);
				}

				auto FinalizeFunction = [](UFunction* Function) {
					Function->Bind();
					Function->StaticLink(true);

					Function->FunctionFlags |= FUNC_Native;

					for (TFieldIterator<FProperty> PropIt(Function, EFieldIteratorFlags::ExcludeSuper); PropIt; ++PropIt)
					{
						FProperty* Property = *PropIt;
						if (Property->HasAnyPropertyFlags(CPF_Parm))
						{
							++Function->NumParms;
							Function->ParmsSize = Property->GetOffset_ForUFunction() + Property->GetSize();

							if (Property->HasAnyPropertyFlags(CPF_OutParm))
							{
								Function->FunctionFlags |= FUNC_HasOutParms;
							}

							if (Property->HasAnyPropertyFlags(CPF_ReturnParm))
							{
								Function->ReturnValueOffset = Property->GetOffset_ForUFunction();

								if (!Property->HasAnyPropertyFlags(CPF_IsPlainOldData | CPF_NoDestructor))
								{
									Property->DestructorLinkNext = Function->DestructorLink;
									Function->DestructorLink = Property;
								}
							}
						}
						else
						{
							if (!Property->HasAnyPropertyFlags(CPF_ZeroConstructor))
							{
								Function->FirstPropertyToInit = Property;
								Function->FunctionFlags |= FUNC_HasDefaults;
								break;
							}
						}
					}
				};

				FinalizeFunction(Function);

				Function->SetNativeFunc(&UJavascriptGeneratedFunction::Thunk);

				Function->Next = Class->Children;
				Class->Children = Function;

				// Add the function to it's owner class function name -> function map
				Class->AddFunctionToFunctionMap(Function, Function->GetFName());

				return true;
			};

			auto maybe_ClassFlags = Opts->Get(context, I.Keyword("ClassFlags"));
			if (!maybe_ClassFlags.IsEmpty())
			{
				auto ClassFlags = maybe_ClassFlags.ToLocalChecked();
				if (ClassFlags->IsArray())
				{
					SetClassFlags(Class, StringArrayFromV8(isolate, ClassFlags));
				}
			}

			auto maybe_PropertyDecls = Opts->Get(context, I.Keyword("Properties"));
			if (!maybe_PropertyDecls.IsEmpty())
			{
				auto PropertyDecls = maybe_PropertyDecls.ToLocalChecked();

				if (PropertyDecls->IsArray())
				{
					auto arr = v8::Handle<Array>::Cast(PropertyDecls);
					auto len = arr->Length();

					for (decltype(len) Index = 0; Index < len; ++Index)
					{
						auto maybe_PropertyDecl = arr->Get(context, len - Index - 1);
						if (!maybe_PropertyDecl.IsEmpty())
						{
							auto PropertyDecl = maybe_PropertyDecl.ToLocalChecked();
							if (PropertyDecl->IsObject())
							{
								auto Property = CreatePropertyFromDecl(context, I, Class, PropertyDecl);

								if (Property)
								{
									Class->AddCppProperty(Property);

									if (Property->HasAnyPropertyFlags(CPF_Net))
									{
										Class->NumReplicatedProperties++;
									}
								}
							}
						}
					}
				}
			}

			auto maybe_Functions = Opts->Get(context, I.Keyword("Functions"));
			TMap<FString, v8::Handle<Value>> Others;
			if (!maybe_Functions.IsEmpty())
			{
				auto Functions = maybe_Functions.ToLocalChecked();
				if (Functions->IsObject())
				{
					auto FuncMap = Functions->ToObject(context).ToLocalChecked();
					auto Keys = FuncMap->GetOwnPropertyNames(context).ToLocalChecked();

					auto NumKeys = Keys->Length();

					for (decltype(NumKeys) Index = 0; Index < NumKeys; ++Index)
					{
						auto maybe_Name = Keys->Get(context, Index);
						if (!maybe_Name.IsEmpty())
						{
							auto Name = maybe_Name.ToLocalChecked();
							auto UName = StringFromV8(isolate, Name);
							auto maybe_Function = FuncMap->Get(context, Name);

							if (maybe_Function.IsEmpty()) continue;

							auto Function = maybe_Function.ToLocalChecked();
							if (!Function->IsFunction()) continue;

							if (UName != TEXT("prector") && UName != TEXT("ctor") && UName != TEXT("constructor"))
							{
								if (!AddFunction(*UName, Function))
								{
									Others.Add(UName, Function);
								}
							}
						}
					}
				}

			}

			Class->Bind();
			Class->StaticLink(true);

			{
				auto FinalClass = Context->Environment->ExportUClass(Class,false);
				auto Prototype = FinalClass->PrototypeTemplate();

				for (auto It = Others.CreateIterator(); It; ++It)
				{
					Prototype->Set(I.Keyword(It.Key()), It.Value());
				}

				Context->Environment->RegisterUClass(Class, FinalClass);
			}

			auto FinalClass = Context->ExportObject(Class);
			if (!maybe_Functions.IsEmpty())
			{
				(void)FinalClass->ToObject(context).ToLocalChecked()->Set(context, I.Keyword("proxy"), maybe_Functions.ToLocalChecked());
			}

			info.GetReturnValue().Set(FinalClass);

			// Make sure CDO is ready for use
			if (Archetype)
				Class->ClassDefaultObject = Archetype;
			else
				Class->GetDefaultObject();

#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION > 12) || ENGINE_MAJOR_VERSION > 4
			// Assemble reference token stream for garbage collection/ RTGC.
			if (!Class->HasAnyClassFlags(CLASS_TokenStreamAssembled))
			{
				Class->AssembleReferenceTokenStream();
			}
#endif
			auto end = FPlatformTime::Seconds();
			UE_LOG(LogJavascript, Warning, TEXT("Create UClass(%s) Elapsed: %.6f"), *Name, end - start);
		};

		auto fn1 = [](const FunctionCallbackInfo<Value>& info) {
#if WITH_EDITOR
			auto start = FPlatformTime::Seconds();
			auto Context = reinterpret_cast<FJavascriptContextImplementation*>((Local<External>::Cast(info.Data()))->Value());

			auto isolate = info.GetIsolate();

			FIsolateHelper I(isolate);

			HandleScope scope(isolate);
			auto context = Context->context();
			auto Opts = info[0]->ToObject(context).ToLocalChecked();
			auto Class = (UBlueprintGeneratedClass*)UClassFromV8(isolate, Opts->Get(context, I.Keyword("SelfClass")).ToLocalChecked());

			// Recreate the CDO after rebind properties.
			TMap<UClass*, UClass*> OldToNewMap;
			FBlueprintCompileReinstancer::MoveCDOToNewClass(Class, OldToNewMap, true);
			Class->ClassDefaultObject = nullptr;

			Class->Children = nullptr;
			auto maybe_PropertyDecls = Opts->Get(context, I.Keyword("Properties"));

			if (!maybe_PropertyDecls.IsEmpty())
			{
				auto PropertyDecls = maybe_PropertyDecls.ToLocalChecked();
				if (PropertyDecls->IsArray())
				{
					auto arr = v8::Handle<Array>::Cast(PropertyDecls);
					auto len = arr->Length();

					for (decltype(len) Index = 0; Index < len; ++Index)
					{
						auto maybe_PropertyDecl = arr->Get(context, len - Index - 1);
						if (!maybe_PropertyDecl.IsEmpty())
						{
							auto PropertyDecl = maybe_PropertyDecl.ToLocalChecked();
							if (PropertyDecl->IsObject())
							{
								auto Property = CreatePropertyFromDecl(context, I, Class, PropertyDecl);

								if (Property)
								{
									Class->AddCppProperty(Property);

									if (Property->HasAnyPropertyFlags(CPF_Net))
									{
										Class->NumReplicatedProperties++;
									}
								}
							}
						}
					}
				}
			}

			Class->Bind();
			Class->StaticLink(true);

			// @note: caching target class's proxy function and adjust to reexported class.
			auto prev_v8_template = Context->ExportObject(Class);
			auto ProxyFunctions = prev_v8_template->ToObject(context).ToLocalChecked()->Get(context, I.Keyword("proxy")).ToLocalChecked();

			auto maybe_Functions = Opts->Get(context, I.Keyword("Functions"));
			if (!maybe_Functions.IsEmpty())
			{
				auto Functions = maybe_Functions.ToLocalChecked();
				TMap<FString, v8::Handle<Value>> Others;
				if (!Functions.IsEmpty() && Functions->IsObject())
				{
					auto FuncMap = Functions->ToObject(context).ToLocalChecked();
					auto Function0 = FuncMap->Get(context, I.Keyword("ctor")).ToLocalChecked();
					auto Function1 = FuncMap->Get(context, I.Keyword("prector")).ToLocalChecked();

					auto ProxyFuncMap = ProxyFunctions->ToObject(context).ToLocalChecked();
					(void)ProxyFuncMap->Set(context, I.Keyword("ctor"), Function0);
					(void)ProxyFuncMap->Set(context, I.Keyword("prector"), Function1);
				}
			}
			Context->Environment->PublicExportUClass(Class);

			auto aftr_v8_template = Context->ExportObject(Class);
			(void)(aftr_v8_template->ToObject(context).ToLocalChecked()->Set(context, I.Keyword("proxy"), ProxyFunctions));

			Class->GetDefaultObject(true);

#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION > 12) || ENGINE_MAJOR_VERSION > 4
			// Assemble reference token stream for garbage collection/ RTGC.
			Class->AssembleReferenceTokenStream(true);
#endif
			auto end = FPlatformTime::Seconds();
			UE_LOG(LogJavascript, Warning, TEXT("Rebind UClass(%s) Elapsed: %.6f"), *Class->GetName(), end - start);
#endif
		};

		auto ctx = context();
		auto global = ctx->Global();
		auto self = External::New(isolate(), this);

		(void)global->Set(ctx, V8_KeywordString(isolate(), "CreateClass"), FunctionTemplate::New(isolate(), FV8Exception::GuardLambda(fn), self)->GetFunction(ctx).ToLocalChecked());
		(void)global->Set(ctx, V8_KeywordString(isolate(), "RebindClassProperties"), FunctionTemplate::New(isolate(), FV8Exception::GuardLambda(fn1), self)->GetFunction(ctx).ToLocalChecked());
	}

	void ExportUnrealEngineStructs()
	{
		auto fn = [](const FunctionCallbackInfo<Value>& info) {
			auto start  = FPlatformTime::Seconds();

			auto Context = reinterpret_cast<FJavascriptContextImplementation*>((Local<External>::Cast(info.Data()))->Value());

			auto isolate = info.GetIsolate();

			FIsolateHelper I(isolate);

			HandleScope scope(isolate);
			auto context = Context->context();
			auto Name = StringFromV8(isolate, info[0]);
			auto Opts = info[1]->ToObject(context).ToLocalChecked();
			auto Outer = UObjectFromV8(context, Opts->Get(context, I.Keyword("Outer")).ToLocalChecked());
			auto ParentStruct = (UScriptStruct*)UClassFromV8(isolate, Opts->Get(context, I.Keyword("Parent")).ToLocalChecked());
			Outer = Outer ? Outer : GetTransientPackage();

			UScriptStruct* Struct = nullptr;
			{
				auto Klass = NewObject<UScriptStruct>(Outer, *Name, RF_Public);
				Struct = Klass;
			}

			// Set properties we need to regenerate the class with
			if (ParentStruct)
			{
				Struct->PropertyLink = ParentStruct->PropertyLink;
				Struct->SetSuperStruct(ParentStruct);
				Struct->StructFlags = (EStructFlags)(ParentStruct->StructFlags & STRUCT_Inherit);
			}

			auto StructFlags = Opts->Get(context, I.Keyword("StructFlags")).ToLocalChecked();
			if (!StructFlags.IsEmpty() && StructFlags->IsArray())
			{
				SetStructFlags(Struct, StringArrayFromV8(isolate, StructFlags));
			}

			auto PropertyDecls = Opts->Get(context, I.Keyword("Properties")).ToLocalChecked();
			if (!PropertyDecls.IsEmpty() && PropertyDecls->IsArray())
			{
				auto arr = v8::Handle<Array>::Cast(PropertyDecls);
				auto len = arr->Length();

				for (decltype(len) Index = 0; Index < len; ++Index)
				{
					auto maybe_PropertyDecl = arr->Get(context, len - Index - 1);
					if (!maybe_PropertyDecl.IsEmpty())
					{
						auto PropertyDecl = maybe_PropertyDecl.ToLocalChecked();
						if (PropertyDecl->IsObject())
						{
							auto Property = CreatePropertyFromDecl(context, I, Struct, PropertyDecl);

							if (Property)
							{
								Struct->AddCppProperty(Property);
							}
						}
					}
				}
			}

			Struct->Bind();
			Struct->StaticLink(true);

			auto FinalClass = Context->ExportObject(Struct);

			info.GetReturnValue().Set(FinalClass);

			auto end = FPlatformTime::Seconds();
			UE_LOG(LogJavascript, Warning, TEXT("Create UStruct(%s) Elapsed: %.6f"), *Name, end - start);
		};

		auto fn1 = [](const FunctionCallbackInfo<Value>& info) {
#if WITH_EDITOR
			auto start = FPlatformTime::Seconds();
			auto Context = reinterpret_cast<FJavascriptContextImplementation*>((Local<External>::Cast(info.Data()))->Value());
	
			auto isolate = info.GetIsolate();

			FIsolateHelper I(isolate);

			HandleScope scope(isolate);

			auto context = Context->context();
			auto Opts = info[0]->ToObject(context).ToLocalChecked();
			auto Struct = (UScriptStruct*)UClassFromV8(isolate, Opts->Get(context, I.Keyword("SelfStruct")).ToLocalChecked());

			Struct->Children = nullptr;
			auto PropertyDecls = Opts->Get(context, I.Keyword("Properties")).ToLocalChecked();
			if (!PropertyDecls.IsEmpty() && PropertyDecls->IsArray())
			{
				auto arr = v8::Handle<Array>::Cast(PropertyDecls);
				auto len = arr->Length();

				for (decltype(len) Index = 0; Index < len; ++Index)
				{
					auto maybe_PropertyDecl = arr->Get(context, len - Index - 1);
					if (maybe_PropertyDecl.IsEmpty()) continue;
					auto PropertyDecl = maybe_PropertyDecl.ToLocalChecked();
					if (PropertyDecl->IsObject())
					{
						auto Property = CreatePropertyFromDecl(context, I, Struct, PropertyDecl);

						if (Property)
						{
							Struct->AddCppProperty(Property);
						}
					}
				}
			}

			Struct->Bind();
			Struct->StaticLink(true);

			// @note: caching target class's proxy function and adjust to reexported class.
			auto prev_v8_template = Context->ExportObject(Struct);
			auto ProxyFunctions = prev_v8_template->ToObject(context).ToLocalChecked()->Get(context, I.Keyword("proxy")).ToLocalChecked();

			Context->Environment->PublicExportStruct(Struct);

			auto aftr_v8_template = Context->ExportObject(Struct);
			(void)(aftr_v8_template->ToObject(context).ToLocalChecked()->Set(context, I.Keyword("proxy"), ProxyFunctions));
			auto end = FPlatformTime::Seconds();
			UE_LOG(LogJavascript, Warning, TEXT("Rebind UStruct(%s) Elapsed: %.6f"), *Struct->GetName(), end - start);
#endif
		};

		auto ctx = context();
		auto global = ctx->Global();
		auto self = External::New(isolate(), this);

		(void)global->Set(ctx, V8_KeywordString(isolate(), "CreateStruct"), FunctionTemplate::New(isolate(), FV8Exception::GuardLambda(fn), self)->GetFunction(ctx).ToLocalChecked());
		(void)global->Set(ctx, V8_KeywordString(isolate(), "RebindStructProperties"), FunctionTemplate::New(isolate(), FV8Exception::GuardLambda(fn1), self)->GetFunction(ctx).ToLocalChecked());
	}

	void ExposeRequire()
	{
		auto fn = [](const FunctionCallbackInfo<Value>& info) {
			auto isolate = info.GetIsolate();
			HandleScope scope(isolate);

			if (info.Length() != 1 || !(info[0]->IsString()))
			{
				return;
			}

			auto Self = reinterpret_cast<FJavascriptContextImplementation*>((Local<External>::Cast(info.Data()))->Value());

			auto required_module = StringFromV8(isolate, info[0]);

			bool found = false;

			auto inner = [&](const FString& script_path)
			{
				auto relative_path = script_path.Replace(TEXT("/./"), TEXT("/"), ESearchCase::CaseSensitive);

				// path referencing .. cleansing
				int pos = relative_path.Len();
				while (true)
				{
					int idx = relative_path.Find("/../", ESearchCase::CaseSensitive, ESearchDir::FromEnd, pos);
					if (idx < 0)
						break;

					int parentIdx = relative_path.Find("/", ESearchCase::CaseSensitive, ESearchDir::FromEnd, idx) + 1;
					FString parent = relative_path.Mid(parentIdx, idx - parentIdx);
					if (parent == "..")
					{
						pos = idx + 1;
						continue;
					}

					relative_path = relative_path.Mid(0, parentIdx) + relative_path.Mid(idx + 4);
					pos = relative_path.Len(); // reset counter
				}

				auto full_path = FPaths::ConvertRelativePathToFull(relative_path);
#if PLATFORM_WINDOWS
				full_path = full_path.Replace(TEXT("/"), TEXT("\\"));
#endif
				auto it = Self->Modules.Find(full_path);
				if (it)
				{
					info.GetReturnValue().Set(Local<Value>::New(isolate, *it));
					found = true;
					return true;
				}

				FString Text;
				if (FFileHelper::LoadFileToString(Text, *relative_path))
				{
					Text = FString::Printf(TEXT("(function (global, __filename, __dirname) { var module = { exports : {}, filename : __filename }, exports = module.exports; (function () { %s\n })()\n;return module.exports;}(this,'%s', '%s'));"), *Text, *relative_path, *FPaths::GetPath(relative_path));
					auto exports = Self->RunScript(full_path, Text, 0);
					if (exports.IsEmpty())
					{
						UE_LOG(LogJavascript, Log, TEXT("Invalid script for require"));
					}
					Self->Modules.Add(full_path, UniquePersistent<Value>(isolate, exports));
					info.GetReturnValue().Set(exports);
					found = true;
					return true;
				}

				return false;
			};

			auto inner_maybejs = [&](const FString& script_path)
			{
				if (!script_path.EndsWith(TEXT(".js")))
				{
					return inner(script_path + TEXT(".js"));
				}
				else
				{
					return inner(script_path);
				}
			};

			auto inner_package_json = [&](const FString& script_path)
			{
				FString Text;
				if (FFileHelper::LoadFileToString(Text, *(script_path / TEXT("package.json"))))
				{
					Text = FString::Printf(TEXT("(function (json) {return json.main;})(%s);"), *Text);
					auto full_path = FPaths::ConvertRelativePathToFull(script_path);
#if PLATFORM_WINDOWS
					full_path = full_path.Replace(TEXT("/"), TEXT("\\"));
#endif
					auto exports = Self->RunScript(full_path, Text, 0);
					if (exports.IsEmpty() || !exports->IsString())
					{
						return false;
					}
					else
					{
						return inner_maybejs(script_path / StringFromV8(isolate, exports));
					}
				}

				return false;
			};

			auto inner_json = [&](const FString& script_path)
			{
				auto full_path = FPaths::ConvertRelativePathToFull(script_path);
#if PLATFORM_WINDOWS
				full_path = full_path.Replace(TEXT("/"), TEXT("\\"));
#endif
				auto it = Self->Modules.Find(full_path);
				if (it)
				{
					info.GetReturnValue().Set(Local<Value>::New(isolate, *it));
					found = true;
					return true;
				}

				FString Text;
				if (FFileHelper::LoadFileToString(Text, *script_path))
				{
					Text = FString::Printf(TEXT("(function (json) {return json;})(%s);"), *Text);

#if PLATFORM_WINDOWS
					full_path = full_path.Replace(TEXT("/"), TEXT("\\"));
#endif
					auto exports = Self->RunScript(full_path, Text, 0);
					if (exports.IsEmpty() || !exports->IsObject())
					{
						return false;
					}
					else
					{
						Self->Modules.Add(full_path, UniquePersistent<Value>(isolate, exports));
						info.GetReturnValue().Set(exports);
						found = true;
						return true;
					}
				}

				return false;
			};

			auto inner2 = [&](FString base_path)
			{
				if (!FPaths::DirectoryExists(base_path)) return false;

				auto script_path = base_path / required_module;
				if (script_path.EndsWith(TEXT(".js")))
				{
					if (inner(script_path)) return true;
				}
				else
				{
					if (inner(script_path + TEXT(".js"))) return true;
				}
				if (script_path.EndsWith(TEXT(".json")))
				{
					if (inner_json(script_path)) return true;
				}
				else
				{
					if (inner_json(script_path + TEXT(".json"))) return true;
				}

				if (inner(script_path / TEXT("index.js"))) return true;
				if (inner_package_json(script_path)) return true;

				return false;
			};

			auto load_module_paths = [&](FString base_path)
			{
				TArray<FString> Dirs;
				TArray<FString> Parsed;
				base_path.ParseIntoArray(Parsed, TEXT("/"));
				auto PartCount = Parsed.Num();
				while (PartCount > 0) {
					if (Parsed[PartCount-1].Equals(TEXT("node_modules")))
					{
						PartCount--;
						continue;
					}
					else
					{
						TArray<FString> Parts;
						for (int i = 0; i < PartCount; i++) Parts.Add(Parsed[i]);
						FString Dir = FString::Join(Parts, TEXT("/"));
						Dirs.Add(Dir);
					}
					PartCount--;
				}

				return Dirs;
			};

			if (inner(required_module))
				return;

			auto current_script_path = FPaths::GetPath(StringFromV8(isolate, StackTrace::CurrentStackTrace(isolate, 1, StackTrace::kScriptName)->GetFrame(isolate, 0)->GetScriptName()));
			current_script_path = URLToLocalPath(current_script_path);

			if (!(required_module[0] == '.' && inner2(current_script_path)))
			{
				for (const auto& path : load_module_paths(current_script_path))
				{
					if (inner2(path)) break;
					if (inner2(path / TEXT("node_modules"))) break;
				}

				for (const auto& path : Self->Paths)
				{
					if (inner2(path)) break;
					if (inner2(path / TEXT("node_modules"))) break;
				}
			}

			if (!found)
			{
				UE_LOG(LogJavascript, Warning, TEXT("Undefined required script '%s'"), *required_module);
				info.GetReturnValue().Set(v8::Undefined(isolate));
			}
		};

		auto fn2 = [](const FunctionCallbackInfo<Value>& info) {
			auto isolate = info.GetIsolate();
			HandleScope scope(isolate);

			auto Self = reinterpret_cast<FJavascriptContextImplementation*>((Local<External>::Cast(info.Data()))->Value());
			Self->PurgeModules();
		};
		auto ctx = context();
		auto global = ctx->Global();
		auto self = External::New(isolate(), this);

		(void)global->Set(ctx, V8_KeywordString(isolate(), "require"), FunctionTemplate::New(isolate(), FV8Exception::GuardLambda(fn), self)->GetFunction(ctx).ToLocalChecked());
		(void)global->Set(ctx, V8_KeywordString(isolate(), "purge_modules"), FunctionTemplate::New(isolate(), FV8Exception::GuardLambda(fn2), self)->GetFunction(ctx).ToLocalChecked());

		AccessorNameGetterCallback getter = [](Local<Name> property, const PropertyCallbackInfo<Value>& info) {
			auto isolate = info.GetIsolate();
			HandleScope scope(isolate);

			auto Self = reinterpret_cast<FJavascriptContextImplementation*>((Local<External>::Cast(info.Data()))->Value());

			auto out = Object::New(isolate);

			auto context_ = isolate->GetCurrentContext();
			for (auto it = Self->Modules.CreateConstIterator(); it; ++it)
			{
				const auto& name = it.Key();
				const auto& module = it.Value();

				auto FullPath = FPaths::ConvertRelativePathToFull(name);
				(void)out->Set(context_, V8_String(isolate, name), V8_String(isolate, TCHAR_TO_UTF8(*FullPath)));
			}

			info.GetReturnValue().Set(out);
		};
		(void)global->SetAccessor(ctx, V8_KeywordString(isolate(), "modules"), getter, 0, self);
	}


	void ExposeMemory2()
	{
		FIsolateHelper I(isolate());
		auto ctx = context();
		auto global = ctx->Global();

		Local<FunctionTemplate> Template = I.FunctionTemplate();

		auto add_fn = [&](const char* name, FunctionCallback fn) {
			(void)global->Set(ctx, I.Keyword(name), I.FunctionTemplate(FV8Exception::GuardLambda(fn))->GetFunction(ctx).ToLocalChecked());
		};

		add_fn("$memaccess", [](const FunctionCallbackInfo<Value>& info)
		{
			auto isolate = info.GetIsolate();

			FIsolateHelper I(isolate);
			auto context = isolate->GetCurrentContext();

			if (info.Length() == 3 && info[2]->IsFunction())
			{
				HandleScope handle_scope(isolate);

				auto Instance = FStructMemoryInstance::FromV8(context, info[0]);
				auto function = info[2].As<Function>();

				// If given value is an instance
				if (Instance)
				{
					auto Memory = Instance->GetMemory();
					auto GivenStruct = Instance->Struct;

					if (Memory && Instance->Struct->IsChildOf(FJavascriptRawAccess::StaticStruct()))
					{
						auto Source = reinterpret_cast<FJavascriptRawAccess*>(Memory);

						v8::Handle<Value> argv[1];

						auto Name = StringFromV8(isolate, info[1]);

						for (auto Index = 0; Index < Source->GetNumData(); ++Index)
						{
							if (Source->GetDataName(Index).ToString() == Name)
							{
								auto ProxyStruct = Source->GetScriptStruct(Index);
								auto Proxy = Source->GetData(Index);
								if (ProxyStruct && Proxy)
								{
									argv[0] = FJavascriptIsolate::ExportStructInstance(isolate, ProxyStruct, (uint8*)Proxy, FStructMemoryPropertyOwner(Instance));
								}

								(void)function->Call(context, info.This(), 1, argv);
								return;
							}
						}
					}
				}
			}

			if (info.Length() == 2 && info[1]->IsFunction())
			{
				HandleScope handle_scope(isolate);

				auto Instance = FStructMemoryInstance::FromV8(context, info[0]);
				auto function = info[1].As<Function>();

				// If given value is an instance
				if (Instance)
				{
					auto Memory = Instance->GetMemory();
					auto GivenStruct = Instance->Struct;
					if (Memory && Instance->Struct->IsChildOf(FJavascriptMemoryStruct::StaticStruct()))
					{
						auto Source = reinterpret_cast<FJavascriptMemoryStruct*>(Memory);

						v8::Handle<Value> argv[1];

						auto Dimension = Source->GetDimension();
						auto Indices = (int32*)FMemory_Alloca(sizeof(int32) * Dimension);
						if (Dimension == 1)
						{
#if V8_MAJOR_VERSION < 9
							auto ab = ArrayBuffer::New(info.GetIsolate(), Source->GetMemory(nullptr), Source->GetSize(0));
#else
							auto backing_store = ArrayBuffer::NewBackingStore(Source->GetMemory(nullptr), Source->GetSize(0), v8::BackingStore::EmptyDeleter, nullptr);
							auto ab = ArrayBuffer::New(info.GetIsolate(), std::move(backing_store));
#endif
							argv[0] = ab;

							(void)function->Call(context, info.This(), 1, argv);
							return;
						}
						else if (Dimension == 2)
						{
							auto Outer = Source->GetSize(0);
							auto Inner = Source->GetSize(1);
							auto out_arr = Array::New(info.GetIsolate(), Outer);
							argv[0] = out_arr;
							for (auto Index = 0; Index < Outer; ++Index)
							{
								Indices[0] = Index;
#if V8_MAJOR_VERSION < 9
								auto ab = ArrayBuffer::New(info.GetIsolate(), Source->GetMemory(Indices), Inner);
#else
								auto backing_store = ArrayBuffer::NewBackingStore(Source->GetMemory(Indices), Inner, v8::BackingStore::EmptyDeleter, nullptr);
								auto ab = ArrayBuffer::New(info.GetIsolate(), std::move(backing_store));
#endif
								(void)out_arr->Set(context, Index, ab);
							}

							(void)function->Call(context, info.This(), 1, argv);
							return;
						}
					}
					if (Memory && Instance->Struct->IsChildOf(FJavascriptRawAccess::StaticStruct()))
					{
						auto Source = reinterpret_cast<FJavascriptRawAccess*>(Memory);

						v8::Handle<Value> argv[1];

						auto ProxyStruct = Source->GetScriptStruct(0);
						auto Proxy = Source->GetData(0);
						if (ProxyStruct && Proxy)
						{
							argv[0] = FJavascriptIsolate::ExportStructInstance(isolate, ProxyStruct, (uint8*)Proxy, FStructMemoryPropertyOwner(Instance));
						}

						(void)function->Call(context, info.This(), 1, argv);
						return;
					}
				}
			}
			I.Throw(TEXT("memory.fork requires JavascriptMemoryObject"));
		});
	}

	FString GetScriptFileFullPath(const FString& Filename)
	{
		for (auto Path : Paths)
		{
			auto FullPath = Path / Filename;
			if (IFileManager::Get().FileExists(*FullPath))
			{
				return FullPath;
			}
		}
		return Filename;
	}

	FString ReadScriptFile(const FString& Filename)
	{
		auto Path = GetScriptFileFullPath(Filename);

		FString Text;

		if (!FFileHelper::LoadFileToString(Text, *Path))
		{
			UE_LOG(LogJavascript, Warning, TEXT("Failed to read script file '%s'"), *Filename);
		}

		return Text;
	}

	FString RunFile(const FString& Filename, const TArray<FString>& Args = TArray<FString>())
	{
		Isolate::Scope isolate_scope(isolate());
		HandleScope handle_scope(isolate());
		Context::Scope context_scope(context());

		auto Script = ReadScriptFile(Filename);

		auto ScriptPath = GetScriptFileFullPath(Filename);
		FString Text;
		if (Args.Num() > 0)
		{
			FString strArgs = FString::Printf(TEXT("\'%s\'"), *Args[0]);
			for (int32 i = 1; i < Args.Num(); ++i)
			{
				strArgs += TEXT(", ");
				strArgs +=  FString::Printf(TEXT("\'%s\'"), *Args[i]);
			}

			Text = FString::Printf(TEXT("(function (global,__filename,__dirname, ...args) { %s\n; }(this,'%s','%s', %s));"), *Script, *ScriptPath, *FPaths::GetPath(ScriptPath), *strArgs);
		}
		else
		{
			Text = FString::Printf(TEXT("(function (global,__filename,__dirname) { %s\n;}(this,'%s','%s'));"), *Script, *ScriptPath, *FPaths::GetPath(ScriptPath));
		}

		auto ret = RunScript(ScriptPath, Text, 0);
		return ret.IsEmpty()? TEXT("(empty)") : StringFromV8(isolate(), ret);
	}

	FString Public_RunFile(const FString& Filename, const TArray<FString>& Args)
	{
		return RunFile(Filename, Args);
	}

	FString Public_RunScript(const FString& Script, bool bOutput = true)
	{
		Isolate::Scope isolate_scope(isolate());
		HandleScope handle_scope(isolate());
		Context::Scope context_scope(context());

		auto ret = RunScript(TEXT("(inline)"), Script);
		auto str = ret.IsEmpty() ? TEXT("(empty)") : StringFromV8(isolate(), ret);

		if (bOutput && !ret.IsEmpty())
		{
			UE_LOG(LogJavascript, Log, TEXT("%s"), *str);
		}
		return str;
	}

	void RequestV8GarbageCollection()
	{
		isolate()->LowMemoryNotification();
	}

	// Should be guarded with proper handle scope
	Local<Value> RunScript(const FString& Filename, const FString& Script, int line_offset = 0)
	{
		Isolate::Scope isolate_scope(isolate());
		Context::Scope context_scope(context());

		TryCatch try_catch(isolate());
		try_catch.SetVerbose(true);

		auto Path = Filename;
#if PLATFORM_WINDOWS
		// HACK for Visual Studio Code
		if (Path.Len() && Path[1] == ':')
		{
			Path = Path.Mid(0, 1).ToLower() + Path.Mid(1);
		}
#endif
		auto source = V8_String(isolate(), Script);
		auto path = V8_String(isolate(), LocalPathToURL(Path));
		auto ctx = context();
		ScriptOrigin origin(path, -line_offset);
		auto script = Script::Compile(ctx, source, &origin);
		if (script.IsEmpty())
		{
			FJavascriptContext::FromV8(ctx)->UncaughtException(FV8Exception::Report(isolate(), try_catch));
			return Local<Value>();
		}
		else
		{
			auto result = script.ToLocalChecked()->Run(ctx);
			if (try_catch.HasCaught())
			{
				FJavascriptContext::FromV8(ctx)->UncaughtException(FV8Exception::Report(isolate(), try_catch));
				return Local<Value>();
			}
			else
			{
				return result.ToLocalChecked();
			}
		}
	}

    void FindPathFile(FString TargetRootPath, FString TargetFileName, TArray<FString>& OutFiles)
    {
        IFileManager::Get().FindFilesRecursive(OutFiles, TargetRootPath.GetCharArray().GetData(), TargetFileName.GetCharArray().GetData(), true, false);
    }

	void Expose(FString RootName, UObject* Object)
	{
		WKOs.Add(RootName, Object);
		AccessorNameGetterCallback RootGetter = [](Local<Name> property, const PropertyCallbackInfo<Value>& info) {
			auto isolate = info.GetIsolate();
			info.GetReturnValue().Set(info.Data());
		};

		Isolate::Scope isolate_scope(isolate());
		HandleScope handle_scope(isolate());
		auto ctx = context();
		Context::Scope context_scope(ctx);

		(void)ctx->Global()->SetAccessor(ctx, V8_KeywordString(isolate(), RootName), RootGetter, 0, ExportObject(Object));
	}

	Local<Value> ExportObject(UObject* Object, bool bForce = false) override
	{
		return Environment->ExportObject(Object, bForce);
	}

	bool WriteAliases(const FString& Filename)
	{
#if WITH_EDITOR
		struct TokenWriter
		{
			FString Text;

			TokenWriter& push(const char* something)
			{
				Text.Append(ANSI_TO_TCHAR(something));
				return *this;
			}

			TokenWriter& push(const FString& something)
			{
				Text.Append(something);
				return *this;
			}

			const TCHAR* operator * ()
			{
				return *Text;
			}
		};

		TokenWriter w;

		int DefaultValueId = 0;
		auto Packer = RunScript(TEXT(""),TEXT("JSON.stringify")).As<Function>();

		auto guard_pre = [&] { w.push("try { "); };
		auto guard_post = [&] { w.push(" } catch (e) {};\n"); };

		for (auto it = Environment->ClassToFunctionTemplateMap.CreateConstIterator(); it; ++it)
		{
			const UClass* ClassToExport = it.Key();

#if WITH_EDITORONLY_DATA
			// Skip a generated class
			if (ClassToExport->ClassGeneratedBy) continue;
#endif
			auto ClassName = FV8Config::Safeify(ClassToExport->GetName());

			// Function with default value
			{
				// Iterate over all functions
				for (TFieldIterator<UFunction> FuncIt(ClassToExport, EFieldIteratorFlags::ExcludeSuper); FuncIt; ++FuncIt)
				{
					auto Function = *FuncIt;

					// Parse all function parameters.
					uint8* Parms = (uint8*)FMemory_Alloca(Function->ParmsSize);
					FMemory::Memzero(Parms, Function->ParmsSize);

					bool bHasDefault = false;
					TArray<FString> Parameters, ParametersWithDefaults;

					for (TFieldIterator<FProperty> It(Function); It && (It->PropertyFlags & (CPF_Parm | CPF_ReturnParm)) == CPF_Parm; ++It)
					{
						auto Property = *It;
						const FName MetadataCppDefaultValueKey(*(FString(TEXT("CPP_Default_")) + Property->GetName()));
						const FString MetadataCppDefaultValue = Function->GetMetaData(MetadataCppDefaultValueKey);
						FString Parameter = Property->GetName();
						FString ParameterWithValue = Parameter;
						if (!MetadataCppDefaultValue.IsEmpty())
						{
							const uint32 ExportFlags = PPF_None;
							auto Buffer = It->ContainerPtrToValuePtr<uint8>(Parms);
							
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
							const TCHAR* Result = It->ImportText_Direct(*MetadataCppDefaultValue, Buffer, nullptr, ExportFlags);
#else
							const TCHAR* Result = It->ImportText(*MetadataCppDefaultValue, Buffer, ExportFlags, nullptr);
#endif

							if (Result)
							{
								bHasDefault = true;
								auto DefaultValue = Environment->ReadProperty(isolate(), Property, Parms, FNoPropertyOwner());
								{
									auto ctx = context();
									Context::Scope context_scope(ctx);

									v8::Handle<Value> args[] = { DefaultValue };
									auto maybe_ret = Packer->Call(ctx, Packer, 1, args);
									if (!maybe_ret.IsEmpty())
									{
										auto ret = maybe_ret.ToLocalChecked();
										auto Ret = StringFromV8(isolate(), ret);
										ParameterWithValue = FString::Printf(TEXT("%s = %s"), *Parameter, *Ret);
									}
								}

								It->DestroyValue_InContainer(Parms);
							}
						}
						Parameters.Add(Parameter);
						ParametersWithDefaults.Add(ParameterWithValue);
					}

					if (bHasDefault)
					{
						auto Name = FV8Config::Safeify(Function->GetName());
						auto FnId = FString::Printf(TEXT("fnprepatch_%d"), DefaultValueId++);

						guard_pre();

						w.push("let ");
						w.push(FnId);
						w.push(" = ");
						w.push(ClassName);
						w.push(".prototype.");
						w.push(Name);
						w.push(";");
						w.push(ClassName);
						w.push(".prototype.");
						w.push(Name);
						w.push(" = function (");
						w.push(FString::Join(ParametersWithDefaults, TEXT(", ")));
						w.push(") { return ");
						w.push(FnId);
						w.push(".call(this, ");
						w.push(FString::Join(Parameters, TEXT(", ")));
						w.push(") };");

						guard_post();
					}
				}
			}

			TArray<UFunction*> Functions;
			Environment->BlueprintFunctionLibraryMapping.MultiFind(ClassToExport, Functions);

			auto conditional_emit_alias = [&](UFunction* Function, bool is_thunk) {
				auto Alias = FV8Config::GetAlias(Function);
				if (FV8Config::CanExportFunction(ClassToExport, Function) && Alias.Len() > 0)
				{
					guard_pre();

					w.push(ClassName);
					w.push(".prototype.");
					w.push(Alias);
					w.push(" = ");
					w.push(ClassName);
					w.push(".prototype.");
					w.push(FV8Config::Safeify(Function->GetName()));
					w.push(";");

					guard_post();

					if (!is_thunk && Function->FunctionFlags & FUNC_Static)
					{
						guard_pre();

						w.push(ClassName);
						w.push(".");
						w.push(Alias);
						w.push(" = ");
						w.push(ClassName);
						w.push(".");
						w.push(FV8Config::Safeify(Function->GetName()));
						w.push(";");

						guard_post();
					}
				}
			};

			for (auto Function : Functions)
			{
				conditional_emit_alias(Function, true);
			}

			for (TFieldIterator<UFunction> FuncIt(ClassToExport, EFieldIteratorFlags::ExcludeSuper); FuncIt; ++FuncIt)
			{
				conditional_emit_alias(*FuncIt, false);
			}
		}

		for (auto it = Environment->ScriptStructToFunctionTemplateMap.CreateConstIterator(); it; ++it)
		{
			const UStruct* StructToExport = it.Key();

			auto ClassName = FV8Config::Safeify(StructToExport->GetName());

			TArray<UFunction*> Functions;
			Environment->BlueprintFunctionLibraryMapping.MultiFind(StructToExport, Functions);

			auto conditional_emit_alias = [&](UFunction* Function) {
				auto Alias = FV8Config::GetAlias(Function);
				if (Alias.Len() > 0)
				{
					guard_pre();

					w.push(ClassName);
					w.push(".prototype.");
					w.push(Alias);
					w.push(" = ");
					w.push(ClassName);
					w.push(".prototype.");
					w.push(FV8Config::Safeify(Function->GetName()));
					w.push(";");

					guard_post();
				}
			};

			for (auto Function : Functions)
			{
				conditional_emit_alias(Function);
			}
		}

		return FFileHelper::SaveStringToFile(*w, *Filename);
#else
		return false;
#endif
	}

	bool WriteDTS(const FString& Filename, bool bIncludingTooltip)
	{
#if WITH_EDITOR
		TypingGenerator instance(*(Environment.Get()));

		instance.no_tooltip = !bIncludingTooltip;

		instance.ExportBootstrap();

		for (auto it = Environment->ClassToFunctionTemplateMap.CreateConstIterator(); it; ++it)
		{
			instance.Export(it.Key());
		}

		for (auto pair : WKOs)
		{
			auto k = pair.Key;
			auto v = pair.Value;

			instance.ExportWKO(k, v);
		}

		instance.Finalize();

		FString Path, BaseFilename, Extension;
		FPaths::Split(Filename, Path, BaseFilename, Extension);
		auto GlobalNamePath = FPaths::Combine(*Path, TEXT("globals.js"));
		instance.SaveGlobalNames(GlobalNamePath);

		return instance.Save(Filename);
#else
		return false;
#endif
	}

	Local<Value> GetProxyFunction(Local<Context> Context, UObject* Object, const TCHAR* Name)
	{
		auto exported = ExportObject(Object);
		if (exported->IsUndefined())
		{
			return v8::Undefined(isolate());
		}

		auto maybe_obj = exported->ToObject(Context);
		if (maybe_obj.IsEmpty())
		{
			return v8::Undefined(isolate());
		}

		auto maybe_proxy = maybe_obj.ToLocalChecked()->Get(Context, V8_KeywordString(isolate(), "proxy"));
		if (maybe_proxy.IsEmpty())
		{
			return v8::Undefined(isolate());
		}

		auto proxy = maybe_proxy.ToLocalChecked();
		if (proxy.IsEmpty() || !proxy->IsObject())
		{
			return v8::Undefined(isolate());
		}

		auto maybe_proxyObj = proxy->ToObject(Context);
		if (maybe_proxyObj.IsEmpty())
		{
			return v8::Undefined(isolate());
		}
		auto maybe_func = maybe_proxyObj.ToLocalChecked()->Get(Context, V8_KeywordString(isolate(), Name));

		if (maybe_func.IsEmpty())
		{
			return v8::Undefined(isolate());
		}
		
		auto func = maybe_func.ToLocalChecked();

		if (func.IsEmpty() || !func->IsFunction())
		{
			return v8::Undefined(isolate());
		}

		return func;
	}

	Local<Value> GetProxyFunction(Local<Context> Context, UObject* Object, UFunction* Function)
	{
		return GetProxyFunction(Context, Object, *FV8Config::Safeify(Function->GetName()));
	}

	bool HasProxyFunction(UObject* Holder, UFunction* Function)
	{
		Isolate::Scope isolate_scope(isolate());
		HandleScope handle_scope(isolate());

		auto func = GetProxyFunction(context(), Holder, Function);;
		return !func.IsEmpty() && func->IsFunction();
	}

	bool CallProxyFunction(UObject* Holder, UObject* This, UFunction* FunctionToCall, void* Parms)
	{
		SCOPE_CYCLE_COUNTER(STAT_JavascriptProxy);

		Isolate::Scope isolate_scope(isolate());
		HandleScope handle_scope(isolate());

		Context::Scope context_scope(context());

		auto func = GetProxyFunction(context(), Holder, FunctionToCall);
		if (!func.IsEmpty() && func->IsFunction())
		{
			auto th = This ? ExportObject(This) : Local<Value>::Cast(context()->Global());
			if (!th->IsUndefined())
			{
				CallJavascriptFunction(context(), th, FunctionToCall, Local<Function>::Cast(func), Parms);
				return true;
			}
		}
		return false;
	}

	bool CallProxyFunction(UObject* Holder, UObject* This, const TCHAR* Name, void* Parms)
	{
		SCOPE_CYCLE_COUNTER(STAT_JavascriptProxy);

		Isolate::Scope isolate_scope(isolate());
		HandleScope handle_scope(isolate());

		Context::Scope context_scope(context());

		auto func = GetProxyFunction(context(), Holder, Name);
		if (!func.IsEmpty() && func->IsFunction())
		{
			auto th = This ? ExportObject(This) : Local<Value>::Cast(context()->Global());
			if (!th->IsUndefined())
			{
				CallJavascriptFunction(context(), th, nullptr, Local<Function>::Cast(func), Parms);
				return true;
			}
		}
		return false;
	}

	// To tell Unreal engine's GC not to destroy these objects!
	virtual void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector) override;

	virtual void UncaughtException(const FString& Exception) override
	{
		auto _isolate = isolate();
		Isolate::Scope isolate_scope(_isolate);
		HandleScope handle_scope(_isolate);
		Context::Scope context_scope(context());

		auto maybe_global = Local<Value>::Cast(context()->Global())->ToObject(context());
		if (!maybe_global.IsEmpty())
		{
			auto global = maybe_global.ToLocalChecked();
			auto func = global->Get(context(), V8_KeywordString(_isolate, "$uncaughtException")).ToLocalChecked();
			if (!func.IsEmpty() && func->IsFunction())
			{
				auto function = func.As<Function>();

				v8::Handle<Value> argv[1];

				argv[0] = V8_String(_isolate, Exception);

				(void)function->Call(context(), global, 1, argv);
			}
		}
	}

	virtual bool IsExcludeGCObjectTarget(UObject* TargetObj) override
	{
		if (TargetObj == nullptr)
		{
			return true;
		}

		if (TargetObj->GetOutermost()->HasAnyPackageFlags(PKG_PlayInEditor))
		{
			return true;
		}

		if (TargetObj->IsA(AActor::StaticClass()) || TargetObj->IsA(UWorld::StaticClass()) || TargetObj->IsA(ULevel::StaticClass()) || TargetObj->IsA(UActorComponent::StaticClass()))
		{
			return true;
		}
		else if (TargetObj->IsA(UBlueprint::StaticClass()))
		{
			UBlueprint* Blueprint = Cast<UBlueprint>(TargetObj);

			if ((Blueprint && Blueprint->BlueprintType == BPTYPE_LevelScript))
			{
				return true;
			}
		}

		return false;
	}

	virtual void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources) override
	{
		for (auto It = ObjectToObjectMap.CreateIterator(); It; ++It)
		{
			auto Object = It.Key();

			if (World == Object)
			{
				It.RemoveCurrent();
			}
		}
	}

	virtual bool IsExcludeGCStructTarget(UStruct* TargetStruct) override
	{
#if WITH_EDITORONLY_DATA
		UClass* Class = Cast<UClass>(TargetStruct);

		if (Class && Class->ClassGeneratedBy && Cast<UBlueprint>(Class->ClassGeneratedBy)->BlueprintType == EBlueprintType::BPTYPE_LevelScript)
		{
			return true;
		}
#endif
		return false;
	}
};

FJavascriptContext* FJavascriptContext::FromV8(v8::Local<v8::Context> Context)
{
	if (Context.IsEmpty()) return nullptr;

	auto Instance = reinterpret_cast<FJavascriptContextImplementation*>(Context->GetAlignedPointerFromEmbedderData(kContextEmbedderDataIndex));
	if (Instance->IsValid())
	{
		return Instance;
	}
	else
	{
		return nullptr;
	}
}

FJavascriptContext* FJavascriptContext::Create(TSharedPtr<FJavascriptIsolate> InEnvironment, TArray<FString>& InPaths)
{
	return new FJavascriptContextImplementation(InEnvironment, InPaths);
}

// To tell Unreal engine's GC not to destroy these objects!

inline void FJavascriptContextImplementation::AddReferencedObjects(UObject * InThis, FReferenceCollector & Collector)
{
	// All objects
	for (auto It = ObjectToObjectMap.CreateIterator(); It; ++It)
	{
//		UE_LOG(LogJavascript, Log, TEXT("JavascriptContext referencing %s %s"), *(It.Key()->GetClass()->GetName()), *(It.Key()->GetName()));
		auto Object = It.Key();
		if (!(::IsValid(Object)) || !Object->IsValidLowLevelFast() || Object->HasAnyFlags(RF_BeginDestroyed) || Object->HasAnyFlags(RF_FinishDestroyed))
		{
			It.RemoveCurrent();
		}
		else if (!IsExcludeGCObjectTarget(Object))
		{
			Collector.AddReferencedObject(Object, InThis);
		}
	}

	// All structs
	for (auto It = MemoryToObjectMap.CreateIterator(); It; ++It)
	{
		TSharedPtr<FStructMemoryInstance> StructScript = It.Value().Instance;
		if (!StructScript.IsValid() || !(::IsValid(StructScript->Struct)))
		{
			It.RemoveCurrent();
		}
		else if (!IsExcludeGCStructTarget(StructScript->Struct))
		{
			Collector.AddReferencedObject(StructScript->Struct, InThis);
		}
	}
}

PRAGMA_ENABLE_SHADOW_VARIABLE_WARNINGS
