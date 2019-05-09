#pragma once

#include "Translator.h"
#include "V8PCH.h"

struct FV8Exception
{
	static FString Report(v8::Isolate* isolate, v8::TryCatch& try_catch)
	{
		using namespace v8;

		auto exception = StringFromV8(isolate, try_catch.Exception());
		auto message = try_catch.Message();
		if (message.IsEmpty())
		{
			UE_LOG(Javascript, Error, TEXT("%s"), *exception);
			return *exception;
		}
		else
		{
			if (!exception.IsEmpty())
			{
				auto filename = StringFromV8(isolate, message->GetScriptResourceName());
				auto linenum = message->GetLineNumber(isolate->GetCurrentContext()).ToChecked();
				auto line = StringFromV8(isolate, message->GetSourceLine(isolate->GetCurrentContext()).ToLocalChecked());

				UE_LOG(Javascript, Error, TEXT("%s:%d: %s"), *filename, linenum, *exception);

				auto maybe_stack_trace = try_catch.StackTrace(isolate->GetCurrentContext());

				if (maybe_stack_trace.IsEmpty())
				{
					UE_LOG(Javascript, Error, TEXT("%s"), *exception);
					return *exception;
				}

				auto stack_trace = StringFromV8(isolate, maybe_stack_trace.ToLocalChecked());
				if (stack_trace.Len() > 0)
				{
					TArray<FString> Lines;
					stack_trace.ParseIntoArrayLines(Lines);
					for (const auto& line : Lines)
					{
						UE_LOG(Javascript, Error, TEXT("%s"), *line);
					}

					return stack_trace;
				} 
				else
				{
					return *exception;
				}
			}
		}

		return FString();
	}
};