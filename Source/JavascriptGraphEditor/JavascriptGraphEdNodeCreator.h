#pragma once

#include "EdGraph/EdGraph.h"

// Forward decl.
class UJavascriptGraphEdGraph;
class UJavascriptGraphEdNode;


class IJavascriptGraphNodeCreator
{
public:
	IJavascriptGraphNodeCreator(UJavascriptGraphEdGraph* InGraph) {}
	virtual ~IJavascriptGraphNodeCreator() {}

	// Should never be copied or moved
	IJavascriptGraphNodeCreator(IJavascriptGraphNodeCreator& rhs) = delete;
	IJavascriptGraphNodeCreator(IJavascriptGraphNodeCreator&& rhs) = delete;
	IJavascriptGraphNodeCreator& operator=(IJavascriptGraphNodeCreator& rhs) = delete;
	IJavascriptGraphNodeCreator& operator=(IJavascriptGraphNodeCreator&& rhs) = delete;

	virtual UJavascriptGraphEdNode* CreateNode(bool bSelectNewNode = true) = 0;
	virtual UJavascriptGraphEdNode* CreateUserInvokedNode(bool bSelectNewNode = true) = 0;
	virtual void Finalize() = 0;
};

class FDefaultJavascriptGraphNodeCreator : public IJavascriptGraphNodeCreator
{
public:
	FDefaultJavascriptGraphNodeCreator(UJavascriptGraphEdGraph* InGraph);

	virtual UJavascriptGraphEdNode* CreateNode(bool bSelectNewNode = true) override;
	virtual UJavascriptGraphEdNode* CreateUserInvokedNode(bool bSelectNewNode = true) override;
	virtual void Finalize() override;

private:
	TSharedPtr<FGraphNodeCreator<UJavascriptGraphEdNode>> Instance;
};

// Implementation of FCustomJavascriptGraphNodeCreator is based on FGraphNodeCreator in UE 4.21.2.
class FCustomJavascriptGraphNodeCreator : public IJavascriptGraphNodeCreator
{
public:
	FCustomJavascriptGraphNodeCreator(UJavascriptGraphEdGraph* InGraph);
	virtual ~FCustomJavascriptGraphNodeCreator();

	virtual UJavascriptGraphEdNode* CreateNode(bool bSelectNewNode = true) override;
	virtual UJavascriptGraphEdNode* CreateUserInvokedNode(bool bSelectNewNode = true) override;
	virtual void Finalize() override;

private:
	TWeakObjectPtr<UJavascriptGraphEdGraph> Graph;
	UJavascriptGraphEdNode* Node;
	bool bPlaced;
};
