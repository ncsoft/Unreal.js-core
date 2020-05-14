#include "JavascriptStyleSetLibrary.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

static UStruct* Struct_SlateColor;
static UStruct* Struct_ButtonStyle;
static UStruct* Struct_SlateBrush;
static UStruct* Struct_TextBlockStyle;
static UStruct* Struct_EditableTextStyle;
static UStruct* Struct_EditableTextBoxStyle;
static UStruct* Struct_CheckBoxStyle;
static UStruct* Struct_ComboBoxStyle;
static UStruct* Struct_ComboButtonStyle;
static UStruct* Struct_ProgressBarStyle;

void Prepare()
{
	if (Struct_SlateColor) return;	

	auto prepare = [&](UStruct*& Out, const TCHAR* Name)
	{
		FStructProperty* Prop = FindFProperty<FStructProperty>(UJavascriptStyleSetLibrary::StaticClass(), Name);
		Out = Prop->Struct;
	};

	prepare(Struct_SlateColor, TEXT("SlateColor"));
	prepare(Struct_ButtonStyle, TEXT("ButtonStyle"));
	prepare(Struct_SlateBrush, TEXT("SlateBrush"));
	prepare(Struct_TextBlockStyle, TEXT("TextBlockStyle"));
	prepare(Struct_EditableTextStyle, TEXT("EditableTextStyle"));
	prepare(Struct_EditableTextBoxStyle, TEXT("EditableTextBoxStyle"));
	prepare(Struct_CheckBoxStyle, TEXT("CheckBoxStyle"));
	prepare(Struct_ComboBoxStyle, TEXT("ComboBoxStyle"));
	prepare(Struct_ComboButtonStyle, TEXT("ComboButtonStyle"));
	prepare(Struct_ProgressBarStyle, TEXT("ProgressBarStyle"));
}

FSlateColor Fixup(const FSlateColor& Src)
{
	Prepare();

	if (Src.IsColorSpecified())
	{
		return FSlateColor(Src.GetSpecifiedColor());
	}
	else
	{
		return Src;
	}
}

void FixupStruct(UStruct* Struct, void* Dest, const void* Src)
{
	if (Struct == Struct_SlateColor)
	{
		*((FSlateColor*)Dest) = Fixup(*(FSlateColor*)Src);
		return;
	}

	for (TFieldIterator<FProperty> PropertyIt(Struct, EFieldIteratorFlags::IncludeSuper); PropertyIt; ++PropertyIt)
	{
		auto Property = *PropertyIt;
		if (FStructProperty* p = CastField<FStructProperty>(Property))
		{
			FixupStruct(p->Struct, p->ContainerPtrToValuePtr<void>(Dest), p->ContainerPtrToValuePtr<void>(Src));
		}
	}
}

FSlateBrush Fixup(const FSlateBrush& Src)
{
	Prepare();

	FSlateBrush Dest = Src;
	FixupStruct(Struct_SlateBrush, &Dest, &Src);
	return Dest;
}

static const ISlateStyle& GetStyleSet(const FJavascriptStyleSet& Handle)
{
	auto StyleSet = FSlateStyleRegistry::FindSlateStyle(Handle.StyleSetName);
	if (StyleSet)
	{
		return *StyleSet;
	}
	else
	{
		static TSharedPtr<FSlateStyleSet> Default;
		if (!Default.IsValid()) 
		{
			Default = MakeShareable(new FSlateStyleSet(FName()));
		}
		return *(Default.Get());
	}
}

template <typename T>
T FixupWidgetStyle(const FJavascriptStyleSet& Handle, UStruct* Struct, const FName& StyleName)
{
	Prepare();

	const auto& Src = GetStyleSet(Handle).GetWidgetStyle<T>(StyleName);
	T Dest = Src;
	FixupStruct(Struct, &Dest, &Src);
	return Dest;
}

FButtonStyle UJavascriptStyleSetLibrary::GetButtonStyle(const FJavascriptStyleSet& Handle, const FName& StyleName)
{
	return FixupWidgetStyle<FButtonStyle>(Handle, Struct_ButtonStyle, StyleName);
}

FTextBlockStyle UJavascriptStyleSetLibrary::GetTextBlockStyle(const FJavascriptStyleSet& Handle, const FName& StyleName)
{
	return FixupWidgetStyle<FTextBlockStyle>(Handle, Struct_TextBlockStyle, StyleName);
}

FEditableTextStyle UJavascriptStyleSetLibrary::GetEditableTextStyle(const FJavascriptStyleSet& Handle, const FName& StyleName)
{
	return FixupWidgetStyle<FEditableTextStyle>(Handle, Struct_EditableTextStyle, StyleName);
}

FEditableTextBoxStyle UJavascriptStyleSetLibrary::GetEditableTextBoxStyle(const FJavascriptStyleSet& Handle, const FName& StyleName)
{
	return FixupWidgetStyle<FEditableTextBoxStyle>(Handle, Struct_EditableTextBoxStyle, StyleName);
}

FCheckBoxStyle UJavascriptStyleSetLibrary::GetCheckBoxStyle(const FJavascriptStyleSet& Handle, const FName& StyleName)
{
	return FixupWidgetStyle<FCheckBoxStyle>(Handle, Struct_CheckBoxStyle, StyleName);
}

FComboBoxStyle UJavascriptStyleSetLibrary::GetComboBoxStyle(const FJavascriptStyleSet& Handle, const FName& StyleName)
{
	return FixupWidgetStyle<FComboBoxStyle>(Handle, Struct_ComboBoxStyle, StyleName);
}

FComboButtonStyle UJavascriptStyleSetLibrary::GetComboButtonStyle(const FJavascriptStyleSet& Handle, const FName& StyleName)
{
	return FixupWidgetStyle<FComboButtonStyle>(Handle, Struct_ComboButtonStyle, StyleName);
}

FProgressBarStyle UJavascriptStyleSetLibrary::GetProgressBarStyle(const FJavascriptStyleSet& Handle, const FName& StyleName)
{
	return FixupWidgetStyle<FProgressBarStyle>(Handle, Struct_ProgressBarStyle, StyleName);
}

FMargin UJavascriptStyleSetLibrary::GetMargin(const FJavascriptStyleSet& Handle, const FName& StyleName)
{
	return GetStyleSet(Handle).GetMargin(StyleName);
}

FSlateColor UJavascriptStyleSetLibrary::GetSlateColor(const FJavascriptStyleSet& Handle, const FName& StyleName)
{
	return Fixup(GetStyleSet(Handle).GetSlateColor(StyleName));
}

FSlateBrush UJavascriptStyleSetLibrary::GetBrush(const FJavascriptStyleSet& Handle, const FName& StyleName)
{
	auto Brush = GetStyleSet(Handle).GetBrush(StyleName);
	return Brush ? Fixup(*Brush) : FSlateBrush();
}

FLinearColor UJavascriptStyleSetLibrary::GetColor(const FJavascriptStyleSet& Handle, const FName& StyleName)
{
	return GetStyleSet(Handle).GetColor(StyleName);
}

FVector2D UJavascriptStyleSetLibrary::GetVector(const FJavascriptStyleSet& Handle, const FName& StyleName)
{
	return GetStyleSet(Handle).GetVector(StyleName);
}

float UJavascriptStyleSetLibrary::GetFloat(const FJavascriptStyleSet& Handle, const FName& StyleName)
{
	return GetStyleSet(Handle).GetFloat(StyleName);
}

FSlateFontInfo UJavascriptStyleSetLibrary::GetFontStyle(const FJavascriptStyleSet& Handle, const FName& StyleName)
{
	return GetStyleSet(Handle).GetFontStyle(StyleName);
}

FSlateSound UJavascriptStyleSetLibrary::GetSound(const FJavascriptStyleSet& Handle, const FName& StyleName)
{
	return GetStyleSet(Handle).GetSound(StyleName);
}
