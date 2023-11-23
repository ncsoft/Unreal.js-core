#include "JavaScriptModuleCompiler.h"

namespace module_compiler
{
	/*****************************************************************************
	 * char* readFile
	 * Reads file contents to a null-terminated string.
	 *****************************************************************************/
	char* readFile(char filename[]) {

		// Opening file; ifstream::ate is use to determine file size
		std::ifstream file;
		file.open(filename, std::ifstream::ate);
		char* contents;
		if (!file) {
			contents = new char[1];
			return contents;
		}

		// Get file size
		size_t file_size = file.tellg();

		// Return file pointer from end of the file (set by ifstream::ate) to beginning
		file.seekg(0);

		// Reading file to char array and returing it
		std::filebuf* file_buf = file.rdbuf();
		contents = new char[file_size + 1]();
		file_buf->sgetn(contents, file_size);
		file.close();
		return contents;
	}

	/*****************************************************************************
	 * void print
	 * Binding of simple console print function to the VM
	 *****************************************************************************/
	void print(const v8::FunctionCallbackInfo<v8::Value>& args) {

		// Getting arguments; error handling
		v8::Isolate* isolate = args.GetIsolate();
		v8::String::Utf8Value val(isolate, args[0]);
		if (*val == nullptr)
			isolate->ThrowException(
				v8::String::NewFromUtf8(isolate, "First argument of function is empty")
					.ToLocalChecked());

		// Printing
		printf("%s\n", *val);
	}

	/*****************************************************************************
	 * v8::MaybeLocal<v8::Module> loadModule
	 * Loads module from code[] without checking it
	 *****************************************************************************/
	v8::MaybeLocal<v8::Module> JSModuleCompiler::loadModule(char code[],
									  char name[],
									  v8::Local<v8::Context> cx) {

		// Convert char[] to VM's string type
		v8::Local<v8::String> vcode =
			v8::String::NewFromUtf8(cx->GetIsolate(), code).ToLocalChecked();

		// Create script origin to determine if it is module or not.
		// Only first and last argument matters; other ones are default values.
		// First argument gives script name (useful in error messages), last
		// informs that it is a module.
		v8::ScriptOrigin origin(
			v8::String::NewFromUtf8(cx->GetIsolate(), name).ToLocalChecked(),
			v8::Integer::New(cx->GetIsolate(), 0),
			v8::Integer::New(cx->GetIsolate(), 0), v8::False(cx->GetIsolate()),
			v8::Local<v8::Integer>(), v8::Local<v8::Value>(),
			v8::False(cx->GetIsolate()), v8::False(cx->GetIsolate()),
			v8::True(cx->GetIsolate()));

		// Compiling module from source (code + origin)
		v8::Context::Scope context_scope(cx);
		v8::ScriptCompiler::Source source(vcode, origin);
		v8::MaybeLocal<v8::Module> mod;
		mod = v8::ScriptCompiler::CompileModule(cx->GetIsolate(), &source);
		if (mod.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("Script failed to load"));
		}
		// Returning non-checked module
		return mod;
	}

	/*****************************************************************************
	 * v8::Local<v8::Module> checkModule
	 * Checks out module (if it isn't nullptr/empty)
	 *****************************************************************************/
	v8::Local<v8::Module> JSModuleCompiler::checkModule(v8::MaybeLocal<v8::Module> maybeModule,
									  v8::Local<v8::Context> cx) {

		// Checking out
		v8::Local<v8::Module> mod;
		if (!maybeModule.ToLocal(&mod)) {
			printf("Error loading module!\n");
			exit(EXIT_FAILURE);
		}

		// Instantianing (including checking out depedencies). It uses callResolve
		// as callback: check #
		v8::Maybe<bool> result = mod->InstantiateModule(cx, JSModuleCompiler::callResolve);
		if (result.IsNothing()) {
			printf("\nCan't instantiate module.\n");
			exit(EXIT_FAILURE);
		}

		// Returning check-out module
		return mod;
	}

	/*****************************************************************************
	 * v8::Local<v8::Value> execModule
	 * Executes module's code
	 *****************************************************************************/
	v8::Local<v8::Value> JSModuleCompiler::execModule(v8::Local<v8::Module> mod,
									v8::Local<v8::Context> cx,
									bool nsObject) {

		// Executing module with return value
		v8::Local<v8::Value> retValue;
		if (!mod->Evaluate(cx).ToLocal(&retValue)) {
			printf("Error evaluating module!\n");
			exit(EXIT_FAILURE);
		}

		// nsObject determins, if module namespace or return value has to be returned.
		// Module namespace is required during import callback; see lines # and #.
		if (nsObject)
			return mod->GetModuleNamespace();
		else
			return retValue;
	}

	/*****************************************************************************
	 * v8::MaybeLocal<v8::Module> callResolve
	 * Callback from static import.
	 *****************************************************************************/
	v8::MaybeLocal<v8::Module> JSModuleCompiler::callResolve(v8::Local<v8::Context> context,
										   v8::Local<v8::String> specifier,
										   v8::Local<v8::Module> referrer) {

		v8::String::Utf8Value str(context->GetIsolate(), specifier);

		// Return unchecked module
		return loadModule(readFile(*str), *str, context);
	}

	/*****************************************************************************
	 * v8::MaybeLocal<v8::Promise> callDynamic
	 * Callback from dynamic import.
	 *****************************************************************************/
	v8::MaybeLocal<v8::Promise> JSModuleCompiler::callDynamic(v8::Local<v8::Context> context,
											v8::Local<v8::ScriptOrModule> referrer,
											v8::Local<v8::String> specifier,
											v8::Local<v8::FixedArray> import_assertions) {

		// Promise resolver: that way promise for dynamic import can be rejected
		// or full-filed
		v8::Local<v8::Promise::Resolver> resolver =
			v8::Promise::Resolver::New(context).ToLocalChecked();
		v8::MaybeLocal<v8::Promise> promise(resolver->GetPromise());

		// Loading module (with checking)
		v8::String::Utf8Value name(context->GetIsolate(), specifier);
		v8::Local<v8::Module> mod =
			checkModule(loadModule(readFile(*name), *name, context), context);
		v8::Local<v8::Value> retValue = execModule(mod, context, true);

		// Resolving (fulfilling) promise with module global namespace
		resolver->Resolve(context, retValue);
		return promise;
	}

	/*****************************************************************************
	 * void callMeta
	 * Callback for module metadata. 
	 *****************************************************************************/
	void JSModuleCompiler::callMeta(v8::Local<v8::Context> context,
				  v8::Local<v8::Module> module,
				  v8::Local<v8::Object> meta) {

		// In this example, this is throw-away function. But it shows that you can
		// bind module's url. Here, placeholder is used.
		meta->Set(
			context,
			v8::String::NewFromUtf8(context->GetIsolate(), "url").ToLocalChecked(),
			v8::String::NewFromUtf8(context->GetIsolate(), "https://something.sh")
				.ToLocalChecked());
	}
	
}
