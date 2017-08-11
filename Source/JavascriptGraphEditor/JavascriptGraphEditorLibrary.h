#pragma once

#include "JavascriptMenuLibrary.h"
#include "SJavascriptGraphEdNode.h"
#include "ConnectionDrawingPolicy.h"
#include "EdGraph/EdGraph.h"
#include "JavascriptGraphEditorLibrary.generated.h"

class UEdGraph;
class UEdGraphPin;
class UEdGraphNode;
class UJavascriptGraphEdNode;

USTRUCT(BlueprintType)
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

USTRUCT(BlueprintType)
struct FJavascriptConnectionParams
{
	GENERATED_BODY()

		FJavascriptConnectionParams()
	{}

	FJavascriptConnectionParams(const FConnectionParams& In);

	UPROPERTY()
		FLinearColor WireColor;

	UPROPERTY()
		FJavascriptEdGraphPin AssociatedPin1;

	UPROPERTY()
		FJavascriptEdGraphPin AssociatedPin2;

	UPROPERTY()
		float WireThickness;

	UPROPERTY()
		bool bDrawBubbles;

	UPROPERTY()
		bool bUserFlag1;

	UPROPERTY()
		bool bUserFlag2;

	UPROPERTY()
		TEnumAsByte<EEdGraphPinDirection> StartDirection;

	UPROPERTY()
		TEnumAsByte<EEdGraphPinDirection> EndDirection;

	operator FConnectionParams () const;
};

USTRUCT(BlueprintType)
struct FJavascriptDetermineLinkGeometryContainer
{
	GENERATED_BODY()

	FJavascriptDetermineLinkGeometryContainer() {}
	FJavascriptDetermineLinkGeometryContainer(FArrangedChildren* InArrangedNodes, TSharedRef<SWidget>* InOutputPinWidget, TMap<UEdGraphNode*, int32>* InNodeWidgetMap, TMap<TSharedRef<SWidget>, FArrangedWidget>* InPinGeometries,	TMap< UEdGraphPin*, TSharedRef<SGraphPin> >* InPinToPinWidgetMap)
		: ArrangedNodes(InArrangedNodes)
		, OutputPinWidget(InOutputPinWidget)
		, NodeWidgetMap(InNodeWidgetMap)
		, PinGeometries(InPinGeometries)
		, PinToPinWidgetMap(InPinToPinWidgetMap)
	{}

	FArrangedChildren* ArrangedNodes;
	TSharedRef<SWidget>* OutputPinWidget;

	TMap<UEdGraphNode*, int32>* NodeWidgetMap;

	TMap<TSharedRef<SWidget>, FArrangedWidget>* PinGeometries;
	TMap< UEdGraphPin*, TSharedRef<SGraphPin> >* PinToPinWidgetMap;
};

USTRUCT(BlueprintType)
struct FJavascriptPerformSecondPassLayoutContainer
{
	GENERATED_BODY()

	UPROPERTY()
	UEdGraphNode* PrevNode;

	UPROPERTY()
	UEdGraphNode* NextNode;

	UPROPERTY()
	int32 NodeIndex;

	UPROPERTY()
	int32 MaxNodes;
};

USTRUCT(BlueprintType)
struct FJavascriptArrangedWidget
{
	GENERATED_BODY()

	FJavascriptArrangedWidget() {}
	FJavascriptArrangedWidget(FArrangedWidget* InHandle)
		:Handle(InHandle)
	{}
	FArrangedWidget* Handle;
};

USTRUCT(BlueprintType)
struct FJavascriptPinWidget
{
	GENERATED_BODY()

	TSharedRef<SWidget>* Handle;
};

USTRUCT(BlueprintType)
struct FJavascriptGraphConnectionDrawingPolicyContainer
{
	GENERATED_BODY()

	FJavascriptGraphConnectionDrawingPolicyContainer() {}
	FJavascriptGraphConnectionDrawingPolicyContainer(class FJavascriptGraphConnectionDrawingPolicy* InHandle)
		:Handle(InHandle)
	{}

	class FJavascriptGraphConnectionDrawingPolicy* Handle;
};

USTRUCT(BlueprintType)
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

USTRUCT(BlueprintType)
struct FJavascriptNodeCreator
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Javascript | Editor")
	UJavascriptGraphEdNode* Node;

	TSharedPtr<FGraphNodeCreator<UEdGraphNode>> Instance;
};

USTRUCT(BlueprintType)
struct FJavascriptSlateEdNode
{
	GENERATED_BODY()

	FJavascriptSlateEdNode() {}
	FJavascriptSlateEdNode(SJavascriptGraphEdNode* InHandle)
		:Handle(InHandle)
	{}

	SJavascriptGraphEdNode* Handle;
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
	static void TryConnection(UEdGraphSchema* Schema, FJavascriptEdGraphPin A, FJavascriptEdGraphPin B);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void BreakLinkTo(FJavascriptEdGraphPin A, FJavascriptEdGraphPin B);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void BreakAllPinLinks(FJavascriptEdGraphPin A);

	UFUNCTION(BlueprintInternalUseOnly, Category = "Scripting | Javascript")
	static FEdGraphPinType GetPinType(FJavascriptEdGraphPin A);

	UFUNCTION(BlueprintInternalUseOnly, Category = "Scripting | Javascript")
	static void SetPinType(FJavascriptEdGraphPin Pin, FEdGraphPinType PinType);
	
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
	
	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptArrangedWidget FindPinGeometries(FJavascriptDetermineLinkGeometryContainer Container, FJavascriptPinWidget PinWidget);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptPinWidget FindPinToPinWidgetMap(FJavascriptDetermineLinkGeometryContainer Container, FJavascriptEdGraphPin Pin);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptArrangedWidget GetArrangedNodes(FJavascriptDetermineLinkGeometryContainer Container, UEdGraphNode* Node);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptPinWidget GetOutputPinWidget(FJavascriptDetermineLinkGeometryContainer Container);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void DrawConnection(FJavascriptGraphConnectionDrawingPolicyContainer Container, const FVector2D& A, const FVector2D& B, const FJavascriptConnectionParams& Params);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void MakeRotatedBox(FJavascriptGraphConnectionDrawingPolicyContainer Container, FVector2D ArrowDrawPos, float AngleInRadians, FLinearColor WireColor);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static int GetHorveredPinNum(FJavascriptGraphConnectionDrawingPolicyContainer Container);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool IsContainedHoveredPins(FJavascriptGraphConnectionDrawingPolicyContainer Container, FJavascriptEdGraphPin Pin);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void ApplyHoverDeemphasis(FJavascriptGraphConnectionDrawingPolicyContainer Container, FJavascriptEdGraphPin OutputPin, FJavascriptEdGraphPin InputPin, float Thickness, FLinearColor WireColor);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void DetermineWiringStyle(FJavascriptGraphConnectionDrawingPolicyContainer Container, FJavascriptEdGraphPin OutputPin, FJavascriptEdGraphPin InputPin, FJavascriptConnectionParams& Params);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void DrawSplineWithArrow(FJavascriptGraphConnectionDrawingPolicyContainer Container, FVector2D StartAnchorPoint, FVector2D EndAnchorPoint, FJavascriptConnectionParams Params);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void AddPinToHoverSet(const FJavascriptSlateEdNode& InSlateEdNode, FJavascriptEdGraphPin Pin);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void RemovePinFromHoverSet(const FJavascriptSlateEdNode& InSlateNode, FJavascriptEdGraphPin Pin);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FVector2D CenterOf(const FGeometry& Geom) { return FGeometryHelper::CenterOf(Geom); };

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FVector2D FindClosestPointOnGeom(const FGeometry& Geom, const FVector2D& TestPoint) { return FGeometryHelper::FindClosestPointOnGeom(Geom, TestPoint); };

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptEdGraphPin GetDefaultObject() { return FJavascriptEdGraphPin(nullptr); };
};