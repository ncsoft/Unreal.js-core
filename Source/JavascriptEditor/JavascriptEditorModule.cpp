#include "JavascriptEditorModule.h"
#include "JavascriptSettings.h"
#include "JavascriptIsolate.h"
#include "JavascriptContext.h"
#include "JavascriptEditorTick.h"
#include "ScopedTransaction.h"
#include "Misc/CoreDelegates.h"
#if WITH_EDITOR
// Settings
#include "ISettingsModule.h"
#include "Settings/EditorLoadingSavingSettings.h"
#endif
#include "JavascriptEditorObjectManager.h"

#define LOCTEXT_NAMESPACE "UnrealJSEditor"

class FJavascriptEditorModule : public IJavascriptEditorModule
{
	// Begin IModuleInterface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	// End IModuleInterface

	virtual void AddExtension(IEditorExtension* Extension) override;
	virtual void RemoveExtension(IEditorExtension* Extension) override;

	virtual UJavascriptEditorObjectManager* GetEditorObjectManager() override;

	void Bootstrap();
	void GarbageCollect();

#if WITH_EDITOR
	virtual UJavascriptContext* GetJavascriptContext() override { return JavascriptContext; }
#endif

private:
	void InitializeEditorObjectManager();
	void DestroyEditorObjectManager();

private:
#if WITH_EDITOR
	TArray<IEditorExtension*> Extensions;

	UJavascriptEditorTick* Tick{ nullptr };
	UJavascriptContext* JavascriptContext{ nullptr };

	bool bRegistered{ false };
	FDelegateHandle OnPropertyChangedDelegateHandle;

	FTimerHandle TimerHandle_GarbageCollect;
	UJavascriptEditorObjectManager* EditorObjectManager;

	void Unregister();

	void RegisterSettings()
	{
		if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		{
			SettingsModule->RegisterSettings("Project", "Plugins", "UnrealJS",
				LOCTEXT("RuntimeSettingsName", "UnrealJS"),
				LOCTEXT("RuntimeSettingsDescription", "Configure the UnrealJS plugin"),
				GetMutableDefault<UJavascriptSettings>());
		}
	}

	void UnregisterSettings()
	{
		if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		{
			SettingsModule->UnregisterSettings("Project", "Plugins", "UnrealJS");
		}
	}

	// Called when a property on the specified object is modified
	void OnPropertyChanged(UObject* ObjectBeingModified, FPropertyChangedEvent& PropertyChangedEvent)
	{
		if (auto Settings = Cast<UJavascriptSettings>(ObjectBeingModified))
		{
			Settings->Apply();
		}
	}
#endif
};

IMPLEMENT_MODULE(FJavascriptEditorModule, JavascriptEditor)

void FJavascriptEditorModule::AddExtension(IEditorExtension* Extension)
{
#if WITH_EDITOR
	Extensions.Add(Extension);
	Extension->Register();
#endif
}

void FJavascriptEditorModule::RemoveExtension(IEditorExtension* Extension)
{
#if WITH_EDITOR
	Extensions.RemoveSingle(Extension);
	Extension->Unregister();
#endif
}

#if WITH_EDITOR
static void PatchReimportRule()
{
	FAutoReimportWildcard WildcardToInject;
	WildcardToInject.Wildcard = TEXT("Scripts/**.json");
	WildcardToInject.bInclude = false;

	auto Default = GetMutableDefault<UEditorLoadingSavingSettings>();
	bool bHasChanged = false;
	for (auto& Setting : Default->AutoReimportDirectorySettings)
	{
		bool bFound = false;
		for (const auto& Wildcard : Setting.Wildcards)
		{
			if (Wildcard.Wildcard == WildcardToInject.Wildcard)
			{
				bFound = true;
				break;
			}
		}
		if (!bFound)
		{
			Setting.Wildcards.Add(WildcardToInject);
			bHasChanged = true;
		}
	}
	if (bHasChanged)
	{
		Default->PostEditChange();
	}
}
#endif

void FJavascriptEditorModule::GarbageCollect()
{
#if WITH_EDITOR
	if (GEditor && GEditor->IsPlayingSessionInEditor())
	{
		// #20877 - skip GC on PIE or SIE.
		return;
	}
	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
#endif
}

void FJavascriptEditorModule::Bootstrap()
{
#if WITH_EDITOR
	if (!IsRunningCommandlet())
	{
		// Register to be notified when properties are edited
		OnPropertyChangedDelegateHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddRaw(this, &FJavascriptEditorModule::OnPropertyChanged);

		RegisterSettings();

		PatchReimportRule();

		auto Isolate = NewObject<UJavascriptIsolate>();
		Isolate->Init(true);
		auto Context = Isolate->CreateContext();

		JavascriptContext = Context;
		JavascriptContext->AddToRoot();

		JavascriptContext->SetContextId(TEXT("Editor"));

		Tick = NewObject<UJavascriptEditorTick>(JavascriptContext);
		JavascriptContext->Expose(TEXT("Root"), Tick);
		Tick->AddToRoot();

		FEditorScriptExecutionGuard ScriptGuard;

		Context->RunFile("editor.js");

		bRegistered = true;

		FCoreDelegates::OnPreExit.AddRaw(this, &FJavascriptEditorModule::Unregister);

		GEditor->GetTimerManager()->SetTimer(TimerHandle_GarbageCollect, FTimerDelegate::CreateRaw(this, &FJavascriptEditorModule::GarbageCollect), 60.f, true);
	}
#endif
}

void FJavascriptEditorModule::StartupModule()
{
	FCoreDelegates::OnPostEngineInit.AddRaw(this, &FJavascriptEditorModule::Bootstrap);
	InitializeEditorObjectManager();
}

void FJavascriptEditorModule::ShutdownModule()
{
#if WITH_EDITOR
	if (!IsRunningCommandlet())
	{
		Unregister();

		if (UObjectInitialized())
		{
			UnregisterSettings();
		}

		// Unregister the property modification handler
		FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(OnPropertyChangedDelegateHandle);
	}
#endif
}

#if WITH_EDITOR
void FJavascriptEditorModule::Unregister()
{
	if (!bRegistered) return;
	bRegistered = false;

	DestroyEditorObjectManager();

	for (auto e : Extensions) { e->Unregister(); }
	Extensions.Empty();

	JavascriptContext->RunScript(TEXT("this['$exit'] && this['$exit']()"));
	JavascriptContext->RequestV8GarbageCollection();

	JavascriptContext->JavascriptContext.Reset();

	JavascriptContext->RemoveFromRoot();
	Tick->RemoveFromRoot();

	if (TimerHandle_GarbageCollect.IsValid())
	{
		GEditor->GetTimerManager()->ClearTimer(TimerHandle_GarbageCollect);
	}
}
#endif

void FJavascriptEditorModule::InitializeEditorObjectManager()
{
	EditorObjectManager = NewObject<UJavascriptEditorObjectManager>();
	EditorObjectManager->AddToRoot();
}

void FJavascriptEditorModule::DestroyEditorObjectManager()
{
	if (EditorObjectManager->IsValidLowLevel())
	{
		EditorObjectManager->RemoveFromRoot();
	}
	EditorObjectManager = nullptr;
}

UJavascriptEditorObjectManager* FJavascriptEditorModule::GetEditorObjectManager()
{
	return EditorObjectManager;
}

#undef LOCTEXT_NAMESPACE
