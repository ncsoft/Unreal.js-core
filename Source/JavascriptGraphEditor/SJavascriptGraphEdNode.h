#pragma once

#include "SGraphNode.h"
#include "SGraphPin.h"
#include "GraphEditorDragDropAction.h"

class UJavascriptGraphEdNode;

class FDragJavascriptGraphNode : public FGraphEditorDragDropAction
{
public:
	DRAG_DROP_OPERATOR_TYPE(FDragJavascriptGraphNode, FGraphEditorDragDropAction)

	static TSharedRef<FDragJavascriptGraphNode> New(const TSharedRef<SGraphNode>& InDraggedNode);

	virtual void HoverTargetChanged() override;

	UJavascriptGraphEdNode* GetDropTargetNode() const;

protected:
	TArray< TSharedRef<SGraphNode> > DraggedNodes;
};

class SJavascriptGraphEdNode : public SGraphNode
{
	enum EResizableWindowZone
	{
		CRWZ_NotInWindow = 0,
		CRWZ_InWindow = 1,
		CRWZ_RightBorder = 2,
		CRWZ_BottomBorder = 3,
		CRWZ_BottomRightBorder = 4,
		CRWZ_LeftBorder = 5,
		CRWZ_TopBorder = 6,
		CRWZ_TopLeftBorder = 7,
		CRWZ_TopRightBorder = 8,
		CRWZ_BottomLeftBorder = 9,
		CRWZ_TitleBar = 10,
	};

public:
	SLATE_BEGIN_ARGS(SJavascriptGraphEdNode) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UJavascriptGraphEdNode* InNode);

	//~ Begin SGraphNode Interface
	virtual void UpdateGraphNode() override;
	virtual void CreatePinWidgets() override;
	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	virtual void MoveTo(const FVector2D& NewPosition, FNodeSet& NodeFilter, bool bMarkDirty = true) override;
	virtual bool RequiresSecondPassLayout() const override;
	virtual void PerformSecondPassLayout(const TMap< UObject*, TSharedRef<SNode> >& NodeToWidgetLookup) const override;

	virtual FString GetNodeComment() const override;
	virtual void OnCommentTextCommitted(const FText& NewComment, ETextCommit::Type CommitInfo);
	virtual int32 GetSortDepth() const override;
	virtual void EndUserInteraction() const override;

	virtual void CreateOutputSideAddButton(TSharedPtr<SVerticalBox> OutputBox) override;
	virtual FReply OnAddPin() override;
	//~ End SGraphNode Interface

	//~ Begin SPanel Interface
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	virtual FVector2D GetDesiredSizeForMarquee() const override;
	//~ End SPanel Interface

	//~ Begin SWidget Interface
	virtual void OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual void OnDragLeave(const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
	virtual void OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual void OnMouseLeave(const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;
	//~ End SWidget Interface

	virtual FText GetDescription() const;
	virtual EVisibility GetDescriptionVisibility() const;

	virtual const FSlateBrush* GetNameIcon() const;

	/** Find the current window zone the mouse is in */
	virtual SJavascriptGraphEdNode::EResizableWindowZone FindMouseZone(const FVector2D& LocalMouseCoordinates) const;

	bool InSelectionArea() const;

public:
	void PositionBetweenTwoNodesWithOffset(const FGeometry& StartGeom, const FGeometry& EndGeom, int32 NodeIndex, int32 MaxNodes) const;

private:
	TSharedPtr<SWidget> GetTitleAreaWidget();
	TSharedPtr<SWidget>	GetUserWidget();
	TSharedPtr<SWidget> GetContentWidget();
	TSharedPtr<SWidget> ErrorReportingWidget();
	void UpdatePinSlate();

public:
	/** The non snapped size of the node for fluid resizing */
	FVector2D DragSize;
	/** The desired size of the node set during a drag */
	FVector2D UserSize;
	/** If true the user is actively dragging the node */
	bool bUserIsDragging;
	/** The current window zone the mouse is in */
	EResizableWindowZone MouseZone;
};
