PRAGMA_DISABLE_SHADOW_VARIABLE_WARNINGS

#include "JavascriptDetailCustomization.h"
#include "PropertyEditorModule.h"
#include "IDetailCustomization.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "DetailCategoryBuilder.h"

#if WITH_EDITOR
class FJavascriptWholeRowCustomTypeDetails : public IDetailCustomization
{
public:
	FJavascriptWholeRowCustomTypeDetails(UJavascriptWholeRowDetailCustomization* InBody)
		: Body(InBody)
	{}

private:
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& InDetailLayout) override
	{
		auto* BodyPtr = Body.Get();
		if (BodyPtr)
		{
			IDetailCategoryBuilder& CustomCategory = InDetailLayout.EditCategory(BodyPtr->CategoryName, FText::GetEmpty(), ECategoryPriority::Important);
			FDetailWidgetRow& DetailWidgetRow = CustomCategory.AddCustomRow(FText::GetEmpty());
			if (BodyPtr->CustomWidget.Widget.IsValid())
			{
				DetailWidgetRow.WholeRowContent()
				[
					BodyPtr->CustomWidget.Widget.ToSharedRef()
				];
			}
		}
	}

private:
	TWeakObjectPtr<UJavascriptWholeRowDetailCustomization> Body;
};

void UJavascriptWholeRowDetailCustomization::Register()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(TypeName, FOnGetDetailCustomizationInstance::CreateLambda([this] {
		return MakeShareable(new FJavascriptWholeRowCustomTypeDetails(this));
	}));
}

void UJavascriptWholeRowDetailCustomization::Unregister()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.UnregisterCustomPropertyTypeLayout(TypeName);
	CustomWidget.Widget.Reset();
}

//////////////////////////////////////////////////////////////////////////
class FJavascriptCustomTypeDetails : public IDetailCustomization
{
public:
	FJavascriptCustomTypeDetails(UJavascriptDetailCustomization* InBody)
		: Body(InBody)
	{}

private:
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& InDetailLayout) override
	{
		auto* BodyPtr = Body.Get();
		if (BodyPtr)
		{
			IDetailCategoryBuilder& CustomCategory = InDetailLayout.EditCategory(BodyPtr->CategoryName, FText::GetEmpty(), ECategoryPriority::Important);
			FDetailWidgetRow& DetailWidgetRow = CustomCategory.AddCustomRow(FText::GetEmpty());

			if (BodyPtr->NameWidget.Widget.IsValid())
			{
				DetailWidgetRow.NameContent()
				[
					BodyPtr->NameWidget.Widget.ToSharedRef()
				];
			}

			if (BodyPtr->ValueWidget.Widget.IsValid())
			{
				DetailWidgetRow.ValueContent()
				[
					BodyPtr->ValueWidget.Widget.ToSharedRef()
				];
			}
		}
	}

private:
	TWeakObjectPtr<UJavascriptDetailCustomization> Body;
};

void UJavascriptDetailCustomization::Register()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(TypeName, FOnGetDetailCustomizationInstance::CreateLambda([this] {
		return MakeShareable(new FJavascriptCustomTypeDetails(this));
	}));
}

void UJavascriptDetailCustomization::Unregister()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.UnregisterCustomPropertyTypeLayout(TypeName);
	NameWidget.Widget.Reset();
	ValueWidget.Widget.Reset();
}
#endif

PRAGMA_ENABLE_SHADOW_VARIABLE_WARNINGS
