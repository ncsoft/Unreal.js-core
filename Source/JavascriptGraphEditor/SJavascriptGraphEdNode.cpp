#include "JavascriptGraphEditorPrivatePCH.h"
#include "SJavascriptGraphEdNode.h"
#include "JavascriptGraphEdNode.h"

void SJavascriptGraphEdNode::Construct(const FArguments& InArgs, UJavascriptGraphEdNode* InNode)
{
	GraphNode = InNode;
	UpdateGraphNode();
}

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
void SJavascriptGraphEdNode::UpdateGraphNode()
{
	const FMargin NodePadding = FMargin(2.0f);

	InputPins.Empty();
	OutputPins.Empty();

	// Reset variables that are going to be exposed, in case we are refreshing an already setup node.
	RightNodeBox.Reset();
	LeftNodeBox.Reset();	
	
	TSharedPtr<SWidget> UserWidget = SNew(SBox);

	BackgroundColor = FLinearColor::Yellow;

	{
		auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());
		auto StateNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);
		if (StateNode)
		{
			if (Schema->OnTakeWidget.IsBound())
			{
				auto Widget = Schema->OnTakeWidget.Execute(StateNode).Widget;
				if (Widget.IsValid())
				{
					UserWidget = Widget;
				}
			}

			BackgroundColor = StateNode->BackgroundColor;
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
			.BorderBackgroundColor(BackgroundColor)
			//.OnMouseButtonDown(this, &SJavascriptGraphEdNode::OnMouseDown)
			[
				SNew(SOverlay)

				// Pins and node details
				+ SOverlay::Slot()
				.HAlign(HAlign_Fill)
				.VAlign(VAlign_Fill)
				[
					SNew(SHorizontalBox)

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
					]
				]
			]
		];

	// Create comment bubble
	TSharedPtr<SCommentBubble> CommentBubble;
	const FSlateColor CommentColor = GetDefault<UGraphEditorSettings>()->DefaultCommentNodeTitleColor;

	SAssignNew(CommentBubble, SCommentBubble)
		.GraphNode(GraphNode)
		.Text(this, &SGraphNode::GetNodeComment)
		.OnTextCommitted(this, &SGraphNode::OnCommentTextCommitted)
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
}

void SJavascriptGraphEdNode::CreatePinWidgets()
{
	UJavascriptGraphEdNode* StateNode = CastChecked<UJavascriptGraphEdNode>(GraphNode);

	for (int32 PinIdx = 0; PinIdx < StateNode->Pins.Num(); PinIdx++)
	{
		UEdGraphPin* MyPin = StateNode->Pins[PinIdx];
		if (!MyPin->bHidden)
		{
			TSharedPtr<SGraphPin> NewPin;

			auto Schema = CastChecked<UJavascriptGraphAssetGraphSchema>(GraphNode->GetSchema());			
			if (Schema->OnCreatePin.IsBound())
			{
				NewPin = StaticCastSharedPtr<SGraphPin>(Schema->OnCreatePin.Execute(FJavascriptEdGraphPin{ MyPin }).Widget);
			}
			
			if (!NewPin.IsValid())
			{
				NewPin = CreatePinWidget(MyPin);
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
