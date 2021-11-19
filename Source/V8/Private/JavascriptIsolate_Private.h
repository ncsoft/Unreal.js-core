#pragma once

#include "CoreMinimal.h"
#include "Translator.h"
#include "V8PCH.h"

struct FStructMemoryInstance;
struct IPropertyOwner;
class FJavascriptIsolate;

struct FPendingClassConstruction
{
	FPendingClassConstruction() {}
	FPendingClassConstruction(v8::Handle<v8::Object> InObject, UClass* InClass)
		: Object(InObject), Class(InClass)
	{}

	v8::Handle<v8::Object> Object;
	UClass* Class;
	bool bCatched{ false };

	void Finalize(FJavascriptIsolate* Isolate, UObject* Object);
};

class FJavascriptIsolate
{
public:
	/** A map from Unreal UClass to V8 Function template */
	TMap< UClass*, v8::UniquePersistent<v8::FunctionTemplate> > ClassToFunctionTemplateMap;

	/** A map from Unreal UScriptStruct to V8 Function template */
	TMap< UScriptStruct*, v8::UniquePersistent<v8::FunctionTemplate> > ScriptStructToFunctionTemplateMap;

	/** BlueprintFunctionLibrary function mapping */
	TMultiMap< const UStruct*, UFunction*> BlueprintFunctionLibraryMapping;

	TMultiMap< const UStruct*, UFunction*> BlueprintFunctionLibraryFactoryMapping;

	TArray<FPendingClassConstruction> ObjectUnderConstructionStack;

	v8::Isolate* isolate_;

	FDelegateHandle OnWorldCleanupHandle;

	static FJavascriptIsolate* Create(bool bIsEditor);
	static v8::Local<v8::Value> ReadProperty(v8::Isolate* isolate, FProperty* Property, uint8* Buffer, const IPropertyOwner& Owner, const FPropertyAccessorFlags& Flags = FPropertyAccessorFlags());
	static void WriteProperty(v8::Isolate* isolate, FProperty* Property, uint8* Buffer, v8::Handle<v8::Value> Value, const IPropertyOwner& Owner, const FPropertyAccessorFlags& Flags = FPropertyAccessorFlags());
	static v8::Local<v8::Value> ExportStructInstance(v8::Isolate* isolate, UScriptStruct* Struct, uint8* Buffer, const IPropertyOwner& Owner);

	virtual v8::Local<v8::Value> ExportObject(UObject* Object, bool bForce = false) = 0;
	virtual v8::Local<v8::FunctionTemplate> ExportStruct(UScriptStruct* ScriptStruct) = 0;
	virtual v8::Local<v8::FunctionTemplate> ExportUClass(UClass* Class, bool bAutoRegister = true) = 0;
	virtual void RegisterUClass(UClass* Class, v8::Local<v8::FunctionTemplate> Template) = 0;
	virtual v8::Local<v8::ObjectTemplate> GetGlobalTemplate() = 0;
	virtual void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector) = 0;
	virtual v8::Local<v8::Value> ExportStructInstance(UScriptStruct* Struct, uint8* Buffer, const IPropertyOwner& Owner) = 0;
	virtual void PublicExportUClass(UClass* ClassToExport) = 0;
	virtual void PublicExportStruct(UScriptStruct* StructToExport) = 0;
	virtual int IsExcludeGCUClassTarget(UClass* TargetUClass) { return INDEX_NONE; }
	virtual ~FJavascriptIsolate() {}
	virtual void ResetUnrealConsoleDelegate() {}
};

struct FJavascriptIsolateConstant
{
	static const FName MD_BitmaskEnum;
};
