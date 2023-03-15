#include "SJavascriptGraphEdNode.h"
#include "SJavascriptGraphEdNodePin.h"

#include "Blueprint/UserWidget.h"
#include "Components/VerticalBox.h"
#include "Framework/Application/SlateApplication.h"
#include "JavascriptGraphAssetGraphSchema.h"
#include "JavascriptGraphEdNode.h"
#include "JavascriptUMG/JavascriptUMGLibrary.h"
#include "SCommentBubble.h"
#include "SlateOptMacros.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SWindow.h"
#include "Widgets/SInvalidationPanel.h"

TSharedRef<FDragJavascriptGraphNode> FDragJavascriptGraphNode::New(const TSharedRef<SGraphNode>& InDraggedNode)
{
	TSharedRef<FDragJavascriptGraphNode> Operation = MakeShareable(new FDragJavascriptGraphNode);
	Operation->DraggedNodes.Add(InDraggedNode);
	Operation->Construct();
	return Operation;
}

void FDragJavascriptGraphNode::HoverTargetChanged()
{
	TArray<FPinConnectionResponse> UniqueMessages;
	UEdGraphNode* TargetNodeObj = GetHoveredNode();

	if (TargetNodeObj != nullptr)
	{
		TSharedRef<SVerticalBox> FeedbackBox = SNew(SVerticalBox);
		const FSlateBrush* StatusSymbol = nullptr;
		const FText Message = FText::FromString(TEXT("TEST"));

		StatusSymbol = FAppStyle::Get().GetBrush(TEXT("Graph.ConnectorFeedback.OK"));
		// StatusSymbol = FAppStyle::Get().GetBrush(TEXT("Graph.ConnectorFeedback.Error"));
		FeedbackBox->AddSlot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(3.0f)
				[
					SNew(SImage).Image(StatusSymbol)
				]
			];

		FeedbackBox->AddSlot()
			.AutoHeight()
			[
				DraggedNodes[0]
			];

		SetFeedbackMessage(FeedbackBox);
	}
	else
	{
		SetSimpleFeedbackMessage(
			FAppStyle::Get().GetBrush(TEXT("Graph.ConnectorFeedback.Error")),
			FLinearColor::White,
			NSLOCTEXT("GraphEditor.Feedback", "DragNode", "This node cannot be placed here."));
	}
}

UJavascriptGraphEdNode * FDragJavascriptGraphNode::GetDropTargetNode() const
{
	return Cast<UJavascriptGraphEdNode>(GetHoveredNode());
}

void SJavascriptGraphEdNode::Construct(const FArguments& InArgs, UJavascriptGraphEdNode* InNode)
{
	GraphNode = InNode;
	// SetCursor(EMouseCursor::CardinalCross);
	UpdateGraphNode();

	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	if (GraphEdNode)
	{
		// TSharedPtr<FJavascriptFunction> Copy(new FJavascriptFunction);
		// *(Copy.Get()) = GraphEdNode->WidgetFinalized;
		// Copy->Execute();
		GraphEdNode->WidgetFinalized.Execute();
	}
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SJavascriptGraphEdNode::UpdateGraphNode()
{
	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	InputPins.Empty();
	OutputPins.Empty();
	RightNodeBox.Reset();
	LeftNodeBox.Reset();
	InvalidationPanel.Reset();
	LastKnownLayoutScaleMultiplier = 0.0f;
	
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	
	GraphEdNode->BackgroundColor = FLinearColor::Black;

	this->ContentScale.Bind(this, &SGraphNode::GetContentScale);

	this->GetOrAddSlot(ENodeZone::Center)
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			[
				SAssignNew(InvalidationPanel, SInvalidationPanel)
				[
					SNew(SOverlay)
					+SOverlay::Slot()
					.Padding(Settings->GetNonPinNodeBodyPadding())
					[
						SNew(SImage)
						.Image(FAppStyle::Get().GetBrush("Graph.Node.Body"))
						.ColorAndOpacity(this, &SGraphNode::GetNodeBodyColor)
					]
					+SOverlay::Slot()
					[
						SNew(SVerticalBox)
						+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Top)
							[
								GetTitleAreaWidget().ToSharedRef()
							]
						+ SVerticalBox::Slot()
							.AutoHeight()
							.HAlign(HAlign_Fill)
							.VAlign(VAlign_Fill)
							[
								GetContentWidget().ToSharedRef()
							]
						+ SVerticalBox::Slot()
							.AutoHeight()
							.Padding(1.0f)
							[
								ErrorReportingWidget().ToSharedRef()
							]
					]
				]
			]			
		];

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

	this->GetOrAddSlot(ENodeZone::TopCenter)
		.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
		.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
		.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
		.VAlign(VAlign_Top)
		[
			CommentBubble.ToSharedRef()
		];

	CreatePinWidgets();
	CreateOutputSideAddButton(RightNodeBox);
	CreateAdvancedViewArrow(LeftNodeBox);
	SetRenderOpacity(GraphEdNode->RenderOpacity);
	GraphEdNode->SlateGraphNode = SharedThis(this);
}

TSharedPtr<SWidget> SJavascriptGraphEdNode::GetTitleAreaWidget()
{
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	if (Schema->OnTakeTitleAreaWidget.IsBound())
	{
		auto Widget = Schema->OnTakeTitleAreaWidget.Execute(GraphEdNode).Widget;
		if (Widget.IsValid())
		{
			return Widget;
		}
	}
	return SNew(SBox);
}

TSharedPtr<SWidget> SJavascriptGraphEdNode::GetUserWidget()
{
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	if (Schema->OnTakeUserWidget.IsBound())
	{
		auto Widget = Schema->OnTakeUserWidget.Execute(GraphEdNode).Widget;
		if (Widget.IsValid())
		{
			return Widget;
		}
	}
	return SNew(SBox);
}

TSharedPtr<SWidget> SJavascriptGraphEdNode::GetContentWidget()
{
	TSharedPtr<SWidget> LeftNodeBoxWidget = SAssignNew(LeftNodeBox, SVerticalBox);
	TSharedPtr<SWidget> RightNodeBoxWidget = SAssignNew(RightNodeBox, SVerticalBox);

	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	if (GraphEdNode)
	{
		if (Schema->OnTakeContentWidget.IsBound())
		{
			FJavascriptSlateWidget OutLeftNodeBoxWidget;
			FJavascriptSlateWidget OutRightNodeBoxWidget;
			OutLeftNodeBoxWidget.Widget = LeftNodeBoxWidget;
			OutRightNodeBoxWidget.Widget = RightNodeBoxWidget;

			auto ContentWidget = Schema->OnTakeContentWidget.Execute(GraphEdNode, OutLeftNodeBoxWidget, OutRightNodeBoxWidget).Widget;
			if (ContentWidget.IsValid())
			{
				return ContentWidget;
			}
		}
	}

	// default style
	const FMargin NodePadding = FMargin(2.0f);
	return SNew(SHorizontalBox)
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
			GetUserWidget().ToSharedRef()
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
}

TSharedPtr<SWidget> SJavascriptGraphEdNode::ErrorReportingWidget()
{
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	if (Schema->OnTakeErrorReportingWidget.IsBound())
	{
		auto Widget = Schema->OnTakeErrorReportingWidget.Execute(GraphEdNode).Widget;
		if (Widget.IsValid())
		{
			return Widget;
		}
	}
	return SNew(SBox);
}

void SJavascriptGraphEdNode::CreatePinWidgets()
{
	UJavascriptGraphEdNode* GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);

	// Custom node currently does not support pin functionality.
	// Actually, only creating and adding pin widgets to pin box is available.
	// But connecting, drawing connection and other is not.
	// This limitation comes from the fact that we didn't add this node widget to a SGraphPanel
	// like any other implementation of sub-nodes, e.g. Decorator/Service of SGraphNode_BehaviorTree.
	if (GraphEdNode->IsCustomNode)
	{
		return;
	}

	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphEdNode->GetSchema());

	for (int32 PinIdx = 0; PinIdx < GraphEdNode->Pins.Num(); PinIdx++)
	{
		UEdGraphPin* MyPin = GraphEdNode->Pins[PinIdx];
		if (!MyPin->bHidden)
		{
			TSharedPtr<SGraphPin> NewPin = nullptr;

			if (Schema->OnCreatePin.IsBound())
			{
				NewPin = StaticCastSharedPtr<SGraphPin>(Schema->OnCreatePin.Execute(FJavascriptEdGraphPin{ MyPin }).Widget);
			}
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

	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	if (Schema->OnEnablePin.IsBound())
	{
		bool bEnablePin = Schema->OnEnablePin.Execute(JavascriptGraphPin);
		PinToAdd->SetEnabled(bEnablePin);
	}

	bool bDisable = false;
	if (Schema->OnUsingDefaultPin.IsBound())
	{
		bDisable = Schema->OnUsingDefaultPin.Execute(JavascriptGraphPin);
	}

	TSharedPtr<SVerticalBox> NodeBox;

	if (Schema->OnGetCustomPinBoxWidget.IsBound())
	{
		NodeBox = StaticCastSharedPtr<SVerticalBox>(Schema->OnGetCustomPinBoxWidget.Execute(JavascriptGraphPin).Widget);
	}

	if (PinToAdd->GetDirection() == EEdGraphPinDirection::EGPD_Input)
	{
		if (NodeBox.IsValid() == false)
		{
			NodeBox = LeftNodeBox;
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
		if (NodeBox.IsValid() == false)
		{
			NodeBox = RightNodeBox;
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

void SJavascriptGraphEdNode::UpdatePinSlate() 
{
	InputPins.Empty();
	OutputPins.Empty();
	RightNodeBox.Reset();
	LeftNodeBox.Reset();

	CreatePinWidgets();
}

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

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
	return FAppStyle::Get().GetBrush(TEXT("BTEditor.Graph.BTNode.Icon"));
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

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 27
void SJavascriptGraphEdNode::MoveTo(const FVector2D& NewPosition, FNodeSet& NodeFilter)
#else
void SJavascriptGraphEdNode::MoveTo(const FVector2D& NewPosition, FNodeSet& NodeFilter, bool bMarkDirty)
#endif
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
	
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 27
	SGraphNode::MoveTo(NewPosition, NodeFilter);
#else
	SGraphNode::MoveTo(NewPosition, NodeFilter, bMarkDirty);
#endif
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

void SJavascriptGraphEdNode::OnDragEnter(const FGeometry & MyGeometry, const FDragDropEvent & DragDropEvent)
{
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	if (Schema->OnDragEnter.IsBound())
	{
		TSharedPtr<FDragJavascriptGraphNode> DragOp = DragDropEvent.GetOperationAs<FDragJavascriptGraphNode>();
		if (DragOp.IsValid())
		{
			if (Schema->OnDragEnter.Execute(GraphEdNode, DragOp->GetDropTargetNode(), MyGeometry))
			{
				DragOp->SetHoveredNode(SharedThis(this));
			}
			else
			{
				DragOp->SetHoveredNode(TSharedPtr<SGraphNode>(nullptr));
			}
		}
		return;
	}

	SGraphNode::OnDragEnter(MyGeometry, DragDropEvent);
}

void SJavascriptGraphEdNode::OnDragLeave(const FDragDropEvent & DragDropEvent)
{
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	if (Schema->OnDragLeave.IsBound())
	{
		TSharedPtr<FDragJavascriptGraphNode> DragOp = DragDropEvent.GetOperationAs<FDragJavascriptGraphNode>();
		if (DragOp.IsValid())
		{
			if (Schema->OnDragLeave.Execute(GraphEdNode)) 
			{
				DragOp->SetHoveredNode(TSharedPtr<SGraphNode>(nullptr));
			}
		}
		return;
	}

	SGraphNode::OnDragLeave(DragDropEvent);
}

FReply SJavascriptGraphEdNode::OnDragOver(const FGeometry & MyGeometry, const FDragDropEvent & DragDropEvent)
{
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	if (Schema->OnDragOver.IsBound())
	{
		TSharedPtr<FDragJavascriptGraphNode> DragOp = DragDropEvent.GetOperationAs<FDragJavascriptGraphNode>();
		if (DragOp.IsValid())
		{
			if (Schema->OnDragOver.Execute(GraphEdNode, DragOp->GetDropTargetNode(), MyGeometry))
			{
				DragOp->SetHoveredNode(SharedThis(this));
			}
			else
			{
				DragOp->SetHoveredNode(TSharedPtr<SGraphNode>(nullptr));
			}
		}
		return FReply::Handled();
	}
	return SGraphNode::OnDragOver(MyGeometry, DragDropEvent);
}

FReply SJavascriptGraphEdNode::OnDrop(const FGeometry & MyGeometry, const FDragDropEvent & DragDropEvent)
{
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	if (Schema->OnDrop.IsBound())
	{
		TSharedPtr<FDragJavascriptGraphNode> DragOp = DragDropEvent.GetOperationAs<FDragJavascriptGraphNode>();
		if (DragOp.IsValid())
		{
			if (Schema->OnDrop.Execute(GraphEdNode, DragOp->GetDropTargetNode(), MyGeometry))
			{
				;// do something
			}
		}
		return FReply::Handled();
	}
	return SGraphNode::OnDrop(MyGeometry, DragDropEvent);
}

void SJavascriptGraphEdNode::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	if (Schema->OnMouseEnter.IsBound())
	{
		Schema->OnMouseEnter.Execute(GraphEdNode, FJavascriptSlateEdNode{ this }, MouseEvent);
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
		Schema->OnMouseLeave.Execute(GraphEdNode, FJavascriptSlateEdNode{ this }, MouseEvent);
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

		if (Schema->OnMouseMove.Execute(GraphEdNode, Delta, bUserIsDragging, (int32)MouseZone, MouseEvent))
		{
			const TSharedRef<SJavascriptGraphEdNode>& Node = SharedThis(this);
			return FReply::Handled().BeginDragDrop(FDragJavascriptGraphNode::New(Node));
		}
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
		if (Schema->OnMouseButtonDown.Execute(GraphEdNode, MyGeometry, MouseEvent))
		{
			return FReply::Handled();
		}
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
		if (Schema->OnMouseButtonUp.Execute(GraphEdNode, MyGeometry, MouseEvent))
		{
			return FReply::Handled();
		}
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

void SJavascriptGraphEdNode::CacheDesiredSize(float InLayoutScaleMultiplier)
{
	SGraphNode::CacheDesiredSize(InLayoutScaleMultiplier);

	if (InvalidationPanel.IsValid() && LastKnownLayoutScaleMultiplier != InLayoutScaleMultiplier)
	{
		LastKnownLayoutScaleMultiplier = InLayoutScaleMultiplier;

		FTimerDelegate Delegate = FTimerDelegate::CreateSP(this, &SJavascriptGraphEdNode::InvalidateGraphNodeWidget);
		GEditor->GetTimerManager()->SetTimerForNextTick(Delegate);
	}
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
	return GetNodeObj()->NodeComment;
}

void SJavascriptGraphEdNode::OnCommentTextCommitted(const FText& NewComment, ETextCommit::Type CommitInfo)
{
	const FString& TmpNewComment = NewComment.ToString();

	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	if (GraphEdNode && !GraphEdNode->NodeComment.Equals(TmpNewComment) && Schema->OnCommitedBubbleComment.IsBound())
	{
		Schema->OnCommitedBubbleComment.Execute(TmpNewComment);
	}

	GetNodeObj()->OnUpdateCommentText(TmpNewComment);
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
		UpdatePinSlate();
	}

	return FReply::Handled();
}

void SJavascriptGraphEdNode::CreateAdvancedViewArrow(TSharedPtr<SVerticalBox> MainBox)
{
	const bool bHidePins = OwnerGraphPanelPtr.IsValid() && (OwnerGraphPanelPtr.Pin()->GetPinVisibility() != SGraphEditor::Pin_Show);
	const bool bAnyAdvancedPin = GraphNode && (ENodeAdvancedPins::NoPins != GraphNode->AdvancedPinDisplay);
	if (bAnyAdvancedPin && !bHidePins && GraphNode && MainBox.IsValid())
	{
		MainBox->AddSlot()
			.AutoHeight()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Top)
			.Padding(3, 0, 3, 3)
			[
				SNew(SCheckBox)
				.Visibility(this, &SJavascriptGraphEdNode::AdvancedViewArrowVisibility)
			.OnCheckStateChanged(this, &SJavascriptGraphEdNode::OnAdvancedViewChanged)
			.IsChecked(this, &SJavascriptGraphEdNode::IsAdvancedViewChecked)
			.Cursor(EMouseCursor::Default)
			.Style(FAppStyle::Get(), "Graph.Node.AdvancedView")
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(SImage)
				.Image(this, &SJavascriptGraphEdNode::GetAdvancedViewArrow)
			]
			]
			];
	}
}

void SJavascriptGraphEdNode::OnAdvancedViewChanged(const ECheckBoxState NewCheckedState)
{
	if (GraphNode && (ENodeAdvancedPins::NoPins != GraphNode->AdvancedPinDisplay))
	{
		const bool bAdvancedPinsHidden = (NewCheckedState != ECheckBoxState::Checked);
		GraphNode->AdvancedPinDisplay = bAdvancedPinsHidden ? ENodeAdvancedPins::Hidden : ENodeAdvancedPins::Shown;
		UpdateGraphNode();
	}
}

EVisibility SJavascriptGraphEdNode::AdvancedViewArrowVisibility() const
{
	const bool bShowAdvancedViewArrow = GraphNode && (ENodeAdvancedPins::NoPins != GraphNode->AdvancedPinDisplay);
	return bShowAdvancedViewArrow ? EVisibility::Visible : EVisibility::Collapsed;
}

ECheckBoxState SJavascriptGraphEdNode::IsAdvancedViewChecked() const
{
	const bool bAdvancedPinsHidden = GraphNode && (ENodeAdvancedPins::Hidden == GraphNode->AdvancedPinDisplay);
	return bAdvancedPinsHidden ? ECheckBoxState::Unchecked : ECheckBoxState::Checked;
}

const FSlateBrush* SJavascriptGraphEdNode::GetAdvancedViewArrow() const
{
	const bool bAdvancedPinsHidden = GraphNode && (ENodeAdvancedPins::Hidden == GraphNode->AdvancedPinDisplay);
	return FAppStyle::Get().GetBrush(bAdvancedPinsHidden ? TEXT("Icons.ChevronDown") : TEXT("Icons.ChevronUp"));
}

void SJavascriptGraphEdNode::InvalidateGraphNodeWidget()
{
	if (InvalidationPanel.IsValid())
	{
		InvalidationPanel->InvalidateRootLayout();
	}
}
