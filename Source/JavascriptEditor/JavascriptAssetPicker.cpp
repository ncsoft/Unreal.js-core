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
#include "Runtime/Launch/Resources/Version.h"

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
			DefaultObjectPath = OnGetDefaultValue.Execute();
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
						.ButtonStyle(FAppStyle::Get(), "PropertyEditor.AssetComboStyle")
						.ContentPadding(FMargin(2, 2, 2, 1))
						.MenuPlacement(MenuPlacement_BelowAnchor)
						.ButtonContent()
						[									
							SNew(STextBlock)
							.TextStyle(FAppStyle::Get(), "PropertyEditor.AssetClass")
							.Font(FAppStyle::Get().GetFontStyle("PropertyWindow.NormalFont"))
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
						.ButtonStyle(FAppStyle::Get(), "SimpleButton")
						.OnClicked(FOnClicked::CreateLambda([this]() { return OnClickUse(); }))
						.ContentPadding(1.f)
						.ToolTipText(NSLOCTEXT("GraphEditor", "ObjectGraphPin_Use_Tooltip", "Use asset browser selection"))
						[
							SNew(SImage)
							.Image(FAppStyle::Get().GetBrush(TEXT("Icons.Use")))
						]
					]
					// Browse button
					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(1, 0)
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.ButtonStyle(FAppStyle::Get(), "SimpleButton")
						.OnClicked(FOnClicked::CreateLambda([this]() { return OnClickBrowse(); }))
						.ContentPadding(0)
						.ToolTipText(NSLOCTEXT("GraphEditor", "ObjectGraphPin_Browse_Tooltip", "Browse"))
						[
							SNew(SImage)
							.Image(FAppStyle::Get().GetBrush(TEXT("Icons.BrowseContent")))
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
	if (ObjectClass != nullptr)
	{
		UObject* SelectedObject = GEditor->GetSelectedObjects()->GetTop(ObjectClass);
		if (SelectedObject != nullptr)
		{
			DefaultObjectPath = SelectedObject->GetPathName();

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
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
	if (DefaultObjectPath.IsValid())
	{
		Value = FText::FromString(DefaultObjectPath.ToString());
	}
#else
	if (!DefaultObjectPath.IsEmpty())
	{
		Value = FText::FromString(DefaultObjectPath);
	}
#endif
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
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
	if (DefaultObjectPath.IsValid() && FPackageName::DoesPackageExist(DefaultObjectPath.ToString()))
#else
	if (!DefaultObjectPath.IsEmpty() && FPackageName::DoesPackageExist(DefaultObjectPath))
#endif
	{
		FSoftObjectPath SoftObjectPath(DefaultObjectPath);
		UObject* DefaultObject = SoftObjectPath.TryLoad();
		if (DefaultObject != nullptr)
		{
			TArray<UObject*> Objects;
			Objects.Add(DefaultObject);

			GEditor->SyncBrowserToObjects(Objects);
		}
	}

	return FReply::Handled();
}

FText UJavascriptAssetPicker::OnGetComboTextValue() const
{
	FText Value = LOCTEXT("DefaultComboText", "Select Asset");

	if (CategoryObject != nullptr)
	{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
		auto DefaultObjectPathString = DefaultObjectPath.ToString();
#else
		auto DefaultObjectPathString = DefaultObjectPath;
#endif
		if (!DefaultObjectPathString.IsEmpty())
		{
			FString LeftS, RightS;
			if (DefaultObjectPathString.Split(TEXT("/"), &LeftS, &RightS, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
			{
				FString PackS, AssetS;
				if (RightS.Split(TEXT("."), &PackS, &AssetS, ESearchCase::IgnoreCase, ESearchDir::FromEnd))
				{
					Value = FText::FromString(AssetS);
				}
				else
				{
					Value = FText::FromString(RightS);
				}
			}
			else
			{
				Value = FText::FromString(DefaultObjectPathString);
			}
		}
	}

	return Value;
}

TSharedRef<SWidget> UJavascriptAssetPicker::GenerateAssetPicker()
{
	// This class and its children are the classes that we can show objects for
	UClass* AllowedClass = Cast<UClass>(CategoryObject);

	if (AllowedClass == nullptr)
	{
		AllowedClass = UObject::StaticClass();
	}

	FContentBrowserModule& ContentBrowserModule = FModuleManager::Get().LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	FAssetPickerConfig AssetPickerConfig;
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
	AssetPickerConfig.Filter.ClassPaths.Add(FTopLevelAssetPath(AllowedClass));
#else
	AssetPickerConfig.Filter.ClassNames.Add(AllowedClass->GetFName());
#endif
	AssetPickerConfig.bAllowNullSelection = true;
	AssetPickerConfig.Filter.bRecursiveClasses = true;
	AssetPickerConfig.OnAssetSelected = FOnAssetSelected::CreateLambda([this](const FAssetData& AssetData) {

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
		auto ObjectPath = AssetData.GetSoftObjectPath().GetAssetPath();
		if (DefaultObjectPath != ObjectPath)
#else
		auto ObjectPath = AssetData.ObjectPath;
		if (*DefaultObjectPath != ObjectPath)
#endif
		{
			const FScopedTransaction Transaction(NSLOCTEXT("GraphEditor", "ChangeObjectPinValue", "Change Object Pin Value"));

			// Close the asset picker
			AssetPickerAnchor->SetIsOpen(false);

#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
			if (!ObjectPath.IsValid())
			{
				DefaultObjectPath = FTopLevelAssetPath();
			}
			else
			{
				DefaultObjectPath = ObjectPath;
			}
#else
			if (AssetData.ObjectPath.IsNone())
			{
				DefaultObjectPath.Empty();
			}
			else
			{
				DefaultObjectPath = AssetData.ObjectPath.ToString();
			}
#endif

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
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
		AssetPickerConfig.Filter.ClassPaths.Empty();
#else
		AssetPickerConfig.Filter.ClassNames.Empty();
#endif

		// Parse and add the classes from the metadata
		TArray<FString> CustomClassFilterNames;
		ClassFilterString.ParseIntoArray(CustomClassFilterNames, TEXT(","), true);
		for (auto It = CustomClassFilterNames.CreateConstIterator(); It; ++It)
		{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
			AssetPickerConfig.Filter.ClassPaths.Add(FTopLevelAssetPath(**It));
#else
			AssetPickerConfig.Filter.ClassNames.Add(FName(**It));
#endif
			
		}
	}

	return
		SNew(SBox)
		.HeightOverride(300)
		.WidthOverride(300)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::Get().GetBrush("Menu.Background"))
			[
				ContentBrowserModule.Get().CreateAssetPicker(AssetPickerConfig)
			]
		];
}

#undef LOCTEXT_NAMESPACE
