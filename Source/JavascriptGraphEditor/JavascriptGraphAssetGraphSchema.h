#pragma once

#include "JavascriptUMGLibrary.h"
#include "JavascriptGraphEditorLibrary.h"
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
	DECLARE_DYNAMIC_DELEGATE_FourParams(FOnDetermineWiringStyle, FJavascriptEdGraphPin, A, FJavascriptEdGraphPin, B, FJavascriptConnectionParams&, Params, FJavascriptGraphConnectionDrawingPolicyContainer, Container);

	DECLARE_DYNAMIC_DELEGATE_RetVal_SixParams(bool, FOnDrawPreviewConnector, const FGeometry&, PinGeometry, const FVector2D&, StartPoint, const FVector2D&, EndPoint, FJavascriptEdGraphPin, Pin, const FJavascriptConnectionParams&, Params, FJavascriptGraphConnectionDrawingPolicyContainer, Container);

	/** Delegate for constructing a UWidget based on a UObject */
	DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(FVector2D, FOnVectorArith, const FVector2D&, A, const FVector2D&, B);

	/** Delegate for constructing a UWidget based on a UObject */
	DECLARE_DYNAMIC_DELEGATE_RetVal_FiveParams(bool, FOnDrawSplineWithArrow, const FVector2D&, A, const FVector2D&, B, const FJavascriptConnectionParams&, Params, FJavascriptGraphConnectionDrawingPolicyContainer, Container, FVector2D, ArrowRadius);
	DECLARE_DYNAMIC_DELEGATE_RetVal_FourParams(bool, FOnDrawSplineWithArrow_Geom, const FGeometry&, A, const FGeometry&, B, const FJavascriptConnectionParams&, Params, FJavascriptGraphConnectionDrawingPolicyContainer, Container);

	/** Delegate for constructing a UWidget based on a UObject */
	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(FJavascriptSlateWidget, FOnTakeWidget, UJavascriptGraphEdNode*, Instance);

	/** Delegate for constructing a UWidget based on a UObject */
	DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(FText, FOnGetString, const UJavascriptGraphEdNode*, Instance, EGraphSchemaGetStringQuery, Query);

	/** Delegate for constructing a UWidget based on a UObject */
	DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(FJavascriptPinConnectionResponse, FOnCanCreateConnection, FJavascriptEdGraphPin, A, FJavascriptEdGraphPin, B);

	DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(UEdGraphNode*, FOnPerformAction, const FEdGraphSchemaAction&, Action, FPerformActionContext&, Context);	

	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(TArray<FEdGraphSchemaAction>, FOnContextActions, FJavascriptEdGraphPin, FromPin);
	
	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnBuildMenu, FJavascriptGraphMenuBuilder&, Builder);

	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnEdNodeAction, UJavascriptGraphEdNode*, Node);

	DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(bool, FOnCreateAutomaticConversionNodeAndConnections, FJavascriptEdGraphPin, A, FJavascriptEdGraphPin, B);
	
	/** Delegate for constructing a UWidget based on a UObject */
	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(FJavascriptSlateWidget, FOnCreatePin, FJavascriptEdGraphPin, Pin);
	
	DECLARE_DYNAMIC_DELEGATE_RetVal_FiveParams(bool, FOnDetermineLinkGeometry, FJavascriptEdGraphPin, OutPin, FJavascriptEdGraphPin, InputPin, FJavascriptArrangedWidget&, StartWidgetGeometry, FJavascriptArrangedWidget&, EndWidgetGeometry, FJavascriptDetermineLinkGeometryContainer, Container);

	DECLARE_DYNAMIC_DELEGATE_RetVal(bool, FOnGetBoolean);

	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FOnGetBooleanWidget, UJavascriptGraphEdNode*, Instance);
	
	DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(FName, FOnGetSlateBrushName, bool, bHovered, FJavascriptEdGraphPin, Pin);

	DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnMoveTo, UJavascriptGraphEdNode*, Instance, const FVector2D&, NewPosition);

	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(FJavascriptPerformSecondPassLayoutContainer, FOnPerformSecondPassLayout, UJavascriptGraphEdNode*, Instance);
	
	DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnGetPins, UJavascriptGraphEdNode*, Instance, FJavascriptSlateEdNode, SlateEdNode);
	
	DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnTryCreateConnection, FJavascriptEdGraphPin&, PinA, FJavascriptEdGraphPin&, PinB);

	DECLARE_DYNAMIC_DELEGATE_OneParam(FOnPinConnectionListChanged, FJavascriptEdGraphPin, Pin);

	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FOnGetBoolean_GraphPin, FJavascriptEdGraphPin, Pin);

	DECLARE_DYNAMIC_DELEGATE_RetVal_FourParams(FJavascriptSlateWidget, FOnTakeCustomContentWidget, UJavascriptGraphEdNode*, Instance, FJavascriptSlateWidget, OutUserWidget, FJavascriptSlateWidget, OutLeftNodeBoxWidget, FJavascriptSlateWidget, OutRightNodeBoxWidget);

	DECLARE_DYNAMIC_DELEGATE_RetVal_TwoParams(FSlateColor, FOnGetPinColor, bool, bHovered, FJavascriptEdGraphPin, Pin);

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGetPinColor OnGetPinColor;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGetBoolean_GraphPin OnGetDefaultValueVisibility;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGetSlateBrushName OnGetSlateBrushName;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnPinConnectionListChanged OnPinConnectionListChanged;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnTryCreateConnection OnTryCreateConnection;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGetPins OnMouseEnter;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGetPins OnMouseLeave;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnPerformSecondPassLayout OnPerformSecondPassLayout;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGetBooleanWidget OnRequiresSecondPassLayout;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGetBooleanWidget OnSkipMoveTo;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnTakeCustomContentWidget OnTakeCustomContentWidget;
	
	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnCreatePin OnGetValueWidget;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnCreatePin OnGetActualPinWidget;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnCreatePin OnGetPinStatusIndicator;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGetBooleanWidget OnDisableMakePins;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGetBoolean_GraphPin OnUsingDefaultPin;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnGetBoolean OnUsingNodeWidgetMap;
	
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
	FOnTakeWidget OnTakeTitleWidget;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnTakeWidget OnTakeErrorReportingWidget;

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
	FOnEdNodeAction OnNodeConnectionListChanged;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnCreateAutomaticConversionNodeAndConnections OnCreateAutomaticConversionNodeAndConnections;
	
	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnDetermineLinkGeometry OnDetermineLinkGeometry;

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
	virtual void BreakNodeLinks(UEdGraphNode& TargetNode) const override;
	virtual void BreakPinLinks(UEdGraphPin& TargetPin, bool bSendsNodeNotifcation) const override;
	virtual void BreakSinglePinLink(UEdGraphPin* SourcePin, UEdGraphPin* TargetPin) override;

	virtual bool TryCreateConnection(UEdGraphPin* A, UEdGraphPin* B) const override;
	virtual bool CreateAutomaticConversionNodeAndConnections(UEdGraphPin* PinA, UEdGraphPin* PinB) const override;
	//~ End EdGraphSchema Interface
};

