#include "JavascriptPropertyCustomizationLibrary.h"
#include "IPropertyTypeCustomization.h"
#include "IPropertyUtilities.h"
#include "IDetailChildrenBuilder.h"
#include "JavascriptPropertyCustomization.h"
#include "JavascriptUMGLibrary.h"
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

bool UJavascriptPropertyCustomizationLibrary::IsValidHandle(FJavascriptPropertyHandle Handle)
{
	return Handle.IsValid();
}

UWidget* UJavascriptPropertyCustomizationLibrary::CreatePropertyNameWidget(FJavascriptPropertyHandle Handle, const FText& NameOverride, const FText& ToolTipOverride, bool bDisplayResetToDefault, bool bHideText, bool bHideThumbnail)
{
	TSharedRef<SWidget> Widget = Handle->CreatePropertyNameWidget(NameOverride, ToolTipOverride, bDisplayResetToDefault, !bHideText, !bHideThumbnail);
	return UJavascriptUMGLibrary::CreateContainerWidget(Widget);
}
UWidget* UJavascriptPropertyCustomizationLibrary::CreatePropertyValueWidget(FJavascriptPropertyHandle Handle, bool bHideDefaultPropertyButtons)
{
	TSharedRef<SWidget> Widget = Handle->CreatePropertyValueWidget(!bHideDefaultPropertyButtons);
	return UJavascriptUMGLibrary::CreateContainerWidget(Widget);
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
UProperty* UJavascriptPropertyCustomizationLibrary::GetProperty(FJavascriptPropertyHandle Handle)
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
	return { (ChildBuilder->AddExternalObjectProperty(Objects, PropertyName, UniqueIdName, bAllowChildrenOverride, bCreateCategoryNodesOverride)) };
}

UWidget* UJavascriptPropertyCustomizationLibrary::GenerateStructValueWidget(FJavascriptDetailChildrenBuilder ChildBuilder, FJavascriptPropertyHandle StructPropertyHandle)
{
	TSharedRef<SWidget> Widget = ChildBuilder->GenerateStructValueWidget(StructPropertyHandle.PropertyHandle.ToSharedRef());
	return UJavascriptUMGLibrary::CreateContainerWidget(Widget);
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
void UJavascriptPropertyCustomizationLibrary::SetContent(FJavascriptDetailWidgetDecl Decl, UWidget* Widget)
{
	if (Widget)
	{
		Decl.Get()[Widget->TakeWidget()];
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
