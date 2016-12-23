#include "JavascriptGraphEditorPrivatePCH.h"
#include "JavascriptGraphEditorLibrary.h"

#define LOCTEXT_NAMESPACE "JavascriptGraph"

FJavascriptNodeCreator UJavascriptGraphEditorLibrary::NodeCreator(UJavascriptGraphEdGraph* Graph)
{
	FJavascriptNodeCreator Out;
	auto Creator = new FGraphNodeCreator<UJavascriptGraphEdNode>(*Graph);
	Out.Instance = MakeShareable(reinterpret_cast<FGraphNodeCreator<UEdGraphNode>*>(Creator));
	Out.Node = Creator->CreateNode();
	return Out;
}

void UJavascriptGraphEditorLibrary::Finalize(FJavascriptNodeCreator& Creator)
{
	Creator.Instance->Finalize();
}

bool UJavascriptGraphEditorLibrary::SetNodeMetaData(UEdGraphSchema* Schema, UEdGraphNode* Node, FName KeyValue)
{
	return Schema->SetNodeMetaData(Node, KeyValue);
}

void UJavascriptGraphEditorLibrary::MakeLinkTo(FJavascriptEdGraphPin A, FJavascriptEdGraphPin B)
{
	if (A.GraphPin && B.GraphPin)
	{
		A.GraphPin->MakeLinkTo(B.GraphPin);
	}	
}

void UJavascriptGraphEditorLibrary::BreakLinkTo(FJavascriptEdGraphPin A, FJavascriptEdGraphPin B)
{
	if (A.GraphPin && B.GraphPin)
	{
		A.GraphPin->BreakLinkTo(B.GraphPin);
	}
}

void UJavascriptGraphEditorLibrary::BreakAllPinLinks(FJavascriptEdGraphPin A)
{
	if (A.GraphPin)
	{
		A.GraphPin->BreakAllPinLinks();
	}
}

FJavascriptEdGraphPin UJavascriptGraphEditorLibrary::FindPin(UEdGraphNode* Node, const FString& PinName, EEdGraphPinDirection Direction)
{
	return FJavascriptEdGraphPin{ Node->FindPin(PinName, Direction) };
}

FString UJavascriptGraphEditorLibrary::GetPinName(FJavascriptEdGraphPin A)
{
	return A.GraphPin ? A.GraphPin->PinName : TEXT("");
}

bool UJavascriptGraphEditorLibrary::IsValid(FJavascriptEdGraphPin A)
{
	return A.GraphPin != nullptr;
}

class UEdGraphNode* UJavascriptGraphEditorLibrary::GetOwningNode(FJavascriptEdGraphPin A)
{
	return A.GraphPin ? A.GraphPin->GetOwningNode() : nullptr;
}

EEdGraphPinDirection UJavascriptGraphEditorLibrary::GetDirection(FJavascriptEdGraphPin A)
{
	return A.GraphPin ? A.GraphPin->Direction : EEdGraphPinDirection::EGPD_Input;
}

TArray<FJavascriptEdGraphPin> TransformPins(const TArray<UEdGraphPin*>& Pins)
{
	TArray<FJavascriptEdGraphPin> Out;
	for (auto x : Pins)
	{
		Out.Add(FJavascriptEdGraphPin{ x });
	}
	return Out;
}

TArray<FJavascriptEdGraphPin> UJavascriptGraphEditorLibrary::GetLinkedTo(FJavascriptEdGraphPin A)
{
	return TransformPins(A.GraphPin->LinkedTo);
}

TArray<FJavascriptEdGraphPin> UJavascriptGraphEditorLibrary::GetPins(UEdGraphNode* Node)
{
	return TransformPins(Node->Pins);
}

#undef LOCTEXT_NAMESPACE
