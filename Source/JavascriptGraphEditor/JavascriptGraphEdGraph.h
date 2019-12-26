#pragma once

#include "JavascriptGraphEdGraph.generated.h"

class UJavascriptGraphEdNode;

UCLASS()
class UJavascriptGraphEdGraph : public UEdGraph
{
	GENERATED_BODY()

public:
	UJavascriptGraphEdGraph();
	virtual ~UJavascriptGraphEdGraph();

	UFUNCTION()
	void RebuildGenericGraph();

#if WITH_EDITOR
	virtual void PostEditUndo() override;
#endif

	void AddCustomNode(UJavascriptGraphEdNode* NodeToAdd, bool bUserAction = false);
	bool RemoveCustomNode(UJavascriptGraphEdNode* NodeToRemove);

protected:
	friend class FCustomJavascriptGraphNodeCreator;

	UJavascriptGraphEdNode* CreateCustomNode(TSubclassOf<UJavascriptGraphEdNode> NewNodeClass, bool bFromUI);
	UJavascriptGraphEdNode* CreateCustomNode(TSubclassOf<UJavascriptGraphEdNode> NewNodeClass)
	{
		return CreateCustomNode(NewNodeClass, false);
	}
	UJavascriptGraphEdNode* CreateUserInvokedCustomNode(TSubclassOf<UJavascriptGraphEdNode> NewNodeClass)
	{
		return CreateCustomNode(NewNodeClass, true);
	}

private:
	UPROPERTY(Transient)
	TArray<UJavascriptGraphEdNode*> CustomNodes;
};
