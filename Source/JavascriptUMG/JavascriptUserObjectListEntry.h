#pragma once

#include "Blueprint/IUserObjectListEntry.h"
#include "Blueprint/UserWidget.h"
#include "JavascriptUserObjectListEntry.generated.h"

UCLASS()
class JAVASCRIPTUMG_API UJavascriptUserObjectListEntry : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Javascript")
	UObject* Item;
};
