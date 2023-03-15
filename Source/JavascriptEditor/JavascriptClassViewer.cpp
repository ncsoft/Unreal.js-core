
#include "JavascriptClassViewer.h"
#include "Modules/ModuleManager.h"
#include "ClassViewerModule.h"
#include "ClassViewerFilter.h"

#include "Widgets/Layout/SWrapBox.h"

#define LOCTEXT_NAMESPACE "JavascriptClassViewer"

class FClassFilter : public IClassViewerFilter
{
public:
	/** All children of these classes will be included unless filtered out by another setting. */
	TSet< const UClass* > AllowedChildrenOfClasses;

	TSet< const UClass* > DisallowedClasses;

	/** Disallowed class flags. */
	EClassFlags DisallowedClassFlags;

	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		return !InClass->HasAnyClassFlags(DisallowedClassFlags) && InFilterFuncs->IfInClassesSet(DisallowedClasses, InClass) != EFilterReturn::Passed
			&& InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InClass) != EFilterReturn::Failed;
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InUnloadedClassData, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		return !InUnloadedClassData->HasAnyClassFlags(DisallowedClassFlags) && InFilterFuncs->IfInClassesSet(DisallowedClasses, InUnloadedClassData) != EFilterReturn::Passed
			&& InFilterFuncs->IfInChildOfClassesSet(AllowedChildrenOfClasses, InUnloadedClassData) != EFilterReturn::Failed;
	}
};

UJavascriptClassViewer::UJavascriptClassViewer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TSharedRef<SWidget> UJavascriptClassViewer::RebuildWidget()
{
	if (IsDesignTime())
	{
		return RebuildDesignWidget(
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("JavascriptClassViewer", "JavascriptClassViewer"))
			]
		);
	}
	else
	{
		if (OnGetDefaultValue.IsBound())
		{
			DefaultClass = OnGetDefaultValue.Execute();
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
						SAssignNew(ClassViewerAnchor, SComboButton)
						.ContentPadding(FMargin(2, 2, 2, 1))
						.MenuPlacement(MenuPlacement_BelowAnchor)
						.ButtonContent()
						[
							SNew(STextBlock)
							.Font(FAppStyle::Get().GetFontStyle("PropertyWindow.NormalFont"))
							.Text(TAttribute<FText>::Create([this]() { return OnGetComboTextValue(); }))
							.ToolTipText(TAttribute<FText>::Create([this]() { return GetObjectToolTip(); }))
						]
						.OnGetMenuContent(FOnGetContent::CreateLambda([this]() { return GeneratePathPicker(); }))
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

FReply UJavascriptClassViewer::OnClickUse()
{
	FEditorDelegates::LoadSelectedAssetsIfNeeded.Broadcast();

	const UClass* SelectedClass = GEditor->GetFirstSelectedClass(CategoryClass);
	if (SelectedClass != nullptr)
	{
		DefaultClass = const_cast<UClass*>(SelectedClass);

		if (OnSetDefaultValue.IsBound())
		{
			OnSetDefaultValue.Execute(this->GetValue());
		}
	}

	return FReply::Handled();
}

FText UJavascriptClassViewer::GetValue() const
{
	FText Value;
	if (DefaultClass != nullptr)
	{
		Value = FText::FromString(DefaultClass->GetFullName());
	}
	else
	{
		Value = FText::GetEmpty();
	}

	return Value;
}

FText UJavascriptClassViewer::GetObjectToolTip() const
{
	return GetValue();
}

FReply UJavascriptClassViewer::OnClickBrowse()
{
	if (DefaultClass != nullptr)
	{
		TArray<UObject*> Objects;
		Objects.Add(DefaultClass);

		GEditor->SyncBrowserToObjects(Objects);
	}

	return FReply::Handled();
}

FText UJavascriptClassViewer::OnGetComboTextValue() const
{
	FText Value = LOCTEXT("DefaultComboText", "Select Class");

	if (CategoryClass != nullptr)
	{
		if (UField* Field = Cast<UField>(DefaultClass))
		{
			Value = Field->GetDisplayNameText();
		}
	}

	return Value;
}

TSharedRef<SWidget> UJavascriptClassViewer::GeneratePathPicker()
{
	FClassViewerModule& ClassViewerModule = FModuleManager::Get().LoadModuleChecked<FClassViewerModule>(TEXT("ClassViewer"));
	FClassViewerInitializationOptions Options;
	Options.bShowNoneOption = true;
	TSharedPtr<FClassFilter> Filter = MakeShareable(new FClassFilter);
	Filter->DisallowedClassFlags = CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists;

	// This class and its children are the classes that we can show objects for
	for (UClass* AllowedChildrenOfClass : AllowedChildrenOfClasses)
	{
		Filter->AllowedChildrenOfClasses.Add(AllowedChildrenOfClass);
	}
	Options.ClassFilters.Add(Filter.ToSharedRef());

	return
		SNew(SBox)
		.HeightOverride(300)
		.WidthOverride(300)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::Get().GetBrush("Menu.Background"))
			[
				ClassViewerModule.CreateClassViewer(Options,
					FOnClassPicked::CreateLambda([&](UClass* InChosenClass)
					{
						if (DefaultClass != InChosenClass)
						{
							// Close the Class Viewer
							ClassViewerAnchor->SetIsOpen(false);

							DefaultClass = InChosenClass;

							if (OnSetDefaultValue.IsBound())
							{
								OnSetDefaultValue.Execute(this->GetValue());
							}
						}
					}
				))
			]
		];
}

#undef LOCTEXT_NAMESPACE
