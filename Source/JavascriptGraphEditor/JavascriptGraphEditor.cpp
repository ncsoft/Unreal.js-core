// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#include "EdGraph/EdGraphNode.h"
#include "EdGraphUtilities.h"
#include "IJavascriptGraphEditor.h"
#include "JavascriptGraphEdNode.h"

DEFINE_LOG_CATEGORY(JavascriptGraphEditor)

class FGraphPanelNodeFactory_GenericGraph : public FGraphPanelNodeFactory
{
	virtual TSharedPtr<class SGraphNode> CreateNode(UEdGraphNode* Node) const override
	{
		if (UJavascriptGraphEdNode* GraphEdNode = Cast<UJavascriptGraphEdNode>(Node))
		{
			return SNew(SJavascriptGraphEdNode, GraphEdNode);
		}
		return nullptr;
	}
};

TSharedPtr<FGraphPanelNodeFactory> GraphPanelNodeFactory_GenericGraph;

class FJavascriptGraphEditor : public IJavascriptGraphEditor
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override
	{
		GraphPanelNodeFactory_GenericGraph = MakeShareable(new FGraphPanelNodeFactory_GenericGraph());
		FEdGraphUtilities::RegisterVisualNodeFactory(GraphPanelNodeFactory_GenericGraph);
	}

	virtual void ShutdownModule() override
	{
		if (GraphPanelNodeFactory_GenericGraph.IsValid())
		{
			FEdGraphUtilities::UnregisterVisualNodeFactory(GraphPanelNodeFactory_GenericGraph);
			GraphPanelNodeFactory_GenericGraph.Reset();
		}
	}
};

IMPLEMENT_MODULE( FJavascriptGraphEditor, JavascriptGraphEditor )
