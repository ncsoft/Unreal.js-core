#pragma once

#include "V8PCH.h"

struct FStructMemoryInstance;
class FJavascriptIsolate;

struct FJavascriptContext : TSharedFromThis<FJavascriptContext>
{
	FJavascriptContext(TSharedPtr<FJavascriptIsolate> InEnvironment) : Environment(InEnvironment) {}

	/** Isolate **/
	TSharedPtr<FJavascriptIsolate> Environment;

	/** A map from Unreal UObject to V8 Object */
	TMap< UObject*, v8::UniquePersistent<v8::Value> > ObjectToObjectMap;


	struct FExportedStructMemoryInfo
	{
		FExportedStructMemoryInfo() = default;
		FExportedStructMemoryInfo(TSharedPtr<FStructMemoryInstance> InInstance, v8::UniquePersistent<v8::Value>&& InValue)
			: Instance(InInstance)
			, Value(MoveTemp(InValue))
		{}
		FExportedStructMemoryInfo(FExportedStructMemoryInfo&& TempOther) = default;

		~FExportedStructMemoryInfo()
		{
			Instance.Reset();
			Value.Reset();
		}

		// Keeps one reference count for FStructMemoryInstance
		TSharedPtr<FStructMemoryInstance> Instance;
		// Keeps one reference count for V8 Object
		v8::UniquePersistent<v8::Value> Value;
	};

	/** A map from Struct buffer to V8 Object */
	TMap<FStructMemoryInstance*, FExportedStructMemoryInfo> MemoryToObjectMap;

	FDelegateHandle PostWorldCleanupHandle;

	virtual ~FJavascriptContext() {}
	virtual void Expose(FString RootName, UObject* Object) = 0;
	virtual FString GetScriptFileFullPath(const FString& Filename) = 0;
	virtual FString ReadScriptFile(const FString& Filename) = 0;
	virtual FString Public_RunScript(const FString& Script, bool bOutput = true) = 0;
	virtual void RequestV8GarbageCollection() = 0;
	virtual FString Public_RunFile(const FString& Filename, const TArray<FString>& Args) = 0;
    virtual void FindPathFile(const FString TargetRootPath, const FString TargetFileName, TArray<FString>& OutFiles) = 0;
	virtual bool IsDebugContext() const = 0;
	virtual void CreateInspector(int32 Port) = 0;
	virtual void DestroyInspector() = 0;
	virtual bool WriteAliases(const FString& Filename) = 0;
	virtual bool WriteDTS(const FString& Filename, bool bIncludingTooltip) = 0;
	virtual bool HasProxyFunction(UObject* Holder, UFunction* Function) = 0;
	virtual bool CallProxyFunction(UObject* Holder, UObject* This, UFunction* FunctionToCall, void* Parms) = 0;	
	virtual bool CallProxyFunction(UObject* Holder, UObject* This, const TCHAR* Name, void* Parms) = 0;

	virtual void UncaughtException(const FString& Exception) = 0;

	virtual v8::Isolate* isolate() = 0;
	virtual v8::Local<v8::Context> context() = 0;
	virtual v8::Local<v8::Value> ExportObject(UObject* Object, bool bForce = false) = 0;
	virtual v8::Local<v8::Value> GetProxyFunction(v8::Local<v8::Context> Context, UObject* Object, const TCHAR* Name) = 0;

	static FJavascriptContext* FromV8(v8::Local<v8::Context> Context);

	static FJavascriptContext* Create(TSharedPtr<FJavascriptIsolate> InEnvironment, TArray<FString>& InPaths);

	virtual void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector) = 0;

	virtual const FObjectInitializer* GetObjectInitializer() = 0;

	virtual bool IsExcludeGCObjectTarget(UObject* TargetObj) { return false; }
	virtual bool IsExcludeGCStructTarget(UStruct* TargetStruct) { return false; }

	virtual void OnWorldCleanup(UWorld* World, bool bSessionEnded, bool bCleanupResources) = 0;
};
