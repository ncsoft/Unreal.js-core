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
			CustomCategory.AddCustomRow(FText::GetEmpty())
				.WholeRowContent()
				[
					BodyPtr->CustomWidget.Widget.ToSharedRef()
				];
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
			CustomCategory.AddCustomRow(FText::GetEmpty())
				.NameContent()
				[
					BodyPtr->NameWidget.Widget.ToSharedRef()
				]
				.ValueContent()
				[
					BodyPtr->ValueWidget.Widget.ToSharedRef()
				];
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
}
#endif

PRAGMA_ENABLE_SHADOW_VARIABLE_WARNINGS