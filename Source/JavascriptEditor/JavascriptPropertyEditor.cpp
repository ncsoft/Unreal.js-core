#include "JavascriptPropertyEditor.h"
#if WITH_EDITOR
#include "IDetailsView.h"
#include "Containers/Queue.h"
#endif
#define LOCTEXT_NAMESPACE "UMG"

const FString UPropertyEditor::EmptyString;
const TArray<FString> UPropertyEditor::EmptyStringArray;


UPropertyEditor::UPropertyEditor(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{}

#if WITH_EDITOR
void UPropertyEditor::SetObject(UObject* Object, bool bForceRefresh)
{
	BuildPropertyPathMap((Object != nullptr) ? Object->GetClass() : nullptr);

	ObjectsToInspect.Empty();
	ObjectsToInspect.Add(Object);

	if (View.IsValid())
	{
		View->SetObject(Object, bForceRefresh);
	}
}

void UPropertyEditor::SetObjects(TArray<UObject*> Objects, bool bForceRefresh, bool bOverrideLock)
{
	UObject* FirstObject = (Objects.Num() > 0) ? Objects[0] : nullptr;
	BuildPropertyPathMap((FirstObject != nullptr) ? FirstObject->GetClass() : nullptr);

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

void UPropertyEditor::ForceRefresh()
{
	if (View.IsValid())
	{
		View->ForceRefresh();
	}
}

bool UPropertyEditor::IsPropertyReadOnly_Implementation(const FString& PropertyName, const FString& ParentPropertyName, const TArray<FString>& PropertyPaths)
{
	return false;
}

bool UPropertyEditor::IsPropertyVisible_Implementation(const FString& PropertName, const FString& ParentPropertyName, const TArray<FString>& PropertyPaths)
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
		FDetailsViewArgs DetailsViewArgs;
		DetailsViewArgs.bUpdatesFromSelection = bUpdateFromSelection;
		DetailsViewArgs.bLockable = bLockable;
		DetailsViewArgs.bAllowSearch = bAllowSearch;
		DetailsViewArgs.NameAreaSettings = (FDetailsViewArgs::ENameAreaSettings)NameAreaSettings;
		DetailsViewArgs.bHideSelectionTip = bHideSelectionTip;

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
		//bool bEditable = !bReadOnly;
		View->SetIsPropertyEditingEnabledDelegate(FIsPropertyEditingEnabled::CreateLambda([this]() {
			auto ReadOnly = bReadOnly;
			if (ReadOnlyDelegate.IsBound())
			{
				ReadOnly = ReadOnly || ReadOnlyDelegate.Execute();
			}
			return !ReadOnly;
			//return bEditable;
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
	const TArray<FString>* PropertyPaths = PropertyPathMap.Find(&InPropertyAndParent.Property);

	return IsPropertyReadOnly(InPropertyAndParent.Property.GetName(),
		(InPropertyAndParent.ParentProperties.Num() > 0) ? InPropertyAndParent.ParentProperties[0]->GetName() : EmptyString,
		(PropertyPaths != nullptr) ? *PropertyPaths : EmptyStringArray);
}

bool UPropertyEditor::NativeIsPropertyVisible(const FPropertyAndParent& InPropertyAndParent)
{
	const TArray<FString>* PropertyPaths = PropertyPathMap.Find(&InPropertyAndParent.Property);

	return IsPropertyVisible(InPropertyAndParent.Property.GetName(),
		(InPropertyAndParent.ParentProperties.Num() > 0) ? InPropertyAndParent.ParentProperties[0]->GetName() : EmptyString,
		(PropertyPaths != nullptr) ? *PropertyPaths : EmptyStringArray);
}

void UPropertyEditor::BuildPropertyPathMap(UStruct* InPropertyRootType)
{
	if (bEnablePropertyPath == false)
		return;

	// Note: Using UProperty* as the key for PropertyPathMap has a limit of ambiguous detection
	// when multiple properties with the same type(UObject or UScriptStruct) exist in InPropertyRootType.
	// This limit comes from the interface of IDetailsView::SetIsPropertyReadOnlyDelegate,
	// which is intended to determine readonly-ness only with the meta data of a property and its parent,
	// not including object instances nor variables determined at run-time.

	if (InPropertyRootType == PropertyRootType)
	{
		return;
	}

	PropertyRootType = InPropertyRootType;
	PropertyPathMap.Reset();

	if (InPropertyRootType == nullptr)
	{
		return;
	}

	struct FFieldInfo
	{
		FFieldInfo()
			: Field(nullptr)
			, bIsInArray(false)
		{}
		FFieldInfo(FField* InField, const FString& InParentPath, bool InIsInArray)
			: Field(InField)
			, ParentPath(InParentPath)
			, bIsInArray(InIsInArray)
		{}

		FField* Field;
		FString ParentPath;
		bool bIsInArray;
	};

	TQueue<FFieldInfo> FieldInfoQueue;
	FieldInfoQueue.Enqueue(FFieldInfo(InPropertyRootType->ChildProperties, FString(), false));

	TSet<UStruct*> ResolvedTypeStructSet;
	ResolvedTypeStructSet.Add(InPropertyRootType);

	FFieldInfo FieldInfo;
	while (FieldInfoQueue.Dequeue(FieldInfo))
	{
		for (FField* Field = FieldInfo.Field; Field != nullptr; Field = Field->Next)
		{
			if (FProperty* Property = CastField<FProperty>(Field))
			{
				FString PropPath;
				if (FieldInfo.ParentPath.Len() > 0)
				{
					// Child property in an array has the same name with the container property.
					// So we should not append name of current property, but append post-fix '[]' instead.
					PropPath = FieldInfo.bIsInArray
						? FString::Printf(TEXT("%s[]"), *FieldInfo.ParentPath)
						: FString::Printf(TEXT("%s.%s"), *FieldInfo.ParentPath, *Property->GetName());
				}
				else
				{
					// Root property never can be in an array. So we ignore TypeStructInfo.bIsInArray here.
					PropPath = Property->GetName();
				}
				PropertyPathMap.FindOrAdd(Property).AddUnique(PropPath);

				FField* ChildField = nullptr;
				bool bIsChildInArray = false;

				if (FStructProperty* StructProp = CastField<FStructProperty>(Property))
				{
					bool bWasAlreadyInSet = false;
					ResolvedTypeStructSet.Add(StructProp->Struct, &bWasAlreadyInSet);
					if (bWasAlreadyInSet == false)
					{
						ChildField = StructProp->Struct->ChildProperties;
					}
				}
				else if (FObjectProperty* ObjectProp = CastField<FObjectProperty>(Property))
				{
					bool bWasAlreadyInSet = false;
					ResolvedTypeStructSet.Add(ObjectProp->PropertyClass, &bWasAlreadyInSet);
					if (bWasAlreadyInSet == false)
					{
						ChildField = ObjectProp->PropertyClass->ChildProperties;
					}
				}
				else if (FArrayProperty* ArrayProp = CastField<FArrayProperty>(Property))
				{
					// ArrayProp->Inner is a pointer to UProperty and ArrayProp->Inner->Next is always nullptr.
					// So, we don't need to deal with ResolvedTypeStructSet here.
					ChildField = ArrayProp->Inner;
					bIsChildInArray = true;
				}

				if (ChildField != nullptr)
				{
					FieldInfoQueue.Enqueue(FFieldInfo(ChildField, PropPath, bIsChildInArray));
				}
			}
		}
	}
}
#endif

#undef LOCTEXT_NAMESPACE
