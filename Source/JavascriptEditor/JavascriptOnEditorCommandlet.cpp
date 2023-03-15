#include "JavascriptOnEditorCommandlet.h"
#include "JavascriptIsolate.h"
#include "JavascriptContext.h"
#if WITH_EDITOR
// Settings
#include "JavascriptSettings.h"
#include "ISettingsModule.h"
#include "Settings/EditorLoadingSavingSettings.h"
#endif

#if !UE_SERVER
//#include "AppMediaTimeSource.h"
#include "IHeadMountedDisplayModule.h"
//#include "IMediaModule.h"
#include "HeadMountedDisplay.h"
//#include "MRMeshModule.h"
#include "Interfaces/ISlateRHIRendererModule.h"
#include "Interfaces/ISlateNullRendererModule.h"
#include "EngineFontServices.h"
#endif

DEFINE_LOG_CATEGORY(LogJavascriptOnEditor);

#define LOCTEXT_NAMESPACE "UnrealJSEditor"

UJavascriptOnEditorCommandlet::UJavascriptOnEditorCommandlet(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

UEditorEngine* UJavascriptOnEditorCommandlet::GetEngine()
{
	return Cast<UEditorEngine>(GEngine);
}

void UJavascriptOnEditorCommandlet::RegisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "Plugins", "UnrealJS",
			LOCTEXT("RuntimeSettingsName", "UnrealJS"),
			LOCTEXT("RuntimeSettingsDescription", "Configure the UnrealJS plugin"),
			GetMutableDefault<UJavascriptSettings>());
	}
}

void UJavascriptOnEditorCommandlet::UnregisterSettings()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "Plugins", "UnrealJS");
	}
}

void UJavascriptOnEditorCommandlet::OnPropertyChanged(UObject* ObjectBeingModified, FPropertyChangedEvent& PropertyChangedEvent)
{
	if (auto Settings = Cast<UJavascriptSettings>(ObjectBeingModified))
	{
		Settings->Apply();
	}
}

void UJavascriptOnEditorCommandlet::Bootstrap()
{
	FSlateApplication::Create();

	TSharedPtr<FSlateRenderer> SlateRenderer = GUsingNullRHI ?
		FModuleManager::Get().LoadModuleChecked<ISlateNullRendererModule>("SlateNullRenderer").CreateSlateNullRenderer() :
		FModuleManager::Get().GetModuleChecked<ISlateRHIRendererModule>("SlateRHIRenderer").CreateSlateRHIRenderer();
	TSharedRef<FSlateRenderer> SlateRendererSharedRef = SlateRenderer.ToSharedRef();

	{
		SCOPED_BOOT_TIMING("CurrentSlateApp.InitializeRenderer");
		// If Slate is being used, initialize the renderer after RHIInit
		FSlateApplication& CurrentSlateApp = FSlateApplication::Get();
		CurrentSlateApp.InitializeRenderer(SlateRendererSharedRef);
	}

	// Register to be notified when properties are edited
	//OnPropertyChangedDelegateHandle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddRaw(this, &UJavascriptOnEditorCommandlet::OnPropertyChanged);

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

	//GEditor->GetTimerManager()->SetTimer(TimerHandle_GarbageCollect, FTimerDelegate::CreateRaw(this, &FJavascriptEditorModule::GarbageCollect), 60.f, true);	
}

void UJavascriptOnEditorCommandlet::Terminate()
{
	JavascriptContext->RunScript(TEXT("this['$exit'] && this['$exit']()"));
}

int32 UJavascriptOnEditorCommandlet::Main(const FString& Params)
{
	bool bSuccess = false;

#if !UE_BUILD_SHIPPING
	const TCHAR* ParamStr = *Params;
	ParseCommandLine(ParamStr, CmdLineTokens, CmdLineSwitches);	

	{
		double start = FPlatformTime::Seconds();		
		Bootstrap();
		double end = FPlatformTime::Seconds();
		UE_LOG(LogJavascriptOnEditor, Warning, TEXT("JS bootstrap Elapsed: %.6f"), end - start);

		{
			FEditorScriptExecutionGuard ScriptGuard;

			if (CmdLineTokens.Num())
			{
				FString FileName = CmdLineTokens[0];
				CmdLineTokens.RemoveAt(0);				
				FString result = JavascriptContext->RunFileWithArgs(FileName, CmdLineTokens);
				if (result == TEXT("true"))
				{
					bSuccess = true;
				}
			}
		}

		Terminate();

		UnregisterSettings();

		JavascriptContext->JavascriptContext.Reset();
		JavascriptContext->RemoveFromRoot();
	}
#else	
	bSuccess = false;
#endif

	return bSuccess ? 0 : 1;
}
#undef LOCTEXT_NAMESPACE
