#include "JavascriptProfile.h"
#include "Config.h"
#include "Translator.h"
#include "Helpers.h"
#include "JavascriptLibrary.h"
#include "V8PCH.h"

using namespace v8;

void UJavascriptProfile::BeginDestroy()
{
	Super::BeginDestroy();

	if (Profile)
	{
		reinterpret_cast<CpuProfile*>(Profile)->Delete();
		Profile = nullptr;
	}
}

FJavascriptProfileNode UJavascriptProfile::GetTopDownRoot()
{
	FJavascriptProfileNode out{ nullptr };
	
	if (Profile)
	{
		out.Node = reinterpret_cast<CpuProfile*>(Profile)->GetTopDownRoot();
	}

	return out;
}

int32 UJavascriptProfile::GetSamplesCount()
{
	if (Profile)
	{
		return reinterpret_cast<CpuProfile*>(Profile)->GetSamplesCount();
	}

	return 0;
}

FJavascriptProfileNode UJavascriptProfile::GetSample(int32 index)
{
	FJavascriptProfileNode out{ nullptr };

	if (Profile)
	{
		out.Node = reinterpret_cast<CpuProfile*>(Profile)->GetSample(index);
	}

	return out;
}

float UJavascriptProfile::GetSampleTimestamp(int32 index)
{
	if (Profile)
	{
		return (float)reinterpret_cast<CpuProfile*>(Profile)->GetSampleTimestamp(index);
	}

	return -1;
}

FJavascriptCpuProfiler UJavascriptProfile::Start(const FString& Title, bool bRecordSamples)
{
	auto isolate = Isolate::GetCurrent();

	FIsolateHelper I(isolate);
	auto profiler = v8::CpuProfiler::New(isolate);
	profiler->StartProfiling(I.String(Title), bRecordSamples);

	FJavascriptCpuProfiler out { nullptr };
	out.Profiler = profiler;
	return out;
}

UJavascriptProfile* UJavascriptProfile::Stop(const FJavascriptCpuProfiler& Profiler, const FString& Title)
{
	auto isolate = Isolate::GetCurrent();

	FIsolateHelper I(isolate);

	auto Profile = reinterpret_cast<CpuProfiler*>(Profiler.Profiler)->StopProfiling(I.String(Title));

	auto instance = NewObject<UJavascriptProfile>();
	instance->Profile = Profile;
	return instance;
}

void UJavascriptProfile::SetSamplingInterval(const FJavascriptCpuProfiler& Profiler, int32 us)
{
	reinterpret_cast<CpuProfiler*>(Profiler.Profiler)->SetSamplingInterval(us);
}

void UJavascriptProfile::SetIdle(const FJavascriptCpuProfiler& Profiler, bool is_idle)
{
	reinterpret_cast<CpuProfiler*>(Profiler.Profiler)->SetIdle(is_idle);
}

FString UJavascriptLibrary::GetFunctionName(FJavascriptProfileNode Node)
{
	return StringFromV8(Isolate::GetCurrent(), reinterpret_cast<const CpuProfileNode*>(Node.Node)->GetFunctionName());
}
int32 UJavascriptLibrary::GetScriptId(FJavascriptProfileNode Node)
{
	return reinterpret_cast<const CpuProfileNode*>(Node.Node)->GetScriptId();
}
FString UJavascriptLibrary::GetScriptResourceName(FJavascriptProfileNode Node)
{
	return StringFromV8(Isolate::GetCurrent(), reinterpret_cast<const CpuProfileNode*>(Node.Node)->GetScriptResourceName());
}
int32 UJavascriptLibrary::GetLineNumber(FJavascriptProfileNode Node)
{
	return reinterpret_cast<const CpuProfileNode*>(Node.Node)->GetLineNumber();
}
int32 UJavascriptLibrary::GetColumnNumber(FJavascriptProfileNode Node)
{
	return reinterpret_cast<const CpuProfileNode*>(Node.Node)->GetColumnNumber();
}
int32 UJavascriptLibrary::GetHitLineCount(FJavascriptProfileNode Node)
{
	return reinterpret_cast<const CpuProfileNode*>(Node.Node)->GetHitLineCount();
}
FString UJavascriptLibrary::GetBailoutReason(FJavascriptProfileNode Node)
{
	return ANSI_TO_TCHAR(reinterpret_cast<const CpuProfileNode*>(Node.Node)->GetBailoutReason());
}
int32 UJavascriptLibrary::GetHitCount(FJavascriptProfileNode Node)
{
	return reinterpret_cast<const CpuProfileNode*>(Node.Node)->GetHitCount();
}
int32 UJavascriptLibrary::GetCallUid(FJavascriptProfileNode Node)
{
	return reinterpret_cast<const CpuProfileNode*>(Node.Node)->GetCallUid();
}
int32 UJavascriptLibrary::GetNodeId(FJavascriptProfileNode Node)
{
	return reinterpret_cast<const CpuProfileNode*>(Node.Node)->GetNodeId();
}
int32 UJavascriptLibrary::GetChildrenCount(FJavascriptProfileNode Node)
{
	return reinterpret_cast<const CpuProfileNode*>(Node.Node)->GetChildrenCount();
}
FJavascriptProfileNode UJavascriptLibrary::GetChild(FJavascriptProfileNode Node, int32 index)
{
	FJavascriptProfileNode out;
	out.Node = reinterpret_cast<const CpuProfileNode*>(Node.Node)->GetChild(index);
	return out;
}
int32 UJavascriptLibrary::GetDeoptInfosCount(FJavascriptProfileNode Node, int32 index)
{
	const auto& infos = reinterpret_cast<const CpuProfileNode*>(Node.Node)->GetDeoptInfos();
	return infos.size();
}
FString UJavascriptLibrary::GetDeoptInfo_Reason(FJavascriptProfileNode Node, int32 index)
{
	const auto& infos = reinterpret_cast<const CpuProfileNode*>(Node.Node)->GetDeoptInfos();
	return ANSI_TO_TCHAR(infos[index].deopt_reason);
}
FString UJavascriptLibrary::GetDeoptInfo_Stack(FJavascriptProfileNode Node, int32 index)
{
	FString out;
	const auto& infos = reinterpret_cast<const CpuProfileNode*>(Node.Node)->GetDeoptInfos();
	for (const auto& frame : infos[index].stack)
	{
		out.Append(FString::Printf(TEXT("%d:%d"), frame.script_id, frame.position));
	}
	return out;
}