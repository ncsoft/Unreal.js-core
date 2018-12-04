#include "PropertyEditor.h"
#if WITH_EDITOR
#include "IDetailsView.h"
#endif
#define LOCTEXT_NAMESPACE "UMG"

const FString UPropertyEditor::EmptyString;


UPropertyEditor::UPropertyEditor(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{}

#if WITH_EDITOR
void UPropertyEditor::SetObject(UObject* Object, bool bForceRefresh)
{
	ObjectsToInspect.Empty();
	ObjectsToInspect.Add(Object);	

	if (View.IsValid())
	{
		View->SetObject(Object, bForceRefresh);
	}
}

void UPropertyEditor::SetObjects(TArray<UObject*> Objects, bool bForceRefresh, bool bOverrideLock)
{
	ObjectsToInspect.Empty();
	for (auto Object : Objects)
	{
		ObjectsToInspect.Add(Object);
	}

	if (View.IsValid())
	{
		View->SetObjects(Objects, bForceRefresh, bOverrideLock);
	}
}

bool UPropertyEditor::IsPropertyReadOnly_Implementation(const FString& PropertyName, const FString& ParentPropertyName)
{
	return false;
}

bool UPropertyEditor::IsPropertyVisible_Implementation(const FString & PropertName, const FString & ParentPropertyName)
{
	return true;
}

TSharedRef<SWidget> UPropertyEditor::RebuildWidget()
{
	if (IsDesignTime())
	{
		return RebuildDesignWidget(SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("PropertyEditor", "PropertyEditor"))
			]);
	}
	else
	{
		FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		FDetailsViewArgs DetailsViewArgs(bUpdateFromSelection, bLockable, bAllowSearch, (FDetailsViewArgs::ENameAreaSettings)NameAreaSettings, bHideSelectionTip);
		View = EditModule.CreateDetailView(DetailsViewArgs);
				
		{
			TArray<UObject*> Objects;
			for (auto Object : ObjectsToInspect)
			{
				if (Object.IsValid())
				{
					Objects.Add(Object.Get());
				}
			}
			View->SetObjects(Objects);
		}

		View->OnFinishedChangingProperties().AddUObject(this, &UPropertyEditor::OnFinishedChangingProperties);
		bool bEditable = !bReadOnly;
		View->SetIsPropertyEditingEnabledDelegate(FIsPropertyEditingEnabled::CreateLambda([bEditable]() {
			return bEditable;
		}));
		View->SetIsPropertyReadOnlyDelegate(FIsPropertyReadOnly::CreateUObject(this, &UPropertyEditor::NativeIsPropertyReadOnly));
		View->SetIsPropertyVisibleDelegate(FIsPropertyVisible::CreateUObject(this, &UPropertyEditor::NativeIsPropertyVisible));

		return View.ToSharedRef();
	}
}

void UPropertyEditor::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();

	Construct();
}

void UPropertyEditor::ReleaseSlateResources(bool bReleaseChildren)
{
	if (View.IsValid()) 
	{
		if (CanSafelyRouteEvent())
		{
			Destruct();
		}
	}

	Super::ReleaseSlateResources(bReleaseChildren);

	View.Reset();
}

void UPropertyEditor::OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property != nullptr && OnChange.IsBound())
	{
		OnChange.Broadcast(PropertyChangedEvent.Property->GetFName(),
			(PropertyChangedEvent.MemberProperty != nullptr) ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None);
	}
}

bool UPropertyEditor::NativeIsPropertyReadOnly(const FPropertyAndParent& InPropertyAndParent)
{
	return IsPropertyReadOnly(InPropertyAndParent.Property.GetName(),
		(InPropertyAndParent.ParentProperty != nullptr) ? InPropertyAndParent.ParentProperty->GetName() : EmptyString);
}

bool UPropertyEditor::NativeIsPropertyVisible(const FPropertyAndParent & InPropertyAndParent)
{
	return IsPropertyVisible(InPropertyAndParent.Property.GetName(),
		(InPropertyAndParent.ParentProperty != nullptr) ? InPropertyAndParent.ParentProperty->GetName() : EmptyString);
}
#endif

#undef LOCTEXT_NAMESPACE
