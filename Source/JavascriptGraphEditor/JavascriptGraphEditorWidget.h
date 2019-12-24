#pragma once

#include "Components/Widget.h"
#include "JavascriptGraphEditorLibrary.h"
#include "JavascriptGraphEditorWidget.generated.h"

class UJavascriptGraphEdGraph;

USTRUCT()
struct FJavascriptGraphAppearanceInfo
{
	GENERATED_BODY()
			
	/** Image to draw in corner of graph */
	UPROPERTY()
	FSlateBrush CornerImage;
	/** Text to write in corner of graph */
	UPROPERTY()
	FText CornerText;
	/** If set, will be used as override for PIE notify text */
	UPROPERTY()
	FText PIENotifyText;
	/** If set, will be used as override for read only text */
	UPROPERTY()
	FText ReadOnlyText;
	/** Text to display if the graph is empty (to guide the user on what to do) */
	UPROPERTY()
	FText InstructionText;
	/** Allows graphs to nicely fade instruction text (or completely hide it). */
	/*UPROPERTY()
	TAttribute<float> InstructionFade;*/

	operator FGraphAppearanceInfo ()
	{
		FGraphAppearanceInfo Out;
		Out.CornerImage = &CornerImage;
		Out.CornerText = CornerText;
		Out.PIENotifyText = PIENotifyText;
		Out.ReadOnlyText = ReadOnlyText;
		Out.InstructionText = InstructionText;
		return Out;
	}
};

// UEnum for SGraphEditor::EPinVisibility
UENUM()
enum class EPinVisibility : uint8
{
	Pin_Show,
	Pin_HideNoConnection,
	Pin_HideNoConnectionNoDefault
};

/**
*
*/
UCLASS()
class JAVASCRIPTGRAPHEDITOR_API UJavascriptGraphEditorWidget : public UWidget
{
	GENERATED_UCLASS_BODY()

	/** Delegate for constructing a UWidget based on a UObject */
	DECLARE_DYNAMIC_DELEGATE_OneParam(FSingleNode, UEdGraphNode*, Node);

	/** Delegate for constructing a UWidget based on a UObject */
	DECLARE_DYNAMIC_DELEGATE_OneParam(FSetNodes, TArray<class UObject*>, Set);

	DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnDropActor, const TArray< AActor* >&, Actors, class UEdGraph*, Graph, const FVector2D&, Point);

	DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnDisallowedPinConnection, FJavascriptEdGraphPin, A, FJavascriptEdGraphPin, B);

	UPROPERTY(EditAnywhere, Category = Content)
	UJavascriptGraphEdGraph* EdGraph;

	UFUNCTION(BlueprintCallable, Category = Content)
	void SetGraph(UJavascriptGraphEdGraph* InEdGraph);

	UFUNCTION(BlueprintCallable, Category = Content)
	static UJavascriptGraphEdGraph* NewGraph(UObject* ParentScope);

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FSingleNode OnNodeDoubleClicked;
	
	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnDropActor OnDropActor;
	
	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FOnDisallowedPinConnection OnDisallowedPinConnection;

	UPROPERTY(EditAnywhere, Category = Events, meta = (IsBindableEvent = "True"))
	FSetNodes OnSelectedNodesChanged;

	UPROPERTY()
	FJavascriptGraphAppearanceInfo AppearanceInfo;

	UPROPERTY()
	FJavascriptUICommandList CommandList;

	UFUNCTION(BlueprintCallable, Category = Content)
	void ClearSelectionSet();

	UFUNCTION(BlueprintCallable, Category = Content)
	TArray<UObject*> GetSelectedNodes();

	UFUNCTION(BlueprintCallable, Category = Content)
	void SetNodeSelection(UEdGraphNode* Node, bool bSelect);

	UFUNCTION(BlueprintCallable, Category = Content)
	void SelectAllNodes();		

	UFUNCTION(BlueprintCallable, Category = Content)
	void SetViewLocation(const FVector2D& Location, float ZoomAmount);

	UFUNCTION(BlueprintCallable, Category = Content)
	void GetViewLocation(FVector2D& OutLocation, float& OutZoomAmount);

	UFUNCTION(BlueprintCallable, Category = Content)
	void JumpToNode(UEdGraphNode* JumpToMe, bool bRequestRename = false, bool bSelectNode = true);

	UFUNCTION(BlueprintCallable, Category = Content)
	void JumpToPin(FJavascriptEdGraphPin JumpToMe);

	UFUNCTION(BlueprintCallable, Category = Content)
	void SetPinVisibility(EPinVisibility InVisibility);

	UFUNCTION(BlueprintCallable, Category = Content)
	FVector2D GetPasteLocation();

	UFUNCTION(BlueprintCallable, Category = Content)
	void NotifyGraphChanged();

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface

	void HandleOnSelectedNodesChanged(const FGraphPanelSelectionSet& Selection);
	void HandleOnNodeDoubleClicked(UEdGraphNode* Node);
	void HandleDropActors(const TArray< TWeakObjectPtr<class AActor> >& Actors, class UEdGraph* Graph, const FVector2D& Point);
	void HandleDisallowedPinConnection(const UEdGraphPin*, const UEdGraphPin*);
};
