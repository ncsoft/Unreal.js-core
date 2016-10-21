#include "JavascriptEditor.h"
#include "JavascriptGraphSchema.h"

UJavascriptGraphSchema::UJavascriptGraphSchema(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UJavascriptGraphSchema::GetGraphNodeContextActions(FGraphContextMenuBuilder& ContextMenuBuilder, int32 SubNodeFlags) const
{
	UEdGraph* Graph = (UEdGraph*)ContextMenuBuilder.CurrentGraph;
	UJavascriptEdGraphNode* OpNode = NewObject<UJavascriptEdGraphNode>(Graph);


}