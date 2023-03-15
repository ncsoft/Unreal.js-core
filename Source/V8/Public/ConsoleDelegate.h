#include "V8PCH.h"

THIRD_PARTY_INCLUDES_START
#include <interface-types.h>
THIRD_PARTY_INCLUDES_END

namespace v8
{
	class UnrealConsoleDelegate : public debug::ConsoleDelegate
	{
	public:
		explicit UnrealConsoleDelegate(Isolate* isolate);
		void Debug(const debug::ConsoleCallArguments& args,
			const debug::ConsoleContext& context) override;
		void Error(const debug::ConsoleCallArguments& args,
			const debug::ConsoleContext& context) override;
		void Info(const debug::ConsoleCallArguments& args,
			const debug::ConsoleContext& context) override;
		void Log(const debug::ConsoleCallArguments& args,
			const debug::ConsoleContext& context) override;
		void Warn(const debug::ConsoleCallArguments& args,
			const debug::ConsoleContext& context) override;
		void Assert(const debug::ConsoleCallArguments& args,
			const debug::ConsoleContext& context) override;

		FString StringFromConsoleCallArgs(const debug::ConsoleCallArguments& args, int StartIndex = 0)
		{
			HandleScope handle_scope(isolate_);

			TArray<FString> ArgStrings;

			for (int Index = StartIndex; Index < args.Length(); Index++)
			{
				ArgStrings.Add(StringFromV8(isolate_, args[Index]));
			}

			return FString::Join(ArgStrings, TEXT(" "));
		}

	private:
		Isolate* isolate_;
	};


	UnrealConsoleDelegate::UnrealConsoleDelegate(Isolate* isolate)
		: isolate_(isolate)
	{
	}

	void UnrealConsoleDelegate::Debug(const debug::ConsoleCallArguments& args, const debug::ConsoleContext& context)
	{
		UE_LOG(LogJavascript, Log, TEXT("%s"), *StringFromConsoleCallArgs(args));
	}
	void UnrealConsoleDelegate::Error(const debug::ConsoleCallArguments& args, const debug::ConsoleContext& context)
	{
		UE_LOG(LogJavascript, Error, TEXT("%s"), *StringFromConsoleCallArgs(args));
	}
	void UnrealConsoleDelegate::Info(const debug::ConsoleCallArguments& args, const debug::ConsoleContext& context)
	{
		UE_LOG(LogJavascript, Display, TEXT("%s"), *StringFromConsoleCallArgs(args));
	}
	void UnrealConsoleDelegate::Log(const debug::ConsoleCallArguments& args, const debug::ConsoleContext& context)
	{
		UE_LOG(LogJavascript, Log, TEXT("%s"), *StringFromConsoleCallArgs(args));
	}
	void UnrealConsoleDelegate::Warn(const debug::ConsoleCallArguments& args, const debug::ConsoleContext& context)
	{
		UE_LOG(LogJavascript, Warning, TEXT("%s"), *StringFromConsoleCallArgs(args));
	}
	void UnrealConsoleDelegate::Assert(const debug::ConsoleCallArguments& args, const debug::ConsoleContext& context)
	{
		bool to_assert = args.Length() < 1 || args[0]->IsFalse();
		if (to_assert)
		{
			auto stack_frame = StackTrace::CurrentStackTrace(isolate_, 1, StackTrace::kOverview)->GetFrame(isolate_, 0);
			auto filename = stack_frame->GetScriptName();
			auto line_number = stack_frame->GetLineNumber();

			UE_LOG(LogJavascript, Error, TEXT("Assertion:%s:%d %s"), *StringFromV8(isolate_, filename), line_number, *StringFromConsoleCallArgs(args, 1));
		}
	}

}