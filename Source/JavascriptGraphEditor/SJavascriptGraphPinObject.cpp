// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

#include "SJavascriptGraphPinObject.h"
#include "Modules/ModuleManager.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Editor.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "ScopedTransaction.h"
#include "Engine/Selection.h"
#include "SLevelOfDetailBranchNode.h"

#define LOCTEXT_NAMESPACE "SJavascriptGraphPinObject"

namespace JavascriptGraphPinObjectDefs
{
	// Active Combo pin alpha
	static const float ActiveComboAlpha = 1.f;
	// InActive Combo pin alpha
	static const float InActiveComboAlpha = 0.6f;
	// Active foreground pin alpha
	static const float ActivePinForegroundAlpha = 1.f;
	// InActive foreground pin alpha
	static const float InactivePinForegroundAlpha = 0.15f;
	// Active background pin alpha
	static const float ActivePinBackgroundAlpha = 0.8f;
	// InActive background pin alpha
	static const float InactivePinBackgroundAlpha = 0.4f;
};

void SJavascriptGraphPinObject::Construct(const FArguments& InArgs, UEdGraphPin* InGraphPinObj, class UJavascriptGraphPinObject* InWidget)
{
	this->SetCursor(EMouseCursor::Default);

	typedef SJavascriptGraphPinObject ThisClass;

	bShowLabel = true;

	GraphPinObj = InGraphPinObj;
	Widget = InWidget;

	check(GraphPinObj != NULL);
	check(Widget != NULL);

	const UEdGraphSchema* Schema = GraphPinObj->GetSchema();
	check(Schema);

	TSharedRef<SWrapBox> LabelAndValue =
		SNew(SWrapBox)
		.PreferredWidth(150.f);

	TSharedRef<SWidget> ValueWidget = GetValueWidget();
	if (ValueWidget != SNullWidget::NullWidget)
	{
		LabelAndValue->AddSlot()
			.Padding(FMargin(0, 0, InArgs._SideToSideMargin, 0))
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
	TSharedPtr<SHorizontalBox> PinContent;
	FullPinHorizontalRowWidget = PinContent = SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			LabelAndValue
		];

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
				SNew(SBox)
			]
			.HighDetail()
			[
				PinContent.ToSharedRef()
			]
		]
	);
}

TSharedRef<SWidget>	SJavascriptGraphPinObject::GetValueWidget()
{
	typedef SJavascriptGraphPinObject ThisClass;

	if (AllowSelfPinWidget())
	{
		UObject* DefaultObject = GraphPinObj->DefaultObject;

		if (GraphPinObj->GetSchema()->IsSelfPin(*GraphPinObj))
		{
			return SNew(SEditableTextBox)
				.Style(FEditorStyle::Get(), "Graph.EditableTextBox")
				.Text(this, &ThisClass::GetValue)
				.SelectAllTextWhenFocused(false)
				.IsReadOnly(true)
				.ForegroundColor(FSlateColor::UseForeground());
		}
	}
	// Don't show literal buttons for component type objects
	if (GraphPinObj->GetSchema()->ShouldShowAssetPickerForPin(GraphPinObj))
	{
		return
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(2, 0)
			.MaxWidth(100.0f)
			[
				SAssignNew(AssetPickerAnchor, SComboButton)
				.ButtonStyle(FEditorStyle::Get(), "PropertyEditor.AssetComboStyle")
			.ForegroundColor(this, &ThisClass::OnGetComboForeground)
			.ContentPadding(FMargin(2, 2, 2, 1))
			.ButtonColorAndOpacity(this, &ThisClass::OnGetWidgetBackground)
			.MenuPlacement(MenuPlacement_BelowAnchor)
			.ButtonContent()
			[
				SNew(STextBlock)
				.ColorAndOpacity(this, &ThisClass::OnGetComboForeground)
			.TextStyle(FEditorStyle::Get(), "PropertyEditor.AssetClass")
			.Font(FEditorStyle::GetFontStyle("PropertyWindow.NormalFont"))
			.Text(this, &ThisClass::OnGetComboTextValue)
			.ToolTipText(this, &ThisClass::GetObjectToolTip)
			]
		.OnGetMenuContent(this, &ThisClass::GenerateAssetPicker)
			]
		// Use button
		+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(1, 0)
			.VAlign(VAlign_Center)
			[
				SAssignNew(UseButton, SButton)
				.ButtonStyle(FEditorStyle::Get(), "NoBorder")
			.ButtonColorAndOpacity(this, &ThisClass::OnGetWidgetBackground)
			.OnClicked(GetOnUseButtonDelegate())
			.ContentPadding(1.f)
			.ToolTipText(NSLOCTEXT("GraphEditor", "ObjectGraphPin_Use_Tooltip", "Use asset browser selection"))
			[
				SNew(SImage)
				.ColorAndOpacity(this, &ThisClass::OnGetWidgetForeground)
			.Image(FEditorStyle::GetBrush(TEXT("PropertyWindow.Button_Use")))
			]
			]
		// Browse button
		+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(1, 0)
			.VAlign(VAlign_Center)
			[
				SAssignNew(BrowseButton, SButton)
				.ButtonStyle(FEditorStyle::Get(), "NoBorder")
			.ButtonColorAndOpacity(this, &ThisClass::OnGetWidgetBackground)
			.OnClicked(GetOnBrowseButtonDelegate())
			.ContentPadding(0)
			.ToolTipText(NSLOCTEXT("GraphEditor", "ObjectGraphPin_Browse_Tooltip", "Browse"))
			[
				SNew(SImage)
				.ColorAndOpacity(this, &ThisClass::OnGetWidgetForeground)
			.Image(FEditorStyle::GetBrush(TEXT("PropertyWindow.Button_Browse")))
			]
			];
	}

	return SNullWidget::NullWidget;
}

FOnClicked SJavascriptGraphPinObject::GetOnUseButtonDelegate()
{
	return FOnClicked::CreateSP(this, &SJavascriptGraphPinObject::OnClickUse);
}

FOnClicked SJavascriptGraphPinObject::GetOnBrowseButtonDelegate()
{
	return FOnClicked::CreateSP(this, &SJavascriptGraphPinObject::OnClickBrowse);
}

FText SJavascriptGraphPinObject::GetObjectToolTip() const
{
	return GetValue();
}

FString SJavascriptGraphPinObject::GetObjectToolTipAsString() const
{
	return GetValue().ToString();
}

FReply SJavascriptGraphPinObject::OnClickUse()
{
	FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();

	UClass* ObjectClass = Cast<UClass>(GraphPinObj->PinType.PinSubCategoryObject.Get());
	if (ObjectClass != NULL)
	{
		UObject* SelectedObject = GEditor->GetSelectedObjects()->GetTop(ObjectClass);
		if (SelectedObject != NULL)
		{
			GraphPinObj->GetSchema()->TrySetDefaultObject(*GraphPinObj, SelectedObject);

			if (Widget->OnSetDefaultValue.IsBound())
			{
				Widget->OnSetDefaultValue.Execute(this->GetValue());
			}
		}
	}

	return FReply::Handled();
}

FReply SJavascriptGraphPinObject::OnClickBrowse()
{
	if (GraphPinObj->DefaultObject != NULL)
	{
		TArray<UObject*> Objects;
		Objects.Add(GraphPinObj->DefaultObject);

		GEditor->SyncBrowserToObjects(Objects);
	}
	return FReply::Handled();
}

TSharedRef<SWidget> SJavascriptGraphPinObject::GenerateAssetPicker()
{
	// This class and its children are the classes that we can show objects for
	UClass* AllowedClass = Cast<UClass>(GraphPinObj->PinType.PinSubCategoryObject.Get());

	if (AllowedClass == NULL)
	{
		AllowedClass = UObject::StaticClass();
	}

	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	FAssetPickerConfig AssetPickerConfig;
	AssetPickerConfig.Filter.ClassNames.Add(AllowedClass->GetFName());
	AssetPickerConfig.bAllowNullSelection = true;
	AssetPickerConfig.Filter.bRecursiveClasses = true;
	AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateSP(this, &SJavascriptGraphPinObject::OnAssetSelectedFromPicker);
	AssetPickerConfig.InitialAssetViewType = EAssetViewType::List;
	AssetPickerConfig.bAllowDragging = false;

	// Check with the node to see if there is any "AllowClasses" metadata for the pin
	FString ClassFilterString = GraphPinObj->GetOwningNode()->GetPinMetaData(GraphPinObj->PinName, FName(TEXT("AllowedClasses")));
	if (!ClassFilterString.IsEmpty())
	{
		// Clear out the allowed class names and have the pin's metadata override.
		AssetPickerConfig.Filter.ClassNames.Empty();

		// Parse and add the classes from the metadata
		TArray<FString> CustomClassFilterNames;
		ClassFilterString.ParseIntoArray(CustomClassFilterNames, TEXT(","), true);
		for (auto It = CustomClassFilterNames.CreateConstIterator(); It; ++It)
		{
			AssetPickerConfig.Filter.ClassNames.Add(FName(**It));
		}
	}

	return
		SNew(SBox)
		.HeightOverride(300)
		.WidthOverride(300)
		[
			SNew(SBorder)
			.BorderImage(FEditorStyle::GetBrush("Menu.Background"))
		[
			ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
		]
		];
}

void SJavascriptGraphPinObject::OnAssetSelectedFromPicker(const FAssetData& AssetData)
{
	UObject* AssetObject = AssetData.GetAsset();
	if (GraphPinObj->DefaultObject != AssetObject)
	{
		const FScopedTransaction Transaction(NSLOCTEXT("GraphEditor", "ChangeObjectPinValue", "Change Object Pin Value"));
		GraphPinObj->Modify();

		// Close the asset picker
		AssetPickerAnchor->SetIsOpen(false);

		// Set the object found from the asset picker
		GraphPinObj->GetSchema()->TrySetDefaultObject(*GraphPinObj, AssetObject);

		if (Widget->OnSetDefaultValue.IsBound())
		{
			Widget->OnSetDefaultValue.Execute(this->GetValue());
		}
	}
}


FText SJavascriptGraphPinObject::GetValue() const
{
	UObject* DefaultObject = GraphPinObj->DefaultObject;
	FText Value;
	if (DefaultObject != NULL)
	{
		Value = FText::FromString(DefaultObject->GetFullName());
	}
	else
	{
		if (GraphPinObj->GetSchema()->IsSelfPin(*GraphPinObj))
		{
			Value = FText::FromString(GraphPinObj->PinName);
		}
		else
		{
			Value = FText::GetEmpty();
		}
	}
	return Value;
}

FText SJavascriptGraphPinObject::GetObjectName() const
{
	FText Value = FText::GetEmpty();

	if (GraphPinObj != NULL)
	{
		UObject* DefaultObject = GraphPinObj->DefaultObject;
		if (DefaultObject != NULL)
		{
			Value = FText::FromString(DefaultObject->GetName());
			int32 StringLen = Value.ToString().Len();

			//If string is too long, then truncate (eg. "abcdefgijklmnopq" is converted as "abcd...nopq")
			const int32 MaxAllowedLength = 16;
			if (StringLen > MaxAllowedLength)
			{
				//Take first 4 characters
				FString TruncatedStr(Value.ToString().Left(4));
				TruncatedStr += FString(TEXT("..."));

				//Take last 4 characters
				TruncatedStr += Value.ToString().Right(4);
				Value = FText::FromString(TruncatedStr);
			}
		}
	}
	return Value;
}

FText SJavascriptGraphPinObject::GetDefaultComboText() const
{
	return LOCTEXT("DefaultComboText", "Select Asset");
}

FText SJavascriptGraphPinObject::OnGetComboTextValue() const
{
	FText Value = GetDefaultComboText();

	if (GraphPinObj != NULL)
	{
		UObject* DefaultObject = GraphPinObj->DefaultObject;
		if (UField* Field = Cast<UField>(DefaultObject))
		{
			Value = Field->GetDisplayNameText();
		}
		else if (DefaultObject != NULL)
		{
			Value = FText::FromString(DefaultObject->GetName());
		}
	}
	return Value;
}

FSlateColor SJavascriptGraphPinObject::OnGetComboForeground() const
{
	float Alpha = IsHovered() ? JavascriptGraphPinObjectDefs::ActiveComboAlpha : JavascriptGraphPinObjectDefs::InActiveComboAlpha;
	return FSlateColor(FLinearColor(1.f, 1.f, 1.f, Alpha));
}

FSlateColor SJavascriptGraphPinObject::OnGetWidgetForeground() const
{
	float Alpha = IsHovered() ? JavascriptGraphPinObjectDefs::ActivePinForegroundAlpha : JavascriptGraphPinObjectDefs::InactivePinForegroundAlpha;
	return FSlateColor(FLinearColor(1.f, 1.f, 1.f, Alpha));
}

FSlateColor SJavascriptGraphPinObject::OnGetWidgetBackground() const
{
	float Alpha = IsHovered() ? JavascriptGraphPinObjectDefs::ActivePinBackgroundAlpha : JavascriptGraphPinObjectDefs::InactivePinBackgroundAlpha;
	return FSlateColor(FLinearColor(1.f, 1.f, 1.f, Alpha));
}

#undef LOCTEXT_NAMESPACE
