#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "JavascriptEditorObjectManager.generated.h"

#define JAVASCRIPT_EDITOR_DESTROY_UOBJECT(UObjectVariable) \
{ \
	if (::IsValid(UObjectVariable) && UObjectVariable->IsValidLowLevel()) \
	{ \
		UObjectVariable->ConditionalBeginDestroy(); \
	} \
}

/**
 * 
 */
UCLASS(BlueprintType)
class JAVASCRIPTEDITOR_API UJavascriptEditorObjectManager : public UObject
{
	GENERATED_BODY()

public:
	//~ UObject interface
	virtual void BeginDestroy() override;

public:

	UFUNCTION()
	void Clear(bool bWithClass = false);
	
	UFUNCTION()
	bool SetObject(FString Key, UObject* Value);
	
	UFUNCTION()
	TArray<UObject*> GetObjects(FString Key);

	UFUNCTION()
	void RemoveObjects(FString Key);

	UFUNCTION()
	TArray<FString> GetObjectKeys();

	UFUNCTION()
	UClass* GetRef(FString Key);

	UFUNCTION()
	UScriptStruct* GetStructRef(FString Key);

	UFUNCTION()
	bool SetRef(FString Key, UClass* Value, bool bOverride = false);
	
	UFUNCTION()
	bool SetStructRef(FString Key, UClass* Value, bool bOverride = false);

	UFUNCTION()
	bool HasRef(FString Key);

	UFUNCTION()
	bool HasStructRef(FString Key);

	UFUNCTION()
	void RemoveRef(FString Key);

	UFUNCTION()
	void RemoveStructRef(FString Key);

private:	
	TMap<FString, TArray<UObject*>> ObjectPathMap;
	TMap<FString, UClass*> RefClassPathMap;
	TMap<FString, UScriptStruct*> RefStructPathMap;
};
