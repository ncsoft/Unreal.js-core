#include "JavascriptProcess.h"

#if PLATFORM_WINDOWS
#include "Windows/WindowsHWrapper.h"
#endif
//PRAGMA_DISABLE_OPTIMIZATION

UJavascriptProcess::UJavascriptProcess(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{	
}

UJavascriptProcess* UJavascriptProcess::Create(const FString& URL, const FString& Parms, bool bLaunchDetached, bool bLaunchHidden, bool bLaunchReallyHidden, int32 PriorityModifier, const FString& OptionalWorkingDirectory, bool bUsePipe)
{
	uint32 ProcessID;
	void* ReadPipe{ nullptr };
	void* WritePipe{ nullptr };
	if (bUsePipe)
	{
		FPlatformProcess::CreatePipe(ReadPipe, WritePipe);
	}
	auto Handle = FPlatformProcess::CreateProc(*URL, *Parms, bLaunchDetached, bLaunchHidden, bLaunchReallyHidden, &ProcessID, PriorityModifier, OptionalWorkingDirectory.Len() ? *OptionalWorkingDirectory : nullptr, WritePipe);
	if (Handle.IsValid())
	{
		auto Instance = NewObject<UJavascriptProcess>();
		Instance->ProcessHandle = Handle;
		Instance->ProcessID = ProcessID;
		Instance->ReadPipe = ReadPipe;
		Instance->WritePipe = WritePipe;
		return Instance;
	}
	else
	{
		return nullptr;
	}
}

UJavascriptProcess* UJavascriptProcess::Open(const FString& ProcName)
{
#if PLATFORM_WINDOWS
	FPlatformProcess::FProcEnumerator ProcEnumerator;

	while (ProcEnumerator.MoveNext())
	{
		auto Current = ProcEnumerator.GetCurrent();
		if (Current.GetName() == ProcName)
		{
			return Open_PID(Current.GetPID());
		}
	}
#endif
	return nullptr;
}

UJavascriptProcess* UJavascriptProcess::Open_PID(int32 ProcessId)
{
#if PLATFORM_WINDOWS
	auto Handle = FPlatformProcess::OpenProcess(ProcessId);
	if (Handle.IsValid())
	{
		auto Instance = NewObject<UJavascriptProcess>();
		Instance->ProcessHandle = Handle;
		Instance->ProcessID = ProcessId;
		/*Instance->ReadPipe = ReadPipe;
		Instance->WritePipe = WritePipe;*/
		return Instance;
	}
#endif
	return nullptr;
}

FString UJavascriptProcess::GetApplicationName(int32 ProcessId)
{
	return FPlatformProcess::GetApplicationName(ProcessId);
}

int32 UJavascriptProcess::GetCurrentProcessId()
{
	return (int32)FPlatformProcess::GetCurrentProcessId();
}

bool UJavascriptProcess::IsApplicationRunning_PID(int32 ProcessId)
{
	return FPlatformProcess::IsApplicationRunning(ProcessId);
}

bool UJavascriptProcess::IsApplicationRunning(const FString& ProcName)
{
	return FPlatformProcess::IsApplicationRunning(*ProcName);
}

bool UJavascriptProcess::IsRunning()
{
	return FPlatformProcess::IsProcRunning(ProcessHandle);
}

void UJavascriptProcess::Close()
{
	FPlatformProcess::CloseProc(ProcessHandle);

	if (ReadPipe || WritePipe)
	{
		FPlatformProcess::ClosePipe(ReadPipe, WritePipe);
		ReadPipe = WritePipe = nullptr;
	}
}
void UJavascriptProcess::Sleep(float Seconds)
{
	FPlatformProcess::Sleep(Seconds);
}

FString UJavascriptProcess::ReadFromPipe()
{
	return FPlatformProcess::ReadPipe(ReadPipe);
}

bool UJavascriptProcess::ReadArrayFromPipe(TArray<uint8>& Array)
{
	return FPlatformProcess::ReadPipeToArray(ReadPipe, Array);
}

bool UJavascriptProcess::WriteToPipe(const FString& Message, FString& OutWritten)
{
	return FPlatformProcess::WritePipe(WritePipe, Message, &OutWritten);
}

void UJavascriptProcess::SimulateKeypress(int32 KeyEvent)
{
#if PLATFORM_WINDOWS
	INPUT input;
	WORD vkey = KeyEvent;
	input.type = INPUT_KEYBOARD;
	input.ki.time = 0;
	input.ki.dwExtraInfo = 0;
	input.ki.wVk = vkey;
	input.ki.dwFlags = KEYEVENTF_UNICODE;
	SendInput(1, &input, sizeof(INPUT));

	input.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
	SendInput(1, &input, sizeof(INPUT));
#endif
}

void UJavascriptProcess::Wait()
{
	return FPlatformProcess::WaitForProc(ProcessHandle);
}

void UJavascriptProcess::Terminate(bool KillTree)
{
	return FPlatformProcess::TerminateProc(ProcessHandle, KillTree);
}

bool UJavascriptProcess::GetReturnCode(int32& ReturnCode)
{
	return FPlatformProcess::GetProcReturnCode(ProcessHandle,&ReturnCode);
}

void UJavascriptProcess::SetEnvironmentVar(const FString& VarName, const FString& VarValue)
{
	FPlatformMisc::SetEnvironmentVar(*VarName, *VarValue);
}

FString UJavascriptProcess::GetEnvironmentVar(const FString& VarName)
{
	return FPlatformMisc::GetEnvironmentVariable(*VarName);
}

void UJavascriptProcess::LaunchURL(const FString& URL, const FString& Parms, FString& Error)
{
	FPlatformProcess::LaunchURL(*URL, *Parms, &Error);
}

bool UJavascriptProcess::CanLaunchURL(const FString& URL)
{
	return FPlatformProcess::CanLaunchURL(*URL);
}

FString UJavascriptProcess::GetString(const FString& Key, bool bFlag)
{
	if (Key == TEXT("BaseDir")) return FPlatformProcess::BaseDir();
	if (Key == TEXT("UserDir")) return FPlatformProcess::UserDir();
	if (Key == TEXT("UserSettingsDir")) return FPlatformProcess::UserSettingsDir();
	if (Key == TEXT("UserTempDir")) return FPlatformProcess::UserTempDir();
	if (Key == TEXT("ApplicationSettingsDir")) return FPlatformProcess::ApplicationSettingsDir();
	if (Key == TEXT("ComputerName")) return FPlatformProcess::ComputerName();
	if (Key == TEXT("UserName")) return FPlatformProcess::UserName(bFlag);
	if (Key == TEXT("ShaderDir")) return FPlatformProcess::ShaderDir();
	if (Key == TEXT("CurrentWorkingDirectory")) return FPlatformProcess::GetCurrentWorkingDirectory();
	if (Key == TEXT("ShaderWorkingDir")) return FPlatformProcess::ShaderWorkingDir();
	if (Key == TEXT("ExecutableName")) return FPlatformProcess::ExecutableName(bFlag);
	if (Key == TEXT("ModulePrefix")) return FPlatformProcess::GetModulePrefix();
	if (Key == TEXT("ModuleExtension")) return FPlatformProcess::GetModuleExtension();
	if (Key == TEXT("BinariesSubdirectory")) return FPlatformProcess::GetBinariesSubdirectory();
	if (Key == TEXT("ModulesDirectory")) return FPlatformProcess::GetModulesDirectory();
	return FString();
}

//PRAGMA_ENABLE_OPTIMIZATION
