#pragma once

#include "CoreMinimal.h"
#include "V8PCH.h"

namespace v8
{
	class Isolate;
	template <class T> class Local;
	class Object;
	class Value;

	struct IDelegateManager
	{
		static IDelegateManager* Create(Isolate* isolate);
		virtual void Destroy() = 0;
		virtual Local<Value> GetProxy(Local<Object> This, UObject* Object, FProperty* Property) = 0;
	};
}