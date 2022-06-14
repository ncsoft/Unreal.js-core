#pragma once

#include "JavascriptUMG/JavascriptUMGLibrary.h"
#include "JavascriptDetailCustomization.generated.h"

/**
 *
 */
UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptWholeRowDetailCustomization : public UObject
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	UPROPERTY()
	FName TypeName;

	UPROPERTY()
	FName CategoryName;

	UPROPERTY()
	FJavascriptSlateWidget CustomWidget { SNullWidget::NullWidget };

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	void Register();

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	void Unregister();
#endif
};

UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptDetailCustomization : public UObject
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	UPROPERTY()
	FName TypeName;

	UPROPERTY()
	FName CategoryName;

	UPROPERTY()
	FJavascriptSlateWidget NameWidget { SNullWidget::NullWidget };

	UPROPERTY()
	FJavascriptSlateWidget ValueWidget { SNullWidget::NullWidget };

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	void Register();

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	void Unregister();
#endif
};
