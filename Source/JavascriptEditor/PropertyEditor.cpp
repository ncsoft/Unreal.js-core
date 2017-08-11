#include "PropertyEditor.h"
#if WITH_EDITOR
#include "IDetailsView.h"
#endif
#define LOCTEXT_NAMESPACE "UMG"

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
		
		return View.ToSharedRef();
	}
}

void UPropertyEditor::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	View.Reset();
}

void UPropertyEditor::OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent)
{
	if (PropertyChangedEvent.Property != nullptr && OnChange.IsBound())
	{
		OnChange.Broadcast(PropertyChangedEvent.Property->GetFName());
	}
}
#endif

#undef LOCTEXT_NAMESPACE