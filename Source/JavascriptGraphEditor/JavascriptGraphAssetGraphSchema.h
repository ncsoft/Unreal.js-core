#pragma once

#include "JavascriptUMGLibrary.h"
#include "JavascriptGraphEditorLibrary.h"
#include "ConnectionDrawingPolicy.h"
#include "JavascriptGraphAssetGraphSchema.generated.h"

class UJavascriptGraphAssetGraphSchema;

/** Action to add a node to the graph */
USTRUCT()
struct FJavascriptGraphAction_NewNode : public FEdGraphSchemaAction
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY()
	const UJavascriptGraphAssetGraphSchema* Schema;

	FJavascriptGraphAction_NewNode() {}
	FJavascriptGraphAction_NewNode(const FEdGraphSchemaAction& Template, const UJavascriptGraphAssetGraphSchema* InSchema);

	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, UEdGraphPin* FromPin, const FVector2D Location, bool bSelectNewNode);
	virtual UEdGraphNode* PerformAction(class UEdGraph* ParentGraph, TArray<UEdGraphPin*>& FromPins, const FVector2D Location, bool bSelectNewNode);
};

UENUM()
enum class EGraphSchemaGetStringQuery : uint8
{
	Description,
	Title,
};

USTRUCT()
struct FJavascriptPinConnectionResponse
{
	GENERATED_BODY()

	UPROPERTY()
	FText Message;

	UPROPERTY()
	TEnumAsByte<enum ECanCreateConnectionResponse> Response;
};

USTRUCT()
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

USTRUCT()
struct FPerformActionContext
{
	GENERATED_BODY()

	UPROPERTY()
	class UEdGraph* ParentGraph;

	UPROPERTY()
	TArray<FJavascriptEdGraphPin> FromPins;

	UPROPERTY()
	FVector2D Location;

	UPROPERTY()
	bool bSelectNewNode;
};

UCLASS(MinimalAPI)
class UJavascriptGraphAssetGraphSchema : public UEdGraphSchema
{
	GENERATED_BODY()

public:
	/** Delegate for constructing a UWidget based on a UObject */
	DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnDetermineWiringStyle, FJavascriptEdGraphPin, A, FJavascriptEdGraphPin, B, FJavascriptConnectionParams&, Params);

	/** Delegate for constructing a UWidget based on a UObject */
	DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(FVector2D, FOnVectorArith, const FVector2D&, A, const FVector2D&, B);

	/** Delegate for constructing a UWidget based on a UObject */
	DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnDrawSplineWithArrow, const FVector2D&, A, const FVector2D&, B, const FJavascriptConnectionParams&, Params);
	DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnDrawSplineWithArrow_Geom, const FGeometry&, A, const FGeometry&, B, const FJavascriptConnectionParams&, Params);
	DECLARE_DYNAMIC_DELEGATE_FourParams(FOnDrawPreviewConnector, const FGeometry&, PinGeometry, const FVector2D&, StartPoint, const FVector2D&, EndPoint, FJavascriptEdGraphPin, Pin);	

	/** Delegate for constructing a UWidget based on a UObject */
	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(FJavascriptSlateWidget, FOnTakeWidget, UJavascriptGraphEdNode*, Instance);

	/** Delegate for constructing a UWidget based on a UObject */
	DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(FText, FOnGetString, const UJavascriptGraphEdNode*, Instance, EGraphSchemaGetStringQuery, Query);

	/** Delegate for constructing a UWidget based on a UObject */
	DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(FJavascriptPinConnectionResponse, FOnCanCreateConnection, FJavascriptEdGraphPin, A, FJavascriptEdGraphPin, B);

	DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(UEdGraphNode*, FOnPerformAction, const FEdGraphSchemaAction&, Action, FPerformActionContext&, Context);	

	DECLARE_DYNAMIC_DELEGATE_RetVal(TArray<FEdGraphSchemaAction>, FOnContextActions);

	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(FLinearColor, FOnGetPinTypeColor, const FEdGraphPinType&, PinType);
	
	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnBuildMenu, FJavascriptGraphMenuBuilder&, Builder);

	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnEdNodeAction, UJavascriptGraphEdNode*, Node);

	/** Delegate for constructing a UWidget based on a UObject */
	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(FJavascriptSlateWidget, FOnCreatePin, FJavascriptEdGraphPin, Pin);

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnDetermineWiringStyle OnDetermineWiringStyle;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnVectorArith OnComputeSplineTangent;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnDrawSplineWithArrow OnDrawSplineWithArrow;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnDrawSplineWithArrow_Geom OnDrawSplineWithArrow_Geom;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnDrawPreviewConnector OnDrawPreviewConnector;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnTakeWidget OnTakeWidget;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGetString OnGetString;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnBuildMenu OnBuildMenu;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnEdNodeAction OnAllocateDefaultPins;	

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnCreatePin OnCreatePin;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnCanCreateConnection OnCanCreateConnection;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnPerformAction OnPerformAction;	

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnContextActions OnContextActions;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGetPinTypeColor OnGetPinTypeColor;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnEdNodeAction OnNodeConnectionListChanged;

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	void BreakPinLinks(FJavascriptEdGraphPin TargetPin, bool bSendsNodeNotifcation);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	void BreakSinglePinLink(FJavascriptEdGraphPin SourcePin, FJavascriptEdGraphPin TargetPin);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	void BreakNodeLinks(UEdGraphNode* TargetNode);

	//~ Begin EdGraphSchema Interface
 	virtual void GetGraphContextActions(FGraphContextMenuBuilder& ContextMenuBuilder) const override;
 	virtual void GetContextMenuActions(const UEdGraph* CurrentGraph, const UEdGraphNode* InGraphNode, const UEdGraphPin* InGraphPin, class FMenuBuilder* MenuBuilder, bool bIsDebugging) const override;
 	virtual const FPinConnectionResponse CanCreateConnection(const UEdGraphPin* A, const UEdGraphPin* B) const override;
	virtual class FConnectionDrawingPolicy* CreateConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, class FSlateWindowElementList& InDrawElements, class UEdGraph* InGraphObj) const override;
 	virtual FLinearColor GetPinTypeColor(const FEdGraphPinType& PinType) const override;
	virtual void BreakNodeLinks(UEdGraphNode& TargetNode) const override;
	virtual void BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const override;
	virtual void BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) override;
	//~ End EdGraphSchema Interface
};

