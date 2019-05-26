// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "JavascriptAssetPicker.h"
#include "Modules/ModuleManager.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/SBoxPanel.h"
#include "Editor.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "ScopedTransaction.h"
#include "Engine/Selection.h"
#include "EditorStyle.h"
#include "EditorStyleSet.h"

#define LOCTEXT_NAMESPACE "JavascriptAssetPicker"

/////////////////////////////////////////////////////
// UJavascriptGraphPinObject
UJavascriptAssetPicker::UJavascriptAssetPicker(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TSharedRef<SWidget> UJavascriptAssetPicker::RebuildWidget()
{
	if (IsDesignTime())
	{
		return RebuildDesignWidget(
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("JavascriptGraphPinObject", "JavascriptGraphPinObject"))
			]
		);
	}
	else
	{
		if (OnGetDefaultValue.IsBound())
		{
			UObject* InDefaultObject = OnGetDefaultValue.Execute();
			DefaultObject = InDefaultObject;
		}

		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SWrapBox)
				.PreferredWidth(150.f)
				+ SWrapBox::Slot()
				.Padding(FMargin(0, 0, 5.0f, 0))
				.VAlign(VAlign_Center)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(2, 0)
					.MaxWidth(100.0f)
					[
						SAssignNew(AssetPickerAnchor, SComboButton)
						.ButtonStyle(FEditorStyle::Get(), "PropertyEditor.AssetComboStyle")
						.ContentPadding(FMargin(2, 2, 2, 1))
						.MenuPlacement(MenuPlacement_BelowAnchor)
						.ButtonContent()
						[									
							SNew(STextBlock)
							.TextStyle(FEditorStyle::Get(), "PropertyEditor.AssetClass")
							.Font(FEditorStyle::GetFontStyle("PropertyWindow.NormalFont"))
							.Text(TAttribute<FText>::Create([this]() { return OnGetComboTextValue(); }))
							.ToolTipText(TAttribute<FText>::Create([this]() { return GetObjectToolTip(); }))
						]
						.OnGetMenuContent(FOnGetContent::CreateLambda([this]() { return GenerateAssetPicker(); }))
					]
					// Use button
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(1, 0)
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.ButtonStyle(FEditorStyle::Get(), "NoBorder")
						.OnClicked(FOnClicked::CreateLambda([this]() { return OnClickUse(); }))
						.ContentPadding(1.f)
						.ToolTipText(NSLOCTEXT("GraphEditor", "ObjectGraphPin_Use_Tooltip", "Use asset browser selection"))
						[
							SNew(SImage)
							.Image(FEditorStyle::GetBrush(TEXT("PropertyWindow.Button_Use")))
						]
					]
					// Browse button
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(1, 0)
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.ButtonStyle(FEditorStyle::Get(), "NoBorder")
						.OnClicked(FOnClicked::CreateLambda([this]() { return OnClickBrowse(); }))
						.ContentPadding(0)
						.ToolTipText(NSLOCTEXT("GraphEditor", "ObjectGraphPin_Browse_Tooltip", "Browse"))
						[
							SNew(SImage)
							.Image(FEditorStyle::GetBrush(TEXT("PropertyWindow.Button_Browse")))
						]
					]
				]
			];
	}
}

FReply UJavascriptAssetPicker::OnClickUse()
{
	FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();

	UClass* ObjectClass = Cast<UClass>(CategoryObject);
	if (ObjectClass != NULL)
	{
		UObject* SelectedObject = GEditor->GetSelectedObjects()->GetTop(ObjectClass);
		if (SelectedObject != NULL)
		{
			DefaultObject = SelectedObject;

			if (OnSetDefaultValue.IsBound())
			{
				OnSetDefaultValue.Execute(this->GetValue());
			}
		}
	}

	return FReply::Handled();
}

FText UJavascriptAssetPicker::GetValue() const
{
	FText Value;
	if (DefaultObject != NULL)
	{
		Value = FText::FromString(DefaultObject->GetFullName());
	}
	else
	{
		Value = FText::GetEmpty();
	}

	return Value;
}

FText UJavascriptAssetPicker::GetObjectToolTip() const
{
	return GetValue();
}

FReply UJavascriptAssetPicker::OnClickBrowse()
{
	if (DefaultObject != NULL)
	{
		TArray<UObject*> Objects;
		Objects.Add(DefaultObject);

		GEditor->SyncBrowserToObjects(Objects);
	}

	return FReply::Handled();
}

FText UJavascriptAssetPicker::OnGetComboTextValue() const
{
	FText Value = LOCTEXT("DefaultComboText", "Select Asset");

	if (CategoryObject != NULL)
	{
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

TSharedRef<SWidget> UJavascriptAssetPicker::GenerateAssetPicker()
{
	// This class and its children are the classes that we can show objects for
	UClass* AllowedClass = Cast<UClass>(CategoryObject);

	if (AllowedClass == NULL)
	{
		AllowedClass = UObject::StaticClass();
	}

	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	FAssetPickerConfig AssetPickerConfig;
	AssetPickerConfig.Filter.ClassNames.Add(AllowedClass->GetFName());
	AssetPickerConfig.bAllowNullSelection = true;
	AssetPickerConfig.Filter.bRecursiveClasses = true;
	AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateLambda([this](const FAssetData& AssetData) { 
		UObject* AssetObject = AssetData.GetAsset();
		if (DefaultObject != AssetObject)
		{
			const FScopedTransaction Transaction(NSLOCTEXT("GraphEditor", "ChangeObjectPinValue", "Change Object Pin Value"));

			// Close the asset picker
			AssetPickerAnchor->SetIsOpen(false);

			DefaultObject = AssetObject;

			if (OnSetDefaultValue.IsBound())
			{
				OnSetDefaultValue.Execute(this->GetValue());
			}
		}
	});
	AssetPickerConfig.InitialAssetViewType = EAssetViewType::List;
	AssetPickerConfig.bAllowDragging = false;

	// Check with the node to see if there is any "AllowClasses" metadata for the pin
	FString ClassFilterString = AllowedClasses;
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

#undef LOCTEXT_NAMESPACE