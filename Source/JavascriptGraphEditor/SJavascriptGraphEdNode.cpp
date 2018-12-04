#include "SJavascriptGraphEdNode.h"
#include "SJavascriptGraphEdNodePin.h"

#include "JavascriptGraphEdNode.h"
#include "JavascriptGraphAssetGraphSchema.h"
#include "SlateOptMacros.h"
#include "Widgets/Layout/SBox.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SWindow.h"
#include "SCommentBubble.h"

void SJavascriptGraphEdNode::Construct(const FArguments& InArgs, UJavascriptGraphEdNode* InNode)
{
	GraphNode = InNode;
	// SetCursor(EMouseCursor::CardinalCross);
	UpdateGraphNode();
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SJavascriptGraphEdNode::UpdateGraphNode()
{
	InputPins.Empty();
	OutputPins.Empty();

	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();
	LeftNodeBox.Reset();	
	
	TSharedPtr<SWidget> UserWidget = SNew(SBox);
	TSharedPtr<SWidget> TitleAreaWidget = SNew(SBox);
	TSharedPtr<SWidget> ErrorReportingWidget = SNew(SBox);


	TSharedPtr<SWidget> LeftNodeBoxWidget = SAssignNew(LeftNodeBox, SVerticalBox);
	TSharedPtr<SWidget> RightNodeBoxWidget = SAssignNew(RightNodeBox, SVerticalBox);

	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);

	TSharedPtr<SWidget> AltLeftNodeBoxWidget = Schema->OnUsingAlternativeInputPinBox.IsBound()
		? TSharedPtr<SWidget>(SAssignNew(AltLeftNodeBox, SVerticalBox))
		: nullptr;
	TSharedPtr<SWidget> AltRightNodeBoxWidget = Schema->OnUsingAlternativeOutputPinBox.IsBound()
		? TSharedPtr<SWidget>(SAssignNew(AltRightNodeBox, SVerticalBox))
		: nullptr;

	GraphEdNode->BackgroundColor = FLinearColor::Black;

	if (GraphEdNode)
	{
		if (Schema->OnTakeWidget.IsBound())
		{
			auto Widget = Schema->OnTakeWidget.Execute(GraphEdNode).Widget;
			if (Widget.IsValid())
			{
				UserWidget = Widget;
			}
		}

		if (Schema->OnTakeTitleWidget.IsBound())
		{
			auto Widget = Schema->OnTakeTitleWidget.Execute(GraphEdNode).Widget;
			if (Widget.IsValid())
			{
				TitleAreaWidget = Widget;
			}
		}

		if (Schema->OnTakeErrorReportingWidget.IsBound())
		{
			auto Widget = Schema->OnTakeErrorReportingWidget.Execute(GraphEdNode).Widget;
			if (Widget.IsValid())
			{
				ErrorReportingWidget = Widget;
			}
		}
	}

	// default style
	const FMargin NodePadding = FMargin(2.0f);	
	TSharedPtr<SWidget> ContentWidget = SNew(SHorizontalBox)
		// INPUT PIN AREA
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.MinDesiredHeight(NodePadding.Top)
			[
				LeftNodeBoxWidget.ToSharedRef()
			]
		]
		// STATE NAME AREA
		+ SHorizontalBox::Slot()
			.Padding(FMargin(NodePadding.Left, 0.0f, NodePadding.Right, 0.0f))
			[
				UserWidget.ToSharedRef()
			]
		// OUTPUT PIN AREA
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SBox)
			.MinDesiredHeight(NodePadding.Bottom)
			[
				RightNodeBoxWidget.ToSharedRef()
			]
		];
	

	if (GraphEdNode)
	{
		if (Schema->OnTakeCustomContentWidget.IsBound())
		{
			FJavascriptSlateWidget OutUserWidget;
			FJavascriptSlateWidget OutLeftNodeBoxWidget;
			FJavascriptSlateWidget OutRightNodeBoxWidget;
			FJavascriptSlateWidget OutAltLeftNodeBoxWidget;
			FJavascriptSlateWidget OutAltRightNodeBoxWidget;

			OutUserWidget.Widget = UserWidget;
			OutLeftNodeBoxWidget.Widget = LeftNodeBoxWidget;
			OutRightNodeBoxWidget.Widget = RightNodeBoxWidget;
			OutAltLeftNodeBoxWidget.Widget = AltLeftNodeBoxWidget;
			OutAltRightNodeBoxWidget.Widget = AltRightNodeBoxWidget;

			auto CustomContentWidget = Schema->OnTakeCustomContentWidget.Execute(GraphEdNode, OutUserWidget, OutLeftNodeBoxWidget, OutRightNodeBoxWidget, OutAltLeftNodeBoxWidget, OutAltRightNodeBoxWidget).Widget;
			if (CustomContentWidget.IsValid())
			{
				ContentWidget = CustomContentWidget;
			}
		}
	}

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);
	this->GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("Graph.StateNode.Body"))
			.Padding(0.0f)
			.BorderBackgroundColor(GraphEdNode->BackgroundColor)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()		
					.AutoHeight()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Top)
					[
						TitleAreaWidget.ToSharedRef()
					]
				+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						ContentWidget.ToSharedRef()
					]
				+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(1.0f)
					[
						ErrorReportingWidget.ToSharedRef()
					]
			]
		];

	if (GraphEdNode && Schema->OnGetNodeComment.IsBound())
	{
		FText Text = Schema->OnGetNodeComment.Execute(GraphEdNode);
		if (Text.IsEmpty() == false)
		{
			GetNodeObj()->bCommentBubbleVisible = true;
		}
	}

	TSharedPtr<SCommentBubble> CommentBubble;
	const FSlateColor CommentColor = GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;

	SAssignNew(CommentBubble, SCommentBubble)
		.GraphNode(GraphNode)
		.Text(this, &SJavascriptGraphEdNode::GetNodeComment)
		.OnTextCommitted(this, &SJavascriptGraphEdNode::OnCommentTextCommitted)
		.ColorAndOpacity(CommentColor)
		.AllowPinning(true)
		.EnableTitleBarBubble(true)
		.EnableBubbleCtrls(true)
		.GraphLOD(this, &SGraphNode::GetCurrentLOD)
		.IsGraphNodeHovered(this, &SGraphNode::IsHovered);

	GetOrAddSlot(ENodeZone::TopCenter)
		.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
		.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
		.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
		.VAlign(VAlign_Top)
		[
			CommentBubble.ToSharedRef()
		];
	
	CreatePinWidgets();
	// CreateInputSideAddButton(LeftNodeBox);
	CreateOutputSideAddButton(RightNodeBox);
}

void SJavascriptGraphEdNode::CreatePinWidgets()
{
	UJavascriptGraphEdNode* GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);

	// @todo:
	GraphEdNode->SlateGraphNode = this;
	

	for (int32 PinIdx = 0; PinIdx < GraphEdNode->Pins.Num(); PinIdx++)
	{
		UEdGraphPin* MyPin = GraphEdNode->Pins[PinIdx];
		if (!MyPin->bHidden)
		{
			TSharedPtr<SGraphPin> NewPin;

			/*auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
			if (Schema->OnCreatePin.IsBound())
			{
				NewPin = StaticCastSharedPtr<SGraphPin>(Schema->OnCreatePin.Execute(FJavascriptEdGraphPin{ MyPin }).Widget);
			}*/
			
			if (!NewPin.IsValid())
			{
				NewPin = SNew(SJavascriptGraphPin, MyPin);
			}

			AddPin(NewPin.ToSharedRef());
		}
	}
}

void SJavascriptGraphEdNode::AddPin(const TSharedRef<SGraphPin>& PinToAdd)
{
	PinToAdd->SetOwner(SharedThis(this));

	const UEdGraphPin* PinObj = PinToAdd->GetPinObj();
	const bool bAdvancedParameter = PinObj && PinObj->bAdvancedView;
	if (bAdvancedParameter)
	{
		PinToAdd->SetVisibility( TAttribute<EVisibility>(PinToAdd, &SGraphPin::IsPinVisibleAsAdvanced) );
	}

	FJavascriptEdGraphPin JavascriptGraphPin = FJavascriptEdGraphPin(const_cast<UEdGraphPin*>(PinObj));

	bool bDisable = false;
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	if (Schema->OnUsingDefaultPin.IsBound())
	{
		bDisable = Schema->OnUsingDefaultPin.Execute(JavascriptGraphPin);
	}

	if (PinToAdd->GetDirection() == EEdGraphPinDirection::EGPD_Input)
	{
		TSharedPtr<SVerticalBox> NodeBox = LeftNodeBox;

		if (Schema->OnUsingAlternativeInputPinBox.IsBound() &&
			Schema->OnUsingAlternativeInputPinBox.Execute(JavascriptGraphPin) &&
			AltLeftNodeBox.IsValid())
		{
			NodeBox = AltLeftNodeBox;
		}

		if (bDisable)
		{
			NodeBox->AddSlot()
			[
				PinToAdd
			];
		}
		else
		{
			NodeBox->AddSlot()
				.AutoHeight()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.Padding(Settings->GetInputPinPadding())
				[
					PinToAdd
				];

		}
		InputPins.Add(PinToAdd);
	}
	else // Direction == EEdGraphPinDirection::EGPD_Output
	{
		TSharedPtr<SVerticalBox> NodeBox = RightNodeBox;

		if (Schema->OnUsingAlternativeOutputPinBox.IsBound() &&
			Schema->OnUsingAlternativeOutputPinBox.Execute(JavascriptGraphPin) &&
			AltRightNodeBox.IsValid())
		{
			NodeBox = AltRightNodeBox;
		}

		if (bDisable) 
		{
			NodeBox->AddSlot()
				[
					PinToAdd
				];
		}
		else
		{
			NodeBox->AddSlot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				.VAlign(VAlign_Center)
				.Padding(Settings->GetOutputPinPadding())
				[
					PinToAdd
				];

		}
		OutputPins.Add(PinToAdd);
	}
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

EVisibility SJavascriptGraphEdNode::GetDragOverMarkerVisibility() const
{
	return EVisibility::Visible;
}

FText SJavascriptGraphEdNode::GetDescription() const
{
	UJavascriptGraphEdNode* MyNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	return MyNode ? MyNode->GetDescription() : FText::GetEmpty();
}

EVisibility SJavascriptGraphEdNode::GetDescriptionVisibility() const
{
	return EVisibility::Visible;
}

const FSlateBrush* SJavascriptGraphEdNode::GetNameIcon() const
{
	return FEditorStyle::GetBrush(TEXT("BTEditor.Graph.BTNode.Icon"));
}

SJavascriptGraphEdNode::EResizableWindowZone SJavascriptGraphEdNode::FindMouseZone(const FVector2D & LocalMouseCoordinates) const
{
	EResizableWindowZone OutMouseZone = CRWZ_NotInWindow;
	const FSlateRect HitResultBorderSize = FSlateRect(10, 10, 10, 10);
	const FVector2D NodeSize = GetDesiredSize();

	// Test for hit in location of 'grab' zone
	if (LocalMouseCoordinates.Y > (NodeSize.Y - HitResultBorderSize.Bottom))
	{
		OutMouseZone = CRWZ_BottomBorder;
	}
	else if (LocalMouseCoordinates.Y <= (HitResultBorderSize.Top))
	{
		OutMouseZone = CRWZ_TopBorder;
	}
	else if (LocalMouseCoordinates.Y <= 12.f)
	{
		OutMouseZone = CRWZ_TitleBar;
	}

	if (LocalMouseCoordinates.X > (NodeSize.X - HitResultBorderSize.Right))
	{
		if (OutMouseZone == CRWZ_BottomBorder)
		{
			OutMouseZone = CRWZ_BottomRightBorder;
		}
		else if (OutMouseZone == CRWZ_TopBorder)
		{
			OutMouseZone = CRWZ_TopRightBorder;
		}
		else
		{
			OutMouseZone = CRWZ_RightBorder;
		}
	}
	else if (LocalMouseCoordinates.X <= HitResultBorderSize.Left)
	{
		if (OutMouseZone == CRWZ_TopBorder)
		{
			OutMouseZone = CRWZ_TopLeftBorder;
		}
		else if (OutMouseZone == CRWZ_BottomBorder)
		{
			OutMouseZone = CRWZ_BottomLeftBorder;
		}
		else
		{
			OutMouseZone = CRWZ_LeftBorder;
		}
	}

	// Test for hit on rest of frame
	if (OutMouseZone == CRWZ_NotInWindow)
	{
		if (LocalMouseCoordinates.Y > HitResultBorderSize.Top)
		{
			OutMouseZone = CRWZ_InWindow;
		}
		else if (LocalMouseCoordinates.X > HitResultBorderSize.Left)
		{
			OutMouseZone = CRWZ_InWindow;
		}
	}
	return OutMouseZone;
}

bool SJavascriptGraphEdNode::InSelectionArea() const
{
	return ((MouseZone == CRWZ_RightBorder) || (MouseZone == CRWZ_BottomBorder) || (MouseZone == CRWZ_BottomRightBorder) ||
		(MouseZone == CRWZ_LeftBorder) || (MouseZone == CRWZ_TopBorder) || (MouseZone == CRWZ_TopLeftBorder) ||
		(MouseZone == CRWZ_TopRightBorder) || (MouseZone == CRWZ_BottomLeftBorder));
}

void SJavascriptGraphEdNode::MoveTo(const FVector2D& NewPosition, FNodeSet& NodeFilter)
{
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	
	if (GraphEdNode && Schema->OnMoveTo.IsBound())
	{
		bool bSucc = Schema->OnMoveTo.Execute(GraphEdNode, NewPosition);
		if (!bSucc)
		{
			return;
		}
	}
	
	SGraphNode::MoveTo(NewPosition, NodeFilter);
}

bool SJavascriptGraphEdNode::RequiresSecondPassLayout() const
{
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	if (GraphEdNode && Schema->OnRequiresSecondPassLayout.IsBound())
	{
		return Schema->OnRequiresSecondPassLayout.Execute(GraphEdNode);
	}

	return SGraphNode::RequiresSecondPassLayout();
}

void SJavascriptGraphEdNode::PerformSecondPassLayout(const TMap< UObject*, TSharedRef<SNode> >& NodeToWidgetLookup) const
{
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	if (GraphEdNode && Schema->OnPerformSecondPassLayout.IsBound())
	{
		FGeometry StartGeom;
		FGeometry EndGeom;

		FJavascriptPerformSecondPassLayoutContainer Container = Schema->OnPerformSecondPassLayout.Execute(GraphEdNode);
		UEdGraphNode* PrevNode = Container.PrevNode;
		UEdGraphNode* NextNode = Container.NextNode;
		if (PrevNode && NextNode)
		{
			const TSharedRef<SNode>* pPrevNodeWidget = NodeToWidgetLookup.Find(PrevNode);
			const TSharedRef<SNode>* pNextNodeWidget = NodeToWidgetLookup.Find(NextNode);
			if (pPrevNodeWidget && pNextNodeWidget)
			{
				const TSharedRef<SNode>& PrevNodeWidget = *pPrevNodeWidget;
				const TSharedRef<SNode>& NextNodeWidget = *pNextNodeWidget;

				StartGeom = FGeometry(FVector2D(PrevNode->NodePosX, PrevNode->NodePosY), FVector2D::ZeroVector, PrevNodeWidget->GetDesiredSize(), 1.0f);
				EndGeom = FGeometry(FVector2D(NextNode->NodePosX, NextNode->NodePosY), FVector2D::ZeroVector, NextNodeWidget->GetDesiredSize(), 1.0f);

				PositionBetweenTwoNodesWithOffset(StartGeom, EndGeom, Container.NodeIndex, Container.MaxNodes);
			}
		}
	}
}

void SJavascriptGraphEdNode::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	if (Schema->OnMouseEnter.IsBound())
	{
		Schema->OnMouseEnter.Execute(GraphEdNode, FJavascriptSlateEdNode{ this });
	}
	
	if (!bUserIsDragging && Schema->OnIsNodeComment.IsBound() && Schema->OnIsNodeComment.Execute(GraphEdNode))
	{
		FVector2D LocalMouseCoordinates = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
		MouseZone = FindMouseZone(LocalMouseCoordinates);
	}

	SNodePanel::SNode::OnMouseEnter(MyGeometry, MouseEvent);
}

void SJavascriptGraphEdNode::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	if (Schema->OnMouseLeave.IsBound())
	{
		Schema->OnMouseLeave.Execute(GraphEdNode, FJavascriptSlateEdNode{ this });
	}

	if (!bUserIsDragging && Schema->OnIsNodeComment.IsBound() && Schema->OnIsNodeComment.Execute(GraphEdNode))
	{
		MouseZone = CRWZ_NotInWindow;
	}

	SNodePanel::SNode::OnMouseLeave(MouseEvent);
}

FReply SJavascriptGraphEdNode::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	if (Schema->OnMouseMove.IsBound())
	{
		FVector2D GraphSpaceCoordinates = NodeCoordToGraphCoord(MouseEvent.GetScreenSpacePosition());
		FVector2D OldGraphSpaceCoordinates = NodeCoordToGraphCoord(MouseEvent.GetLastScreenSpacePosition());
		TSharedPtr<SWindow> OwnerWindow = FSlateApplication::Get().FindWidgetWindow(AsShared());
		FVector2D Delta = (GraphSpaceCoordinates - OldGraphSpaceCoordinates) / (OwnerWindow.IsValid() ? OwnerWindow->GetDPIScaleFactor() : 1.0f);

		Schema->OnMouseMove.Execute(GraphEdNode, Delta, bUserIsDragging, (int32)MouseZone);
	}

	if (!bUserIsDragging && Schema->OnIsNodeComment.IsBound() && Schema->OnIsNodeComment.Execute(GraphEdNode))
	{
		const FVector2D LocalMouseCoordinates = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
		MouseZone = FindMouseZone(LocalMouseCoordinates);
	}

	return SGraphNode::OnMouseMove(MyGeometry, MouseEvent);
}

FReply SJavascriptGraphEdNode::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	
	if (Schema->OnMouseButtonDown.IsBound())
	{
		Schema->OnMouseButtonDown.Execute(GraphEdNode, MyGeometry);
	}

	if (Schema->OnIsNodeComment.IsBound() && Schema->OnIsNodeComment.Execute(GraphEdNode))
	{
		if (InSelectionArea())
		{
			bUserIsDragging = true;
			return FReply::Handled().CaptureMouse(SharedThis(this));
		}
	}

	return FReply::Unhandled();
}

FReply SJavascriptGraphEdNode::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	if (Schema->OnMouseButtonUp.IsBound())
	{
		Schema->OnMouseButtonUp.Execute(GraphEdNode, MyGeometry);
	}

	if (Schema->OnIsNodeComment.IsBound() && Schema->OnIsNodeComment.Execute(GraphEdNode))
	{
		bUserIsDragging = false;
		return FReply::Handled().ReleaseMouseCapture();
	}

	return FReply::Unhandled();
}

FCursorReply SJavascriptGraphEdNode::OnCursorQuery(const FGeometry & MyGeometry, const FPointerEvent & CursorEvent) const
{
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());

	if (Schema->OnIsNodeComment.IsBound() && Schema->OnIsNodeComment.Execute(GraphEdNode))
	{
		if (MouseZone == CRWZ_RightBorder || MouseZone == CRWZ_LeftBorder)
		{
			// right/left of node
			return FCursorReply::Cursor(EMouseCursor::ResizeLeftRight);
		}
		else if (MouseZone == CRWZ_BottomRightBorder || MouseZone == CRWZ_TopLeftBorder)
		{
			// bottom right / top left hand corner
			return FCursorReply::Cursor(EMouseCursor::ResizeSouthEast);
		}
		else if (MouseZone == CRWZ_BottomBorder || MouseZone == CRWZ_TopBorder)
		{
			// bottom / top of node
			return FCursorReply::Cursor(EMouseCursor::ResizeUpDown);
		}
		else if (MouseZone == CRWZ_BottomLeftBorder || MouseZone == CRWZ_TopRightBorder)
		{
			// bottom left / top right hand corner
			return FCursorReply::Cursor(EMouseCursor::ResizeSouthWest);
		}
		else if (MouseZone == CRWZ_TitleBar)
		{
			return FCursorReply::Cursor(EMouseCursor::CardinalCross);
		}
	}
	return FCursorReply::Unhandled();
}

FVector2D SJavascriptGraphEdNode::ComputeDesiredSize(float LayoutScaleMultiplier) const
{
	const FVector2D& InDesiredSize = SNodePanel::SNode::ComputeDesiredSize(LayoutScaleMultiplier);

	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	if (Schema->OnIsNodeComment.IsBound() && Schema->OnIsNodeComment.Execute(GraphEdNode))
	{
		return FVector2D(GraphEdNode->NodeWidth, GraphEdNode->NodeHeight);
	}

	return InDesiredSize;
}

FVector2D SJavascriptGraphEdNode::GetDesiredSizeForMarquee() const
{
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);

	if (GraphEdNode->bTitleSelectionOnly) 
	{
		FVector2D SelectionSize = FVector2D(SGraphNode::GetDesiredSizeForMarquee());
		SelectionSize.Y = GraphEdNode->TitleHeight;

		return SelectionSize;
	}
	
	return SGraphNode::GetDesiredSizeForMarquee();
}

void SJavascriptGraphEdNode::PositionBetweenTwoNodesWithOffset(const FGeometry& StartGeom, const FGeometry& EndGeom, int32 NodeIndex, int32 MaxNodes) const
{
	// Get a reasonable seed point (halfway between the boxes)
	const FVector2D StartCenter = FGeometryHelper::CenterOf(StartGeom);
	const FVector2D EndCenter = FGeometryHelper::CenterOf(EndGeom);
	const FVector2D SeedPoint = (StartCenter + EndCenter) * 0.5f;

	// Find the (approximate) closest points between the two boxes
	const FVector2D StartAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(StartGeom, SeedPoint);
	const FVector2D EndAnchorPoint = FGeometryHelper::FindClosestPointOnGeom(EndGeom, SeedPoint);

	// Position ourselves halfway along the connecting line between the nodes, elevated away perpendicular to the direction of the line
	const float Height = 30.0f;

	const FVector2D DesiredNodeSize = GetDesiredSize();

	FVector2D DeltaPos(EndAnchorPoint - StartAnchorPoint);

	if (DeltaPos.IsNearlyZero())
	{
		DeltaPos = FVector2D(10.0f, 0.0f);
	}

	const FVector2D Normal = FVector2D(DeltaPos.Y, -DeltaPos.X).GetSafeNormal();

	const FVector2D NewCenter = StartAnchorPoint + (0.5f * DeltaPos) + (Height * Normal);

	FVector2D DeltaNormal = DeltaPos.GetSafeNormal();

	// Calculate node offset in the case of multiple transitions between the same two nodes
	// MultiNodeOffset: the offset where 0 is the centre of the transition, -1 is 1 <size of node>
	// towards the PrevStateNode and +1 is 1 <size of node> towards the NextStateNode.

	const float MutliNodeSpace = 0.2f; // Space between multiple transition nodes (in units of <size of node> )
	const float MultiNodeStep = (1.f + MutliNodeSpace); //Step between node centres (Size of node + size of node spacer)

	const float MultiNodeStart = -((MaxNodes - 1) * MultiNodeStep) / 2.f;
	const float MultiNodeOffset = MultiNodeStart + (NodeIndex * MultiNodeStep);

	// Now we need to adjust the new center by the node size, zoom factor and multi node offset
	const FVector2D NewCorner = NewCenter - (0.5f * DesiredNodeSize) + (DeltaNormal * MultiNodeOffset * DesiredNodeSize.Size());

	GraphNode->NodePosX = NewCorner.X;
	GraphNode->NodePosY = NewCorner.Y;
}


FString SJavascriptGraphEdNode::GetNodeComment() const
{
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	if (GraphEdNode && Schema->OnGetNodeComment.IsBound())
	{
		FText Text = Schema->OnGetNodeComment.Execute(GraphEdNode);
		GetNodeObj()->NodeComment = Text.ToString();
	}

	return GetNodeObj()->NodeComment;
}

void SJavascriptGraphEdNode::OnCommentTextCommitted(const FText& NewComment, ETextCommit::Type CommitInfo)
{
	GetNodeObj()->OnUpdateCommentText(NewComment.ToString());

	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	if (GraphEdNode && Schema->OnSetNodeComment.IsBound())
	{
		Schema->OnSetNodeComment.Execute(GraphEdNode, NewComment);
	}
}

int32 SJavascriptGraphEdNode::GetSortDepth() const
{
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	if (GraphEdNode && Schema->OnIsNodeComment.IsBound() && Schema->OnIsNodeComment.Execute(GraphEdNode))
	{
		return -1;
	}

	return 0;
}

void SJavascriptGraphEdNode::EndUserInteraction() const
{
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	if (GraphEdNode && Schema->OnEndUserInteraction.IsBound())
	{
		Schema->OnEndUserInteraction.Execute(GraphEdNode);
	}
}

void SJavascriptGraphEdNode::CreateOutputSideAddButton(TSharedPtr<SVerticalBox> OutputBox)
{
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	if (GraphEdNode && Schema->OnCreateOutputSideAddButton.IsBound())
	{
		if (Schema->OnCreateOutputSideAddButton.Execute(GraphEdNode))
		{
			TSharedRef<SWidget> AddPinButton = AddPinButtonContent(
				NSLOCTEXT("SequencerNode", "SequencerNodeAddPinButton", "Add pin"),
				NSLOCTEXT("SequencerNode", "SequencerNodeAddPinButton_ToolTip", "Add new pin"));

			FMargin AddPinPadding = Settings->GetOutputPinPadding();
			AddPinPadding.Top += 6.0f;

			OutputBox->AddSlot()
				.AutoHeight()
				.VAlign(VAlign_Center)
				.Padding(AddPinPadding)
				[
					AddPinButton
				];
		}
	}
}

FReply SJavascriptGraphEdNode::OnAddPin()
{
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	if (GraphEdNode && Schema->OnAddPinByAddButton.IsBound())
	{
		Schema->OnAddPinByAddButton.Execute(GraphEdNode);
		UpdateGraphNode();
		GraphNode->GetGraph()->NotifyGraphChanged();
	}

	return FReply::Handled();
}
