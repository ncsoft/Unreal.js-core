#include "JavascriptPropertyCustomizationLibrary.h"
#include "IPropertyTypeCustomization.h"
#include "IDetailChildrenBuilder.h"

#if WITH_EDITOR
FJavascriptPropertyHandle UJavascriptPropertyCustomizationLibrary::GetChildHandle(FJavascriptPropertyHandle Parent, FName Name)
{
	FJavascriptPropertyHandle Out;
	if (Parent.PropertyHandle.IsValid())
	{
		Out.PropertyHandle = Parent->GetChildHandle(Name);
	}
	return Out;
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
UProperty* UJavascriptPropertyCustomizationLibrary::GetProperty(FJavascriptPropertyHandle Handle)
{
	return Handle->GetProperty();
}
void UJavascriptPropertyCustomizationLibrary::SetOnPropertyValueChanged(FJavascriptPropertyHandle Handle, FJavascriptFunction Function)
{
	FSimpleDelegate Delegate;
	Delegate.BindLambda([=] () {
		((FJavascriptFunction*)&Function)->Execute();
	});
	Handle->SetOnPropertyValueChanged(Delegate);
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
	return{ &(ChildBuilder->AddProperty(PropertyHandle.PropertyHandle.ToSharedRef() )) };
}
FJavascriptSlateWidget UJavascriptPropertyCustomizationLibrary::GenerateStructValueWidget(FJavascriptDetailChildrenBuilder ChildBuilder, FJavascriptPropertyHandle StructPropertyHandle)
{
	return{ ChildBuilder->GenerateStructValueWidget(StructPropertyHandle.PropertyHandle.ToSharedRef()) };
}


FJavascriptDetailWidgetRow UJavascriptPropertyCustomizationLibrary::CustomWidget(FJavascriptDetailPropertyRow Row, bool bShowChildren)
{
	return{ &Row->CustomWidget(bShowChildren) };
}

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
#endif