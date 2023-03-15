#include "V8PCH.h"

PRAGMA_DISABLE_SHADOW_VARIABLE_WARNINGS

#include <libplatform/libplatform.h>
#include "JavascriptContext.h"
#include "JavascriptStats.h"
#include "JavascriptSettings.h"
#include "Containers/Ticker.h"
#include "Containers/Queue.h"
#include "Misc/Paths.h"
#include "UObject/UObjectIterator.h"
#include "UObject/UObjectGlobals.h"

DEFINE_STAT(STAT_V8IdleTask);
DEFINE_STAT(STAT_JavascriptDelegate);
DEFINE_STAT(STAT_JavascriptProxy);
DEFINE_STAT(STAT_Scavenge);
DEFINE_STAT(STAT_MarkSweepCompact);
DEFINE_STAT(STAT_IncrementalMarking);
DEFINE_STAT(STAT_ProcessWeakCallbacks);

DEFINE_STAT(STAT_JavascriptPropertyGet);
DEFINE_STAT(STAT_JavascriptPropertySet);
DEFINE_STAT(STAT_JavascriptFunctionCallToEngine);
DEFINE_STAT(STAT_JavascriptFunctionCallToJavascript);
DEFINE_STAT(STAT_JavascriptReadOffStruct);

DEFINE_STAT(STAT_NewSpace);
DEFINE_STAT(STAT_OldSpace);
DEFINE_STAT(STAT_CodeSpace);
DEFINE_STAT(STAT_MapSpace);
DEFINE_STAT(STAT_LoSpace);

DEFINE_LOG_CATEGORY(LogJavascript);

using namespace v8;

static float GV8IdleTaskBudget = 1 / 60.0f;

static FAutoConsoleCommand ToggleEnableV8Hotreload(
	TEXT("v8.ToggleHotReload"),
	TEXT("Toggle HotReload Flags when changed watching files."),
	FConsoleCommandDelegate::CreateLambda(
		[]()
		{
			bool bFlag = !IV8::Get().IsEnableHotReload();
			IV8::Get().SetEnableHotReload(bFlag);
			FName NAME_Javascript("LogJavascript");
			GLog->Log(NAME_Javascript, ELogVerbosity::Log, *FString::Printf(TEXT("v8 hotreload %s"), bFlag ? TEXT("on") : TEXT("off")));
		}
	)
);

UJavascriptSettings::UJavascriptSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if PLATFORM_IOS
	V8Flags = TEXT("--harmony --harmony-shipping --expose-gc --jitless");
#else
	V8Flags = TEXT("--harmony --harmony-shipping --expose-gc");
#endif
	bEnableHotReload = true;
}

void UJavascriptSettings::Apply() const
{
	IV8::Get().SetFlagsFromString(V8Flags);
}

class FUnrealJSPlatform : public v8::Platform
{
private:
	std::unique_ptr<v8::Platform> platform_;
#if V8_MAJOR_VERSION < 9
	TQueue<v8::IdleTask*> IdleTasks;
	FTickerDelegate TickDelegate;
	FTSTicker::FDelegateHandle TickHandle;
#endif
	bool bActive{ true };

public:
	v8::Platform* platform() const
	{
		return platform_.get();
	}
	FUnrealJSPlatform() 
		: platform_(platform::NewDefaultPlatform(0, platform::IdleTaskSupport::kEnabled))
	{
#if V8_MAJOR_VERSION < 9
		TickDelegate = FTickerDelegate::CreateRaw(this, &FUnrealJSPlatform::HandleTicker);
		TickHandle = FTSTicker::GetCoreTicker().AddTicker(TickDelegate);
#endif
	}

	~FUnrealJSPlatform()
	{
#if V8_MAJOR_VERSION < 9
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
#endif
		platform_.release();
	}

	void Shutdown()
	{
		bActive = false;
#if V8_MAJOR_VERSION < 9
		RunIdleTasks(FLT_MAX);
#endif
	}
	virtual int NumberOfWorkerThreads() { return platform_->NumberOfWorkerThreads(); }

	virtual std::shared_ptr<v8::TaskRunner> GetForegroundTaskRunner(Isolate* isolate)
	{
		return platform_->GetForegroundTaskRunner(isolate);
	}

#if V8_MAJOR_VERSION < 9
	virtual void CallOnForegroundThread(Isolate* isolate, Task* task)
	{
		std::shared_ptr<v8::TaskRunner> taskrunner =
			platform_->GetForegroundTaskRunner(isolate);
		taskrunner->PostTask(std::make_unique<Task>(task));
	}

	virtual void CallDelayedOnForegroundThread(Isolate* isolate, Task* task,
		double delay_in_seconds)
	{
		std::shared_ptr<v8::TaskRunner> taskrunner =
			platform_->GetForegroundTaskRunner(isolate);
		taskrunner->PostDelayedTask(std::make_unique<Task>(task), delay_in_seconds);
	}

	virtual void CallIdleOnForegroundThread(Isolate* isolate, IdleTask* task)
	{
		IdleTasks.Enqueue(task);
	}
#else
	std::unique_ptr<JobHandle> PostJob(
		TaskPriority priority, std::unique_ptr<JobTask> job_task) override
	{
		return platform_->PostJob(priority, std::move(job_task));
	}

	virtual ZoneBackingAllocator* GetZoneBackingAllocator() override
	{
		return platform_->GetZoneBackingAllocator();
	}
#endif

	virtual void CallOnWorkerThread(std::unique_ptr<Task> task)
	{
		platform_->CallOnWorkerThread(std::move(task));
	}

	virtual void CallDelayedOnWorkerThread(std::unique_ptr<Task> task,
		double delay_in_seconds)
	{
		platform_->CallDelayedOnWorkerThread(std::move(task), delay_in_seconds);
	}

	virtual bool IdleTasksEnabled(Isolate* isolate) 
	{
		return bActive;
	}

	virtual double MonotonicallyIncreasingTime()
	{
		return platform_->MonotonicallyIncreasingTime();
	}

#if V8_MAJOR_VERSION > 5 && V8_MINOR_VERSION > 3
	virtual double CurrentClockTimeMillis()
	{
		return platform_->CurrentClockTimeMillis();
	}
#endif

#if V8_MAJOR_VERSION > 5
	v8::TracingController* GetTracingController() override
	{
		return platform_->GetTracingController();
	}
#endif

#if V8_MAJOR_VERSION < 9
	void RunIdleTasks(float Budget)
	{
		float Start = FPlatformTime::Seconds();
		while (!IdleTasks.IsEmpty() && Budget > 0)
		{
			v8::IdleTask* Task = nullptr;
			IdleTasks.Dequeue(Task);

			{
				SCOPE_CYCLE_COUNTER(STAT_V8IdleTask);

				Task->Run(MonotonicallyIncreasingTime() + Budget);
			}
			
			delete Task;
			
			float Now = FPlatformTime::Seconds();
			float Elapsed = Now - Start;
			Start = Now;
			Budget -= Elapsed;
		}
	}

	bool HandleTicker(float DeltaTime)
	{	
		RunIdleTasks(FMath::Max<float>(0, GV8IdleTaskBudget - DeltaTime));
		return true;
	}
#endif
};

class FV8Module : public IV8
{
public:
	TArray<FString> Paths;
	FUnrealJSPlatform platform_;

	/** IModuleInterface implementation */
	virtual void StartupModule() override
	{
		Paths.Add(GetGameScriptsDirectory());
		//@HACK : Dirty hacks
		Paths.Add(GetPluginScriptsDirectory());
		Paths.Add(GetPluginScriptsDirectory2());
		Paths.Add(GetPluginScriptsDirectory3());
		Paths.Add(GetPluginScriptsDirectory4());
		Paths.Add(GetPakPluginScriptsDirectory());

		const UJavascriptSettings& Settings = *GetDefault<UJavascriptSettings>();
		Settings.Apply();

		FCoreUObjectDelegates::GetPreGarbageCollectDelegate().AddRaw(this, &FV8Module::OnPreGarbageCollection);

		V8::InitializeICUDefaultLocation(nullptr);
		V8::InitializePlatform(&platform_);
		V8::Initialize();

		FName NAME_Javascript("LogJavascript");
		GLog->Log(NAME_Javascript, ELogVerbosity::Log, *FString::Printf(TEXT("Unreal.js started. V8 %d.%d.%d"), V8_MAJOR_VERSION, V8_MINOR_VERSION, V8_BUILD_NUMBER));
	}

	virtual void ShutdownModule() override
	{
		FCoreUObjectDelegates::GetPreGarbageCollectDelegate().RemoveAll(this);

		platform_.Shutdown();

		V8::Dispose();
		V8::ShutdownPlatform();
	}

	//@HACK
	static FString GetPluginScriptsDirectory()
	{
		return FPaths::EnginePluginsDir() / "Backend/UnrealJS/Content/Scripts/";
	}

	static FString GetPluginScriptsDirectory2()
	{
		return FPaths::EnginePluginsDir() / "UnrealJS/Content/Scripts/";
	}

	static FString GetPluginScriptsDirectory3()
	{
		return FPaths::EnginePluginsDir() / "Marketplace/UnrealJS/Content/Scripts/";
	}

	static FString GetPluginScriptsDirectory4()
	{
		return FPaths::ProjectPluginsDir() / "UnrealJS/Content/Scripts/";
	}

	static FString GetPakPluginScriptsDirectory()
	{
		return FPaths::EngineDir() / "Contents/Scripts/";
	}

	static FString GetGameScriptsDirectory()
	{
		return FPaths::ProjectContentDir() / "Scripts/";
	}

	virtual void AddGlobalScriptSearchPath(const FString& Path) override
	{
		Paths.Add(Path);
	}

	virtual void RemoveGlobalScriptSearchPath(const FString& Path) override
	{
		Paths.Remove(Path);
	}

	virtual TArray<FString> GetGlobalScriptSearchPaths() override
	{
		return Paths;
	}

	virtual void FillAutoCompletion(TSharedPtr<FString> TargetContext, TArray<FString>& OutArray, const TCHAR* Input) override
	{
		static const TCHAR* SourceCode = (const TCHAR *)LR"doc(
(function () {
    var pattern = '{0}'; var head = '';
    pattern.replace(/\\W*([\\w\\.]+)$/, function (a, b, c) { head = pattern.substr(0, c + a.length - b.length); pattern = b });
    var index = pattern.lastIndexOf('.');
    var scope = this;
    var left = '';
    if (index >= 0) {
        left = pattern.substr(0, index + 1);
        try { scope = eval(pattern.substr(0, index)); }
        catch (e) { scope = null; }
        pattern = pattern.substr(index + 1);
    }
    result = [];
    for (var k in scope) {
        if (k.indexOf(pattern) == 0) {
            result.push(head + left + k);
        }
    }
    return result.join(',');
})()
)doc";

		for (TObjectIterator<UJavascriptContext> It; It; ++It)
		{
			UJavascriptContext* Context = *It;

			if (Context->ContextId == TargetContext || (!TargetContext.IsValid() && Context->IsDebugContext()))
			{
				FString Result = Context->RunScript(FString::Format(SourceCode, { FString(Input).ReplaceCharWithEscapedChar() }), false);
				Result.ParseIntoArray(OutArray, TEXT(","));
			}
		}
	}

	virtual void Exec(TSharedPtr<FString> TargetContext, const TCHAR* Command) override
	{
		for (TObjectIterator<UJavascriptContext> It; It; ++It)
		{
			UJavascriptContext* Context = *It;

			if (Context->ContextId == TargetContext || (!TargetContext.IsValid() && Context->IsDebugContext()))
			{
				static FName NAME_Javascript("LogJavascript");
				GLog->Log(NAME_Javascript, ELogVerbosity::Log, Command);
				Context->RunScript(Command);
			}
		}
	}

	virtual bool HasDebugContext() const
	{
		for (TObjectIterator<UJavascriptContext> It; It; ++It)
		{
			UJavascriptContext* Context = *It;

			if (Context->IsDebugContext())
			{
				return true;
			}
		}

		return false;
	}

	virtual void GetContextIds(TArray<TSharedPtr<FString>>& OutContexts) override
	{
		for (TObjectIterator<UJavascriptContext> It; It; ++It)
		{
			UJavascriptContext* Context = *It;

			if (!Context->IsTemplate(RF_ClassDefaultObject))
			{
				OutContexts.Add(Context->ContextId);
			}
		}
	}

	virtual void SetFlagsFromString(const FString& V8Flags) override
	{
		V8::SetFlagsFromString(TCHAR_TO_ANSI(*V8Flags));
	}

	virtual bool IsEnableHotReload() const override
	{
		const UJavascriptSettings& Settings = *GetDefault<UJavascriptSettings>();
		return Settings.bEnableHotReload;
	}

	virtual void SetEnableHotReload(bool bEnable) override
	{		
		UJavascriptSettings& Settings = *GetMutableDefault<UJavascriptSettings>();
		Settings.bEnableHotReload = bEnable;
	}

	virtual void SetIdleTaskBudget(float BudgetInSeconds) override
	{
		GV8IdleTaskBudget = BudgetInSeconds;
	}

	virtual void* GetV8Platform() override
	{
		return platform_.platform();
	}

	void OnPreGarbageCollection()
	{
		for (TObjectIterator<UJavascriptContext> It; It; ++It)
		{
			UJavascriptContext* Context = *It;
			Context->RequestV8GarbageCollection();
		}
	}
};

IMPLEMENT_MODULE(FV8Module, V8)

PRAGMA_ENABLE_SHADOW_VARIABLE_WARNINGS
