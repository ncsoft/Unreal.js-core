#pragma once

#include "JavascriptMenuLibrary.h"
#include "JavascriptGraphEditorLibrary.generated.h"

class UEdGraph;
class UEdGraphPin;
class UEdGraphNode;
class UJavascriptGraphEdNode;

USTRUCT()
struct FJavascriptEdGraphPin
{
	GENERATED_BODY()

	FJavascriptEdGraphPin() {}
	FJavascriptEdGraphPin(UEdGraphPin* InPin) 
		: GraphPin(InPin) 
	{}

	UEdGraphPin* GraphPin;

	operator UEdGraphPin* () const
	{
		return GraphPin;
	}

	bool IsValid() const 
	{
		return GraphPin != nullptr;
	}
};

USTRUCT()
struct FJavascriptGraphMenuBuilder : public FJavascriptMenuBuilder
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	const UEdGraph* Graph;

	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	const UEdGraphNode* GraphNode;

	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	FJavascriptEdGraphPin GraphPin;

	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	bool bIsDebugging;
};

USTRUCT()
struct FJavascriptNodeCreator
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	UJavascriptGraphEdNode* Node;

	TSharedPtr<FGraphNodeCreator<UEdGraphNode>> Instance;
};

UCLASS()
class UJavascriptGraphEditorLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptNodeCreator NodeCreator(UJavascriptGraphEdGraph* Graph);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void Finalize(FJavascriptNodeCreator& Creator);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool SetNodeMetaData(UEdGraphSchema* Schema, UEdGraphNode* Node, FName KeyValue);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void MakeLinkTo(FJavascriptEdGraphPin A, FJavascriptEdGraphPin B);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void BreakLinkTo(FJavascriptEdGraphPin A, FJavascriptEdGraphPin B);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void BreakAllPinLinks(FJavascriptEdGraphPin A);
	
	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptEdGraphPin FindPin(UEdGraphNode* Node, const FString& PinName, EEdGraphPinDirection Direction);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FString GetPinName(FJavascriptEdGraphPin A);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool IsValid(FJavascriptEdGraphPin A);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static class UEdGraphNode* GetOwningNode(FJavascriptEdGraphPin A);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static EEdGraphPinDirection GetDirection(FJavascriptEdGraphPin A);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static TArray<FJavascriptEdGraphPin> GetLinkedTo(FJavascriptEdGraphPin A);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static TArray<FJavascriptEdGraphPin> GetPins(UEdGraphNode* Node);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool CanUserDeleteNode(UEdGraphNode* Node);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool CanDuplicateNode(UEdGraphNode* Node);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void DestroyNode(UEdGraphNode* Node);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void AutowireNewNode(UEdGraphNode* Node, FJavascriptEdGraphPin FromPin);
};