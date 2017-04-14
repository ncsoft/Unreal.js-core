#include "SJavascriptGraphEdNode.h"
#include "SJavascriptGraphEdNodePin.h"

#include "JavascriptGraphEdNode.h"
#include "JavascriptGraphAssetGraphSchema.h"
#include "SlateOptMacros.h"
#include "SBox.h"
#include "SBoxPanel.h"

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
	auto LeftNodeBoxWidget = SAssignNew(LeftNodeBox, SVerticalBox);
	auto RightNodeBoxWidget = SAssignNew(RightNodeBox, SVerticalBox);
	
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);

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

		if (Schema->OnTakeTitleWidget.IsBound())
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
				SAssignNew(LeftNodeBox, SVerticalBox)
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
				SAssignNew(RightNodeBox, SVerticalBox)
			]
		];
	

	if (GraphEdNode)
	{
		if (Schema->OnUsingCustomContent.IsBound())
		{
			auto bUsingCostomContent = Schema->OnUsingCustomContent.Execute(GraphEdNode);
			if (bUsingCostomContent)
			{
				ContentWidget = SNew(SOverlay)
					// PIN AREA
					+ SOverlay::Slot()
					.HAlign(HAlign_Fill)
					.VAlign(VAlign_Fill)
					[
						SAssignNew(RightNodeBox, SVerticalBox)
					]

				// STATE NAME AREA
				+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.Padding(10.0f)
					[
						UserWidget.ToSharedRef()
					];
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
					.VAlign(VAlign_Top)
					[
						ContentWidget.ToSharedRef()
					]
				+ SVerticalBox::Slot()
					.AutoHeight()
					[
						ErrorReportingWidget.ToSharedRef()
					]
			]
		];

	// @TODO: Create comment bubble
	//TSharedPtr<SCommentBubble> CommentBubble;
	//const FSlateColor CommentColor = GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;

	//SAssignNew(CommentBubble, SCommentBubble)
	//	.GraphNode(GraphNode)
	//	.Text(this, &SGraphNode::GetNodeComment)
	//	.OnTextCommitted(this, &SGraphNode::OnCommentTextCommitted)
	//	.ColorAndOpacity(CommentColor)
	//	.AllowPinning(true)
	//	.EnableTitleBarBubble(true)
	//	.EnableBubbleCtrls(true)
	//	.GraphLOD(this, &SGraphNode::GetCurrentLOD)
	//	.IsGraphNodeHovered(this, &SGraphNode::IsHovered);

	//GetOrAddSlot(ENodeZone::TopCenter)
	//	.SlotOffset(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetOffset))
	//	.SlotSize(TAttribute<FVector2D>(CommentBubble.Get(), &SCommentBubble::GetSize))
	//	.AllowScaling(TAttribute<bool>(CommentBubble.Get(), &SCommentBubble::IsScalingAllowed))
	//	.VAlign(VAlign_Top)
	//	[
	//		CommentBubble.ToSharedRef()
	//	];
	
	if (GraphEdNode)
	{
		if (Schema->OnDisableMakePins.IsBound())
		{
			bool bDisableMakePins = Schema->OnDisableMakePins.Execute(GraphEdNode);
			if (bDisableMakePins)
			{
				return;
			}
		}
	}

	CreatePinWidgets();
}

void SJavascriptGraphEdNode::CreatePinWidgets()
{
	UJavascriptGraphEdNode* GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);

	for (int32 PinIdx = 0; PinIdx < GraphEdNode->Pins.Num(); PinIdx++)
	{
		UEdGraphPin* MyPin = GraphEdNode->Pins[PinIdx];
		if (!MyPin->bHidden)
		{
			TSharedPtr<SGraphPin> NewPin;

			auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());			
			if (Schema->OnCreatePin.IsBound())
			{
				NewPin = StaticCastSharedPtr<SJavascriptGraphPin>(Schema->OnCreatePin.Execute(FJavascriptEdGraphPin{ MyPin }).Widget);
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

	if (PinToAdd->GetDirection() == EEdGraphPinDirection::EGPD_Input)
	{
		LeftNodeBox->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillHeight(1.0f)			
			[
				PinToAdd
			];
		InputPins.Add(PinToAdd);
	}
	else // Direction == EEdGraphPinDirection::EGPD_Output
	{
		RightNodeBox->AddSlot()
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.FillHeight(1.0f)
			[
				PinToAdd
			];
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

void SJavascriptGraphEdNode::MoveTo(const FVector2D& NewPosition, FNodeSet& NodeFilter)
{
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	
	if (GraphEdNode && Schema->OnSkipMoveTo.IsBound())
	{
		bool bSkip = Schema->OnSkipMoveTo.Execute(GraphEdNode);
		if (bSkip)
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

	SGraphNode::OnMouseEnter(MyGeometry, MouseEvent);
}

void SJavascriptGraphEdNode::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	auto GraphEdNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
	auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
	if (Schema->OnMouseLeave.IsBound())
	{
		Schema->OnMouseLeave.Execute(GraphEdNode, FJavascriptSlateEdNode{ this });
	}

	SGraphNode::OnMouseLeave(MouseEvent);
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
