#include "JavascriptEditorObjectManager.h"
#include "Engine/Engine.h"

bool IsCDO(UObject* Object)
{
	check(Object);

	UClass* ObjectClsss = Object->GetClass();
	if (ObjectClsss)
	{
		UObject* CDO = ObjectClsss->GetDefaultObject();
		if (Object == CDO)
		{
			return true;
		}
	}

	return false;
}

void UJavascriptEditorObjectManager::BeginDestroy()
{
	if (IsCDO(this) == false)
	{
		Clear(true);
	}

	Super::BeginDestroy();
}

void UJavascriptEditorObjectManager::Clear(bool bWithClass)
{
	for (auto& ObjectList : ObjectPathMap)
	{
		for (auto& Object : ObjectList.Value)
		{
			Object->RemoveFromRoot();
			JAVASCRIPT_EDITOR_DESTROY_UOBJECT(Object);
		}
	}
	ObjectPathMap.Reset();

	if (bWithClass)
	{
		for (auto& RefClass : RefClassPathMap)
		{
			(RefClass.Value)->RemoveFromRoot();
		}
		RefClassPathMap.Reset();
	}

	if (GEngine)
	{
		GEngine->ForceGarbageCollection(true);
	}
}

bool UJavascriptEditorObjectManager::SetObject(FString Key, UObject* Value)
{
	if (Value)
	{
		// @todo: Add `Object Property delegate`
		if (ObjectPathMap.Contains(Key))
		{
			TArray<UObject*>* ObjectList = (ObjectPathMap.Find(Key));
			ObjectList->Add(Value);
		}
		else
		{
			TArray<UObject*> ObjectList;
			ObjectList.Add(Value);
			ObjectPathMap.Add(Key, ObjectList);
		}
		Value->AddToRoot();
		return true;
	}
	return false;
}

TArray<UObject*> UJavascriptEditorObjectManager::GetObjects(FString Key)
{
	TArray<UObject*> ObjectList;
	if (ObjectPathMap.Contains(Key))
	{
		ObjectList = *(ObjectPathMap.Find(Key));
	}
	return ObjectList;
}

void UJavascriptEditorObjectManager::RemoveObjects(FString Key)
{
	if (ObjectPathMap.Contains(Key))
	{
		TArray<UObject*> ObjectList = *(ObjectPathMap.Find(Key));
		for (auto& Object : ObjectList)
		{
			(Object)->RemoveFromRoot();
			JAVASCRIPT_EDITOR_DESTROY_UOBJECT(Object);			
		}
		ObjectPathMap.Remove(Key);
	}
}

TArray<FString> UJavascriptEditorObjectManager::GetObjectKeys()
{
	TArray<FString> OutKeys;
	ObjectPathMap.GetKeys(OutKeys);
	return OutKeys;
}

UClass* UJavascriptEditorObjectManager::GetRef(FString Key)
{
	if (RefClassPathMap.Contains(Key))
	{
		return *(RefClassPathMap.Find(Key));
	}
	return nullptr;
}

UScriptStruct* UJavascriptEditorObjectManager::GetStructRef(FString Key)
{
	if (RefStructPathMap.Contains(Key))
	{
		return *(RefStructPathMap.Find(Key));
	}
	return nullptr;
}

bool UJavascriptEditorObjectManager::SetRef(FString Key, UClass* Value, bool bOverride)
{
	if (Value)
	{
		if (bOverride)
		{
			RemoveRef(Key);
		}
		else if (HasRef(Key))
		{
			UClass* PrevValue;
			RefClassPathMap.RemoveAndCopyValue(Key, PrevValue);
			PrevValue->RemoveFromRoot();
		}

		RefClassPathMap.Add(Key, Value);
		Value->AddToRoot();
		return true;
	}
	return false;
}

bool UJavascriptEditorObjectManager::SetStructRef(FString Key, UClass* Value, bool bOverride)
{
	if (UScriptStruct* StructRef = reinterpret_cast<UScriptStruct*>(Value))
	{
		if (bOverride)
		{
			RemoveStructRef(Key);
		}
		else if (HasStructRef(Key))
		{
			UScriptStruct* PrevValue;
			RefStructPathMap.RemoveAndCopyValue(Key, PrevValue);
			PrevValue->RemoveFromRoot();
		}

		RefStructPathMap.Add(Key, StructRef);
		StructRef->AddToRoot();
		return true;
	}
	return false;
}

bool UJavascriptEditorObjectManager::HasRef(FString Key)
{
	return RefClassPathMap.Contains(Key);
}

bool UJavascriptEditorObjectManager::HasStructRef(FString Key)
{
	return RefStructPathMap.Contains(Key);
}

void UJavascriptEditorObjectManager::RemoveRef(FString Key)
{
	if (auto Value = GetRef(Key))
	{
		Value->RemoveFromRoot();
		RefClassPathMap.Remove(Key);
	}
}

void UJavascriptEditorObjectManager::RemoveStructRef(FString Key)
{
	if (auto Value = GetStructRef(Key))
	{
		Value->RemoveFromRoot();
		RefStructPathMap.Remove(Key);
	}
}
