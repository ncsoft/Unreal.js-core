#include "SJavascriptGraphEdNodePin.h"
#include "SLevelOfDetailBranchNode.h"
#include "JavascriptGraphAssetGraphSchema.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Components/Widget.h"

PRAGMA_DISABLE_SHADOW_VARIABLE_WARNINGS

void SJavascriptGraphPin::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
{
	this->SetCursor(EMouseCursor::Default);

	typedef SJavascriptGraphPin ThisClass;

	bShowLabel = true;

	SetVisibility(TAttribute<EVisibility>(this, &SJavascriptGraphPin::GetPinVisiblity));

	GraphPinObj = InPin;
	check(GraphPinObj != nullptr);

	const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
	check(Schema);

	auto GraphSchema = CastChecked<UJavascriptGraphAssetGraphSchema>(Schema);

	if (GraphSchema->OnUsingDefaultPin.IsBound())
	{
		auto bDisable = GraphSchema->OnUsingDefaultPin.Execute(FJavascriptEdGraphPin{ const_cast<UEdGraphPin*>(GraphPinObj) });
		if (bDisable)
		{
			SBorder::Construct(SBorder::FArguments()
				.BorderImage(this, &SJavascriptGraphPin::GetPinBorder)
				.BorderBackgroundColor(this, &SJavascriptGraphPin::GetPinColor)
				.OnMouseButtonDown(this, &ThisClass::OnPinMouseDown)
				.Cursor(this, &ThisClass::GetPinCursor)
			);
			return;
		}
	}

	const bool bIsInput = (GetDirection() == EGPD_Input);

	// Create the pin icon widget
	TSharedRef<SWidget> ActualPinWidget =
		SAssignNew(PinImage, SImage)
		.Image(this, &ThisClass::GetPinIcon)
		.IsEnabled(this, &ThisClass::GetIsConnectable)
		.ColorAndOpacity(this, &ThisClass::GetPinColor)
		.OnMouseButtonDown(this, &ThisClass::OnPinMouseDown)
		.Cursor(this, &ThisClass::GetPinCursor);
	if (GraphSchema->OnGetActualPinWidget.IsBound())
	{
		auto Widget = GraphSchema->OnGetActualPinWidget.Execute(FJavascriptEdGraphPin{ const_cast<UEdGraphPin*>(GraphPinObj) }).Widget;
		if (Widget.IsValid())
		{
			ActualPinWidget = Widget.ToSharedRef();
		}
	}

	// Create the pin indicator widget (used for watched values)
	static const FName NAME_NoBorder("NoBorder");
	TSharedRef<SWidget> PinStatusIndicator =
		SNew(SButton)
		.ButtonStyle(FAppStyle::Get(), NAME_NoBorder)
		.Visibility(this, &ThisClass::GetPinStatusIconVisibility)
		.ContentPadding(0)
		.OnClicked(this, &ThisClass::ClickedOnPinStatusIcon)
		[
			SNew(SImage)
			.Image(this, &ThisClass::GetPinStatusIcon)
		];
	if (GraphSchema->OnGetPinStatusIndicator.IsBound())
	{
		auto Widget = GraphSchema->OnGetPinStatusIndicator.Execute(FJavascriptEdGraphPin{ const_cast<UEdGraphPin*>(GraphPinObj) }).Widget;
		if (Widget.IsValid())
		{
			PinStatusIndicator = Widget.ToSharedRef();
		}
	}

	TSharedRef<SWidget> InLabelWidget = SNew(STextBlock)
		.Text(this, &ThisClass::GetPinLabel)
		.TextStyle(FAppStyle::Get(), InArgs._PinLabelStyle)
		.Visibility(this, &ThisClass::GetPinLabelVisibility)
		.ColorAndOpacity(this, &ThisClass::GetPinTextColor);
	TSharedRef<SWidget> InValueWidget = SNew(SBox);
	if (GraphSchema->OnGetValueWidget.IsBound())
	{
		auto Widget = GraphSchema->OnGetValueWidget.Execute(FJavascriptEdGraphPin{ const_cast<UEdGraphPin*>(GraphPinObj) }).Widget;
		if (Widget.IsValid())
		{
			InValueWidget = Widget.ToSharedRef();
		}
	}
	// Create the widget used for the pin body (status indicator, label, and value)
#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION > 22) || ENGINE_MAJOR_VERSION > 4
	LabelAndValue =
#else
	TSharedRef<SWrapBox> LabelAndValue =
#endif
		SNew(SWrapBox)
		.PreferredWidth(150.f);

	if (bIsInput)
	{
		LabelAndValue->AddSlot()
			.VAlign(VAlign_Center)
			[
				PinStatusIndicator
			];


		LabelAndValue->AddSlot()
			.VAlign(VAlign_Center)
			[
				InLabelWidget
			];
		LabelAndValue->AddSlot()
			.Padding(bIsInput ? FMargin(InArgs._SideToSideMargin, 0, 0, 0) : FMargin(0, 0, InArgs._SideToSideMargin, 0))
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.Padding(0.0f)
			.IsEnabled(this, &ThisClass::IsEditingEnabled)
			[
				InValueWidget
			]
			];
	}
	else
	{
		LabelAndValue->AddSlot()
			.VAlign(VAlign_Center)
			[
				InLabelWidget
			];

		LabelAndValue->AddSlot()
			.Padding(bIsInput ? FMargin(InArgs._SideToSideMargin, 0, 0, 0) : FMargin(0, 0, InArgs._SideToSideMargin, 0))
			.VAlign(VAlign_Center)
			[
				SNew(SBox)
				.Padding(0.0f)
			.IsEnabled(this, &ThisClass::IsEditingEnabled)
			[
				InValueWidget
			]
			];

		LabelAndValue->AddSlot()
			.VAlign(VAlign_Center)
			[
				PinStatusIndicator
			];
	}

	float SideMargin = 0.f;
	if (GraphSchema->OnGetSideMarginInPin.IsBound())
	{
		SideMargin = GraphSchema->OnGetSideMarginInPin.Execute(FJavascriptEdGraphPin{ const_cast<UEdGraphPin*>(GraphPinObj) });
	}
	
	TSharedPtr<SHorizontalBox> PinContent;
	if (bIsInput)
	{
		// Input pin
		FullPinHorizontalRowWidget = PinContent =
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(SideMargin, 0, InArgs._SideToSideMargin, 0)
			[
				ActualPinWidget
			]
		+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION > 22) || ENGINE_MAJOR_VERSION > 4
				LabelAndValue.ToSharedRef()
#else
				LabelAndValue
#endif
			];
	}
	else
	{
		// Output pin
		FullPinHorizontalRowWidget = PinContent = SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
#if (ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION > 22) || ENGINE_MAJOR_VERSION > 4
				LabelAndValue.ToSharedRef()
#else
				LabelAndValue
#endif			
			]
		+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(InArgs._SideToSideMargin, 0, SideMargin, 0)
			[
				ActualPinWidget
			];
	}

	// Set up a hover for pins that is tinted the color of the pin.
	SBorder::Construct(SBorder::FArguments()
		.BorderImage(this, &SJavascriptGraphPin::GetPinBorder)
		.BorderBackgroundColor(this, &SJavascriptGraphPin::GetPinColor)
		.OnMouseButtonDown(this, &ThisClass::OnPinNameMouseDown)
		[
			SNew(SLevelOfDetailBranchNode)
			.UseLowDetailSlot(this, &ThisClass::UseLowDetailPinNames)
			.LowDetail()
			[
				SNew(SSpacer)
			]
			.HighDetail()
			[
				PinContent.ToSharedRef()
			]
		]
	);

	TAttribute<FText> ToolTipAttribute = TAttribute<FText>::Create(TAttribute<FText>::FGetter::CreateSP(this, &SJavascriptGraphPin::GetTooltipText));
	SetToolTipText(ToolTipAttribute);
}

EVisibility SJavascriptGraphPin::GetDefaultValueVisibility() const
{
	const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
	check(Schema);

	auto GraphSchema = CastChecked<UJavascriptGraphAssetGraphSchema>(Schema);
	if (GraphSchema->OnGetDefaultValueVisibility.IsBound())
	{
		bool bVisibility = GraphSchema->OnGetDefaultValueVisibility.Execute(FJavascriptEdGraphPin{ const_cast<UEdGraphPin*>(GraphPinObj) });
		return bVisibility ? EVisibility::Visible : EVisibility::Collapsed;
	}

	return SGraphPin::GetDefaultValueVisibility();
}

const FSlateBrush* SJavascriptGraphPin::GetPinBorder() const
{
	const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
	check(Schema);

	auto GraphSchema = CastChecked<UJavascriptGraphAssetGraphSchema>(Schema);

	if (GraphSchema->OnGetSlateBrushName.IsBound())
	{
		FName SlateBrushName = GraphSchema->OnGetSlateBrushName.Execute(IsHovered(), FJavascriptEdGraphPin{ const_cast<UEdGraphPin*>(GraphPinObj) });
		if (SlateBrushName.IsNone() == false)
		{
			return FAppStyle::Get().GetBrush(SlateBrushName);
		}
	}

	return SGraphPin::GetPinBorder();
}

FSlateColor SJavascriptGraphPin::GetPinColor() const
{
	const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
	check(Schema);

	auto GraphSchema = CastChecked<UJavascriptGraphAssetGraphSchema>(Schema);

	if (GraphSchema->OnGetPinColor.IsBound())
	{
		return GraphSchema->OnGetPinColor.Execute(IsHovered(), FJavascriptEdGraphPin{ const_cast<UEdGraphPin*>(GraphPinObj) });
	}

	return SGraphPin::GetPinColor();
}

FSlateColor SJavascriptGraphPin::GetPinTextColor() const
{
	// SGraphPin::GetPinTextColor()
	if (UEdGraphNode* GraphNode = GraphPinObj->GetOwningNodeUnchecked())
	{
		const bool bDisabled = (!GraphNode->IsNodeEnabled() || GraphNode->IsDisplayAsDisabledForced() || !IsEditingEnabled() || GraphNode->IsNodeUnrelated());
		if (GraphPinObj->bOrphanedPin)
		{
			FLinearColor PinColor = FLinearColor::Red;
			if (bDisabled)
			{
				PinColor.A = .25f;
			}
			return PinColor;
		}
		else if (bDisabled)
		{
			return FLinearColor(1.0f, 1.0f, 1.0f, 0.5f);
		}
		if (bUsePinColorForText)
		{
			return GetPinColor();
		}
	}
	// else
	const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
	check(Schema);

	auto GraphSchema = CastChecked<UJavascriptGraphAssetGraphSchema>(Schema);

	if (GraphSchema->OnGetPinTextColor.IsBound())
	{
		return GraphSchema->OnGetPinTextColor.Execute(IsHovered(), FJavascriptEdGraphPin{ const_cast<UEdGraphPin*>(GraphPinObj) });
	}

	return FLinearColor::White;
}

FReply SJavascriptGraphPin::OnPinMouseDown(const FGeometry& SenderGeometry, const FPointerEvent& MouseEvent)
{
	bool NeedSupportMovingPin = true;

	const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
	check(Schema);
	auto GraphSchema = CastChecked<UJavascriptGraphAssetGraphSchema>(Schema);
	if (GraphSchema->OnMouseDownGraphPin.IsBound())
	{
		NeedSupportMovingPin = GraphSchema->OnMouseDownGraphPin.Execute(FJavascriptEdGraphPin{ const_cast<UEdGraphPin*>(GraphPinObj) }, SenderGeometry, MouseEvent);
	}

	if (!NeedSupportMovingPin)
	{
		return FReply::Unhandled();
	}

	return SGraphPin::OnPinMouseDown(SenderGeometry, MouseEvent);
}

void SJavascriptGraphPin::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	if (OwnerNodePtr.IsValid())
	{
		SGraphPin::OnMouseLeave(MouseEvent);
	}
}

EVisibility SJavascriptGraphPin::GetPinVisiblity() const
{
	// The pin becomes too small to use at low LOD, so disable the hit test.
	if (UseLowDetailPinNames())
	{
		return EVisibility::HitTestInvisible;
	}
	return EVisibility::Visible;
}

const FSlateBrush* SJavascriptGraphPin::GetPinIcon() const
{
	if (GraphPinObj->PinType.PinCategory == TEXT("exec"))
	{
		if (IsConnected())
		{
			return IsHovered() ? FAppStyle::Get().GetBrush(TEXT("Graph.ExecPin.ConnectedHovered")) : FAppStyle::Get().GetBrush(TEXT("Graph.ExecPin.Connected"));
		}
		else
		{
			return IsHovered() ? FAppStyle::Get().GetBrush(TEXT("Graph.ExecPin.DisconnectedHovered")) : FAppStyle::Get().GetBrush(TEXT("Graph.ExecPin.Disconnected"));
		}
	}

	return SGraphPin::GetPinIcon();
}

EVisibility SJavascriptGraphPin::GetPinLabelVisibility() const
{
	if (GraphPinObj->PinType.PinCategory == TEXT("exec"))
	{
		return EVisibility::Collapsed;
	}

	const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
	check(Schema);

	auto GraphSchema = CastChecked<UJavascriptGraphAssetGraphSchema>(Schema);
	if (GraphSchema->OnGetPinLabelVisibility.IsBound())
	{
		bool bVisible = GraphSchema->OnGetPinLabelVisibility.Execute(FJavascriptEdGraphPin{ const_cast<UEdGraphPin*>(GraphPinObj) });
		return bVisible ? EVisibility::Visible : EVisibility::Collapsed;
	}
	// 	else if (GraphPinObj->Direction == EGPD_Output)
	// 	{
	// 		return EVisibility::Collapsed;
	// 	}

	return SGraphPin::GetPinLabelVisibility();
}
