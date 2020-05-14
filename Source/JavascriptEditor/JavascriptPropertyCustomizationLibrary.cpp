#include "JavascriptPropertyCustomizationLibrary.h"
#include "IPropertyTypeCustomization.h"
#include "IPropertyUtilities.h"
#include "IDetailChildrenBuilder.h"
#include "JavascriptPropertyCustomization.h"
#include "JavascriptUMG/JavascriptUMGLibrary.h"
#include "Components/Widget.h"

#if WITH_EDITOR
FJavascriptPropertyHandle UJavascriptPropertyCustomizationLibrary::GetChildHandle(FJavascriptPropertyHandle Parent, FName Name)
{
	FJavascriptPropertyHandle Out;
	if (Parent.IsValid())
	{
		Out.PropertyHandle = Parent->GetChildHandle(Name);
	}
	return Out;
}

FJavascriptPropertyHandle UJavascriptPropertyCustomizationLibrary::GetParentHandle(FJavascriptPropertyHandle Handle)
{
	FJavascriptPropertyHandle Out;
	if (Handle.IsValid())
	{
		Out.PropertyHandle = Handle->GetParentHandle();
	}
	return Out;
}

FJavascriptPropertyHandle UJavascriptPropertyCustomizationLibrary::GetKeyHandle(FJavascriptPropertyHandle Handle)
{
	FJavascriptPropertyHandle Out;
	if (Handle.IsValid())
	{
		Out.PropertyHandle = Handle->GetKeyHandle();
	}
	return Out;
}

bool UJavascriptPropertyCustomizationLibrary::IsValidHandle(FJavascriptPropertyHandle Handle)
{
	return Handle.IsValid();
}

FJavascriptSlateWidget UJavascriptPropertyCustomizationLibrary::CreatePropertyNameWidget(FJavascriptPropertyHandle Handle, const FText& NameOverride, const FText& ToolTipOverride, bool bDisplayResetToDefault, bool bHideText, bool bHideThumbnail)
{
	return{ Handle->CreatePropertyNameWidget(NameOverride, ToolTipOverride, bDisplayResetToDefault, !bHideText, !bHideThumbnail) };
}

FJavascriptSlateWidget UJavascriptPropertyCustomizationLibrary::CreatePropertyValueWidget(FJavascriptPropertyHandle Handle, bool bHideDefaultPropertyButtons)
{
	return{ Handle->CreatePropertyValueWidget(!bHideDefaultPropertyButtons) };
}

FString UJavascriptPropertyCustomizationLibrary::GetMetaData(FJavascriptPropertyHandle Handle, const FName& Key)
{
	return Handle->GetMetaData(Key);
}

EPropertyAccessResult UJavascriptPropertyCustomizationLibrary::GetValueAsFormattedString(FJavascriptPropertyHandle Handle, FString& OutValue)
{
	return EPropertyAccessResult(Handle->GetValueAsFormattedString(OutValue));
}
EPropertyAccessResult UJavascriptPropertyCustomizationLibrary::SetValueFromFormattedString(FJavascriptPropertyHandle Handle, const FString& InValue)
{
	return EPropertyAccessResult(Handle->SetValueFromFormattedString(InValue));
}

EPropertyAccessResult UJavascriptPropertyCustomizationLibrary::GetObjectValue(FJavascriptPropertyHandle Handle, UObject*& OutValue)
{
	return EPropertyAccessResult(Handle->GetValue(OutValue));
}

EPropertyAccessResult UJavascriptPropertyCustomizationLibrary::SetObjectValue(FJavascriptPropertyHandle Handle, const UObject* InValue)
{
	return EPropertyAccessResult(Handle->SetValue(InValue));
}

namespace
{
	template<typename T>
	EPropertyAccessResult GetStructPropertyValue(FJavascriptPropertyHandle Handle, T& OutValue)
	{
		FStructProperty* StructProperty = CastField<FStructProperty>(Handle->GetProperty());
		if (StructProperty != nullptr && StructProperty->Struct == T::StaticStruct())
		{
			TArray<const void*> RawDataArray;
			Handle->AccessRawData(RawDataArray);
			int32 NumOfData = RawDataArray.Num();
			if (NumOfData == 0)
			{
				return EPropertyAccessResult::Fail;
			}

			const void* RawData = RawDataArray[0];
			for (int32 i = 1; i < NumOfData; ++i)
			{
				if (RawDataArray[i] != RawData)
				{
					return EPropertyAccessResult::MultipleValues;
				}
			}

			OutValue = *static_cast<const T*>(RawData);
			return EPropertyAccessResult::Success;
		}

		return EPropertyAccessResult::Fail;
	}

	template<typename T>
	EPropertyAccessResult SetStructPropertyValue(FJavascriptPropertyHandle Handle, const T& InValue)
	{
		FStructProperty* StructProperty = CastField<FStructProperty>(Handle->GetProperty());
		if (StructProperty != nullptr && StructProperty->Struct == T::StaticStruct())
		{
			TArray<void*> RawDataArray;
			Handle->AccessRawData(RawDataArray);
			int32 NumOfData = RawDataArray.Num();
			if (NumOfData == 0)
			{
				return EPropertyAccessResult::Fail;
			}

			for (int32 i = 0; i < NumOfData; ++i)
			{
				*static_cast<T*>(RawDataArray[i]) = InValue;
			}

			return EPropertyAccessResult::Success;
		}

		return EPropertyAccessResult::Fail;
	}
}

EPropertyAccessResult UJavascriptPropertyCustomizationLibrary::GetJavascriptRefValue(FJavascriptPropertyHandle Handle, FJavascriptRef& OutValue)
{
	return GetStructPropertyValue(Handle, OutValue);
}

EPropertyAccessResult UJavascriptPropertyCustomizationLibrary::SetJavascriptRefValue(FJavascriptPropertyHandle Handle, const FJavascriptRef& InValue)
{
	return SetStructPropertyValue(Handle, InValue);
}

TFieldPath<FProperty> UJavascriptPropertyCustomizationLibrary::GetProperty(FJavascriptPropertyHandle Handle)
{
	return Handle->GetProperty();
}
void UJavascriptPropertyCustomizationLibrary::SetOnPropertyValueChanged(FJavascriptPropertyHandle Handle, UJavascriptPropertyCustomization* Custom)
{
	FSimpleDelegate Delegate;
	Delegate.BindLambda([Custom]() {
		Custom->OnPropertyValueChanged.Broadcast();
	});
	Handle->SetOnPropertyValueChanged(Delegate);
}

bool UJavascriptPropertyCustomizationLibrary::IsEditConst(FJavascriptPropertyHandle Handle)
{
	return Handle.IsValid() && Handle->IsEditConst();
}

bool UJavascriptPropertyCustomizationLibrary::IsArrayProperty(FJavascriptPropertyHandle Handle)
{
	return Handle.IsValid() && Handle->GetPropertyClass()->IsChildOf(FArrayProperty::StaticClass());
}

bool UJavascriptPropertyCustomizationLibrary::IsArrayPropertyWithValueType(FJavascriptPropertyHandle Handle)
{
	auto ArrayProperty = Handle.IsValid() ? CastField<FArrayProperty>(Handle->GetProperty()) : nullptr;
	auto InnerProperty = (ArrayProperty != nullptr) ? ArrayProperty->Inner : nullptr;
	return (InnerProperty != nullptr) && InnerProperty->IsA(FStructProperty::StaticClass());
}

int32 UJavascriptPropertyCustomizationLibrary::GetIndexInArray(FJavascriptPropertyHandle Handle)
{
	return Handle.IsValid() ? Handle->GetIndexInArray() : INDEX_NONE;
}

TArray<UObject*> UJavascriptPropertyCustomizationLibrary::GetOuterObjects(FJavascriptPropertyHandle Handle)
{
	TArray<UObject*> OuterObjects;
	if (Handle.IsValid())
	{
		Handle->GetOuterObjects(OuterObjects);
	}
	return OuterObjects;
}
FString UJavascriptPropertyCustomizationLibrary::GeneratePathToProperty(FJavascriptPropertyHandle Handle)
{
	return Handle.IsValid() ? Handle->GeneratePathToProperty() : FString();
}

FJavascriptDetailWidgetDecl UJavascriptPropertyCustomizationLibrary::WholeRowContent(FJavascriptDetailWidgetRow Row)
{
	return{ &(Row->WholeRowContent()) };
}
FJavascriptDetailWidgetDecl UJavascriptPropertyCustomizationLibrary::NameContent(FJavascriptDetailWidgetRow Row)
{
	return{ &(Row->NameContent()) };
}
FJavascriptDetailWidgetDecl UJavascriptPropertyCustomizationLibrary::ValueContent(FJavascriptDetailWidgetRow Row)
{
	return{ &(Row->ValueContent()) };
}
void UJavascriptPropertyCustomizationLibrary::SetFilterString(FJavascriptDetailWidgetRow Row, const FText& InFilterString)
{
	Row->FilterString(InFilterString);
}


FJavascriptDetailWidgetRow UJavascriptPropertyCustomizationLibrary::AddChildContent(FJavascriptDetailChildrenBuilder ChildBuilder, const FText& SearchString)
{
	return{ &(ChildBuilder->AddCustomRow(SearchString)) };
}
FJavascriptDetailPropertyRow UJavascriptPropertyCustomizationLibrary::AddChildProperty(FJavascriptDetailChildrenBuilder ChildBuilder, FJavascriptPropertyHandle PropertyHandle)
{
	FJavascriptDetailPropertyRow Row;
	if (PropertyHandle.IsValid())
	{
		Row.PropertyRow = &(ChildBuilder->AddProperty(PropertyHandle.PropertyHandle.ToSharedRef()));
	}
	return Row;
}

FJavascriptDetailPropertyRow UJavascriptPropertyCustomizationLibrary::AddExternalObjects(FJavascriptDetailChildrenBuilder ChildBuilder, TArray<UObject*>& Objects, FName UniqueIdName)
{
	return { (ChildBuilder->AddExternalObjects(Objects, UniqueIdName)) };
}

FJavascriptDetailPropertyRow UJavascriptPropertyCustomizationLibrary::AddExternalObjectProperty(FJavascriptDetailChildrenBuilder ChildBuilder, TArray<UObject*>& Objects, FName PropertyName, FName UniqueIdName, bool bAllowChildrenOverride, bool bCreateCategoryNodesOverride)
{
	FAddPropertyParams Params;
	Params.UniqueId(UniqueIdName);
	Params.AllowChildren(bAllowChildrenOverride);
	Params.CreateCategoryNodes(bCreateCategoryNodesOverride);
	return { (ChildBuilder->AddExternalObjectProperty(Objects, PropertyName, Params)) };
}

FJavascriptSlateWidget UJavascriptPropertyCustomizationLibrary::GenerateStructValueWidget(FJavascriptDetailChildrenBuilder ChildBuilder, FJavascriptPropertyHandle StructPropertyHandle)
{
	return { ChildBuilder->GenerateStructValueWidget(StructPropertyHandle.PropertyHandle.ToSharedRef()) };
}

void UJavascriptPropertyCustomizationLibrary::RequestRefresh(FJavascriptPropertyTypeCustomizationUtils CustomizationUtils, bool bForce)
{
	auto Utils = CustomizationUtils->GetPropertyUtilities();
	if (Utils.IsValid())
	{
		if (bForce)
			Utils->ForceRefresh();
		else
			Utils->RequestRefresh();
	}
}

#pragma region FJavascriptDetailPropertyRow
FJavascriptDetailWidgetRow UJavascriptPropertyCustomizationLibrary::CustomWidget(FJavascriptDetailPropertyRow Row, bool bShowChildren)
{
	return{ &Row->CustomWidget(bShowChildren) };
}
void UJavascriptPropertyCustomizationLibrary::BindVisibility(FJavascriptDetailPropertyRow Row, UJavascriptSimpleGetBoolDelegateWrapper* Wrapper)
{
	if (Row.PropertyRow != nullptr)
	{
		TAttribute<EVisibility> VisibilityAttr;
		if (Wrapper != nullptr && Wrapper->Delegate.IsBound())
		{
			VisibilityAttr = TAttribute<EVisibility>::Create(TAttribute<EVisibility>::FGetter::CreateLambda([Row, _Delegate = Wrapper->Delegate]() {
				EVisibility visibility = _Delegate.Execute() ? EVisibility::Visible : EVisibility::Collapsed;
				return visibility;
			}));
		}

		Row->Visibility(VisibilityAttr);
	}
}
#pragma endregion FJavascriptDetailPropertyRow


#pragma region FJavascriptDetailWidgetDecl
void UJavascriptPropertyCustomizationLibrary::SetContent(FJavascriptDetailWidgetDecl Decl, FJavascriptSlateWidget Widget)
{
	if (Widget.Widget.IsValid())
	{
		Decl.Get()[Widget.Widget.ToSharedRef()];
	}
}
void UJavascriptPropertyCustomizationLibrary::SetVAlign(FJavascriptDetailWidgetDecl Decl, EVerticalAlignment InAlignment)
{
	Decl->VAlign(InAlignment);
}
void UJavascriptPropertyCustomizationLibrary::SetHAlign(FJavascriptDetailWidgetDecl Decl, EHorizontalAlignment InAlignment)
{
	Decl->HAlign(InAlignment);
}
void UJavascriptPropertyCustomizationLibrary::SetMinDesiredWidth(FJavascriptDetailWidgetDecl Decl, float MinWidth)
{
	Decl->MinDesiredWidth(MinWidth);
}
void UJavascriptPropertyCustomizationLibrary::SetMaxDesiredWidth(FJavascriptDetailWidgetDecl Decl, float MaxWidth)
{
	Decl->MaxDesiredWidth(MaxWidth);
}
#pragma endregion FJavascriptDetailWidgetDecl

#endif
