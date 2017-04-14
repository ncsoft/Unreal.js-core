#include "SJavascriptGraphEdNodePin.h"
#include "SLevelOfDetailBranchNode.h"
#include "JavascriptGraphAssetGraphSchema.h"
#include "SWrapBox.h"
#include "SImage.h"
#include "SButton.h"
#include "SBoxPanel.h"

void SJavascriptGraphPin::Construct(const FArguments& InArgs, UEdGraphPin* InPin)
{
	this->SetCursor(EMouseCursor::Default);

	typedef SJavascriptGraphPin ThisClass;

	bShowLabel = true;

	GraphPinObj = InPin;
	check(GraphPinObj != NULL);

	const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
	check(Schema);

	auto GraphSchema = CastChecked<UJavascriptGraphAssetGraphSchema>(Schema);

	if (GraphSchema->OnUsingDefaultPin.IsBound())
	{
		auto bDisable = GraphSchema->OnUsingDefaultPin.Execute();
		if (bDisable)
		{
			SBorder::Construct(SBorder::FArguments()
				.BorderImage(this, &SJavascriptGraphPin::GetPinBorder)
				.BorderBackgroundColor(this, &ThisClass::GetPinColor)
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
		.ButtonStyle(FEditorStyle::Get(), NAME_NoBorder)
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

	TSharedRef<SWidget> LabelWidget = SNew(STextBlock)
		.Text(this, &ThisClass::GetPinLabel)
		.TextStyle(FEditorStyle::Get(), InArgs._PinLabelStyle)
		.Visibility(this, &ThisClass::GetPinLabelVisibility)
		.ColorAndOpacity(this, &ThisClass::GetPinTextColor);
	if (GraphSchema->OnGetLabelWidget.IsBound())
	{
		auto Widget = GraphSchema->OnGetLabelWidget.Execute(FJavascriptEdGraphPin{ const_cast<UEdGraphPin*>(GraphPinObj) }).Widget;
		if (Widget.IsValid())
		{
			LabelWidget = Widget.ToSharedRef();
		}
	}
	// Create the widget used for the pin body (status indicator, label, and value)
	TSharedRef<SWrapBox> LabelAndValue =
		SNew(SWrapBox)
		.PreferredWidth(150.f);

	if (!bIsInput)
	{
		LabelAndValue->AddSlot()
			.VAlign(VAlign_Center)
			[
				PinStatusIndicator
			];

		LabelAndValue->AddSlot()
			.VAlign(VAlign_Center)
			[
				LabelWidget
			];
	}
	else
	{
		LabelAndValue->AddSlot()
			.VAlign(VAlign_Center)
			[
				LabelWidget
			];

		TSharedRef<SWidget> ValueWidget = GetDefaultValueWidget();

		if (ValueWidget != SNullWidget::NullWidget)
		{
			LabelAndValue->AddSlot()
				.Padding(bIsInput ? FMargin(InArgs._SideToSideMargin, 0, 0, 0) : FMargin(0, 0, InArgs._SideToSideMargin, 0))
				.VAlign(VAlign_Center)
				[
					SNew(SBox)
					.Padding(0.0f)
				.IsEnabled(this, &ThisClass::IsEditingEnabled)
				[
					ValueWidget
				]
				];
		}

		LabelAndValue->AddSlot()
			.VAlign(VAlign_Center)
			[
				PinStatusIndicator
			];
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
			.Padding(0, 0, InArgs._SideToSideMargin, 0)
			[
				ActualPinWidget
			]
		+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				LabelAndValue
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
				LabelAndValue
			]
		+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(InArgs._SideToSideMargin, 0, 0, 0)
			[
				ActualPinWidget
			];
	}
	
	// Set up a hover for pins that is tinted the color of the pin.
	SBorder::Construct(SBorder::FArguments()
		.BorderImage(this, &ThisClass::GetPinBorder)
		.BorderBackgroundColor(this, &ThisClass::GetPinColor)
		.OnMouseButtonDown(this, &ThisClass::OnPinNameMouseDown)
		[
			SNew(SLevelOfDetailBranchNode)
			.UseLowDetailSlot(this, &ThisClass::UseLowDetailPinNames)
		.LowDetail()
		[
			//@TODO: Try creating a pin-colored line replacement that doesn't measure text / call delegates but still renders
			ActualPinWidget
		]
	.HighDetail()
		[
			PinContent.ToSharedRef()
		]
		]
	);

}

const FSlateBrush* SJavascriptGraphPin::GetPinBorder() const
{
	const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
	check(Schema);

	auto GraphSchema = CastChecked<UJavascriptGraphAssetGraphSchema>(Schema);

	if (GraphSchema->OnGetSlateBrushName.IsBound())
	{
		FName SlateBrushName = GraphSchema->OnGetSlateBrushName.Execute(IsHovered());
		return FEditorStyle::GetBrush(SlateBrushName);
	}

	return SGraphPin::GetPinBorder();
}
