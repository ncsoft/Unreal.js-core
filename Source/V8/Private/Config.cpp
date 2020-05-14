#include "Config.h"
#include "V8PCH.h"
#include "UObject/Package.h"
#include "UObject/MetaData.h"
#include "UObject/TextProperty.h"


FString UV8Config::Safeify(const FString& Name) const
{
	if (Name == "Object")
	{
		return TEXT("UObject");
	}
	else if (Name == "Node")
	{
		return TEXT("UNode");
	}
	else if (Name == "Function")
	{
		return TEXT("UFunction");
	}
	else if (Name == "PointerEvent")
	{
		return TEXT("UPointerEvent");
	}
	else if (Name == "Image")
	{
		return TEXT("UImage");
	}
	else if (Name == "Selection")
	{
		return TEXT("USelection");
	}
	else if (Name == "FocusEvent")
	{
		return TEXT("UFocusEvent");
	}
	else
	{
		return Name.Replace(TEXT(" "), TEXT(""));
	}
}

bool UV8Config::CanExportClass(const UClass* Class) const
{
	bool bCanExport = (Class->ClassFlags & (CLASS_RequiredAPI | CLASS_MinimalAPI)) != 0;

	return bCanExport;
}

bool UV8Config::CanExportFunction(const UClass* Class, const UFunction* Function) const
{
	// Delegate function is not a real function.
	if ((Function->FunctionFlags & FUNC_Delegate))
	{
		return false;
	}

	// Skip unsupported type or delegate properties which are handled in dedicated code path.
	for (TFieldIterator<FProperty> ParamIt(Function); ParamIt; ++ParamIt)
	{
		FProperty* Param = *ParamIt;
		if (Param->ArrayDim > 1 ||
			Param->IsA(FDelegateProperty::StaticClass()) ||
			Param->IsA(FMulticastDelegateProperty::StaticClass()) ||
			Param->IsA(FInterfaceProperty::StaticClass()))
		{
			return false;
		}
	}

	return true;
}

bool UV8Config::CanExportProperty(const UStruct* Class, const FProperty* Property) const
{
	// Skip unsupported static array and interface.
	if (Property->ArrayDim > 1 ||
		Property->IsA(FInterfaceProperty::StaticClass()))
	{
		return false;
	}

	return true;
}

EPropertyAccessorAvailability UV8Config::GetPropertyAccessorAvailability(FProperty* Property) const
{
	if (Property == nullptr)
		return EPropertyAccessorAvailability::None;

	EPropertyAccessorAvailability Availability = EPropertyAccessorAvailability::Default;

	auto* PropClass = Property->GetClass();
	if (bGenAltPropAccessorForAllProp ||
		PropClass == FTextProperty::StaticClass())
	{
		Availability |= EPropertyAccessorAvailability::AltAccessor;
	}

	auto* ArrayProp = CastField<FArrayProperty>(Property);
	if (ArrayProp != nullptr &&
		ArrayProp->Inner != nullptr &&
		ArrayProp->Inner->GetClass() == FStructProperty::StaticClass())
	{
		Availability |= EPropertyAccessorAvailability::AltAccessor;

		if (bGenGetStructRefArrayFunction)
		{
			Availability |= EPropertyAccessorAvailability::StructRefArray;
		}
	}

	return Availability;
}

FString UV8Config::GetAlias(UFunction* Function, bool no_empty) const
{
	auto has_meta = [](UField* This, const FName& Key) {
		UPackage* Package = This->GetOutermost();
		check(Package);

		UMetaData* MetaData = Package->GetMetaData();
		check(MetaData);

		bool bHasMetaData = MetaData->HasValue(This, Key);

		return bHasMetaData;
	};

	auto get_meta = [](UField* This, const FName& Key) {
		UPackage* Package = This->GetOutermost();
		check(Package);

		UMetaData* MetaData = Package->GetMetaData();
		check(MetaData);

		const FString& MetaDataString = MetaData->GetValue(This, Key);

		return MetaDataString;
	};

	static FName NAME_DisplayName("DisplayName");
	if (!no_empty && has_meta(Function, NAME_DisplayName))
	{
		FString Name = get_meta(Function, NAME_DisplayName).Replace(TEXT(" "), TEXT(""));

		int32 Index;
		if (Name.FindChar('(', Index))
		{
			Name = Name.Mid(0, Index);
		}

		if (Name.Len() > 0)
		{
			bool pass = true;
			for (auto Ch : Name)
			{
				if (!FChar::IsIdentifier(Ch))
				{
					pass = false;
					break;
				}
			}

			if (pass && Name != Function->GetName())
			{
				return Name;
			}
		}
	}

	return no_empty ? Safeify(Function->GetName()) : TEXT("");
}
