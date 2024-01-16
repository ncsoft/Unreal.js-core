#pragma once
/**********************************************************
 * Javascript EC6 (modules) compiler
 * by Jeffrey Kesselman (jpkessel@purdue.edu)
 * 
 * Heavily cribbed from https://gist.github.com/surusek/4c05e4dcac6b82d18a1a28e6742fc23e
 *
 **********************************************************/
#include <libplatform/libplatform.h>
#include <v8.h>

namespace module_compiler
{
	class JSModuleCompiler
	{
	private:
		static v8::MaybeLocal<v8::Module> callResolve(v8::Local<v8::Context> context, v8::Local<v8::String> specifier,
													  v8::Local<v8::Module> referrer);
		static v8::MaybeLocal<v8::Promise> callDynamic(v8::Local<v8::Context> context, v8::Local<v8::ScriptOrModule> referrer,
													   v8::Local<v8::String> specifier,
													   v8::Local<v8::FixedArray> import_assertions);
		static void callMeta(v8::Local<v8::Context> context, v8::Local<v8::Module> module, v8::Local<v8::Object> meta);

	public:
		static v8::MaybeLocal<v8::Module> loadModule(char code[],
										  char name[],
										  v8::Local<v8::Context> cx);

		// Check, if module isn't empty (or pointer to it); line #221
		static v8::Local<v8::Module> checkModule(v8::MaybeLocal<v8::Module> maybeModule,
										  v8::Local<v8::Context> cx);

		// Executes module; line #247
		static v8::Local<v8::Value> execModule(v8::Local<v8::Module> mod,
										v8::Local<v8::Context> cx,
										bool nsObject = false);
		
	};
}
