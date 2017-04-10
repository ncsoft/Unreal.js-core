PRAGMA_DISABLE_SHADOW_VARIABLE_WARNINGS

#include "JavascriptPropertyCustomization.h"
#include "PropertyEditorModule.h"

#if WITH_EDITOR
static int32 NextId = 0;
class FJavascriptCustomPropertyTypeImpl : public IPropertyTypeCustomization
{
public:
	FJavascriptCustomPropertyTypeImpl(UJavascriptPropertyCustomization* InBody)
		: Id(NextId++), Body(InBody)
	{}

	virtual ~FJavascriptCustomPropertyTypeImpl()
	{
		if (Body.IsValid())
		{
			Body->OnDestroy.ExecuteIfBound(Id);
		}
	}
	
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{
		if (Body.IsValid())
		{
			Body->OnCustomizeHeader.ExecuteIfBound({ PropertyHandle }, { &HeaderRow }, { &CustomizationUtils }, Id);
		}
	}
	
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override
	{		
		if (Body.IsValid())
		{
			Body->OnCustomizeChildren.ExecuteIfBound({ PropertyHandle }, { &ChildBuilder }, { &CustomizationUtils }, Id);
		}
	}

private:
	int32 Id;
	TWeakObjectPtr<UJavascriptPropertyCustomization> Body;
}; 

void UJavascriptPropertyCustomization::Register()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout(PropertyTypeName, FOnGetPropertyTypeCustomizationInstance::CreateLambda([this] {
		return MakeShareable(new FJavascriptCustomPropertyTypeImpl(this));
	}));
}

void UJavascriptPropertyCustomization::Unregister()
{
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.UnregisterCustomPropertyTypeLayout(PropertyTypeName);
}
#endif

PRAGMA_ENABLE_SHADOW_VARIABLE_WARNINGS