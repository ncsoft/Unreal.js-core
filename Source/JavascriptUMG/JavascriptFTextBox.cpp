#include "JavascriptFTextBox.h"
#include "JavascriptLibrary.h"

#define LOCTEXT_NAMESPACE "JavascriptFTextBox"

#if WITH_EDITOR
#include "Editor/EditorWidgets/Public/STextPropertyEditableTextBox.h"

class FJavascriptEditableText : public IEditableTextProperty
{
public:
	FJavascriptEditableText(UJavascriptFTextBox* InEditableTextBox)
		: EditableTextBox(InEditableTextBox)
	{
	}

	virtual bool IsMultiLineText() const override
	{
		return true;
	}

	virtual bool IsPassword() const override
	{
		return false;
	}

	virtual bool IsReadOnly() const override
	{
		if (EditableTextBox->OnIsReadOnly.IsBound())
		{
			return EditableTextBox->OnIsReadOnly.Execute();
		}
		return false;
	}

	virtual bool IsDefaultValue() const override
	{
		return EditableTextBox->TextValue.EqualTo(EditableTextBox->DefaultTextValue);
	}

	virtual FText GetToolTipText() const override
	{
		return FText::GetEmpty();
	}

	virtual int32 GetNumTexts() const override
	{
		return 1;
	}

	virtual FText GetText(const int32 InIndex) const override
	{
		check(InIndex == 0);
		return EditableTextBox->TextValue;
	}

	virtual void SetText(const int32 InIndex, const FText& InText) override
	{
		check(InIndex == 0);
		EditableTextBox->TextValue = InText;
		EditableTextBox->HandleOnTextCommitted(InText);
	}

	virtual bool IsValidText(const FText& InText, FText& OutErrorMsg) const override
	{
		if (EditableTextBox->OnIsValidText.IsBound())
		{
			OutErrorMsg = FText::FromString(EditableTextBox->OnIsValidText.Execute(InText.ToString()));
			return OutErrorMsg.IsEmpty();
		}
		return true;
	}

	virtual void GetStableTextId(const int32 InIndex, const IEditableTextProperty::ETextPropertyEditAction InEditAction, const FString& InTextSource, const FString& InProposedNamespace, const FString& InProposedKey, FString& OutStableNamespace, FString& OutStableKey) const override
	{
		check(InIndex == 0);

		OutStableNamespace = InProposedNamespace;

		if (!InProposedKey.IsEmpty())
		{
			OutStableKey = InProposedKey;
		}
		else
		{
			OutStableKey = FGuid::NewGuid().ToString();
		}

		EditableTextBox->HandleOnNamespaceKeyChanged(OutStableNamespace, OutStableKey);
	}

#if ENGINE_MAJOR_VERSION < 5 || ENGINE_MINOR_VERSION < 1
	virtual void RequestRefresh() override
	{
	}
#endif

private:
	UJavascriptFTextBox* EditableTextBox;
};
#endif

UJavascriptFTextBox::UJavascriptFTextBox(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
#if WITH_EDITOR
	STextPropertyEditableTextBox::FArguments Defaults;
	WrapTextAt = Defaults._WrapTextAt.Get();
	AutoWrapText = Defaults._AutoWrapText.Get();
	MinimumDesiredWidth = Defaults._MinDesiredWidth.Get();
	MaximumDesiredHeight = Defaults._MaxDesiredHeight.Get();

	WidgetStyle = *Defaults._Style;
#endif
}

#if WITH_EDITOR
void UJavascriptFTextBox::SynchronizeProperties()
{
	Super::SynchronizeProperties();
}

void UJavascriptFTextBox::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyEditableTextBlock.Reset();
}

void UJavascriptFTextBox::HandleOnNamespaceKeyChanged(const FString& InNamespace, const FString& InKey)
{
	MyTextProperty.Namespace = InNamespace;
	MyTextProperty.Key = InKey;
}

void UJavascriptFTextBox::HandleOnTextCommitted(const FText& InText)
{
	FName TableId = NAME_None;
	FString Key;
	FTextInspector::GetTableIdAndKey(InText, TableId, Key);
	MyTextProperty.TableId = TableId;
	MyTextProperty.Namespace = FTextInspector::GetNamespace(InText).Get(FString());
	MyTextProperty.Key = FTextInspector::GetKey(InText).Get(FString());
	MyTextProperty.Value = InText.ToString();

	OnTextCommitted.Broadcast(MyTextProperty);
}

void UJavascriptFTextBox::HandleOnStringTableKeyChanged(const FName& InTableId, const FString& InKey)
{
	MyTextProperty.TableId = InTableId;
	MyTextProperty.Key = InKey;
}

TSharedRef<SWidget> UJavascriptFTextBox::RebuildWidget()
{
	auto defaultWidget = SNew(SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("JavascriptFTextBox", "JavascriptFTextBox"))
		];

	if (IsDesignTime())
	{
		return RebuildDesignWidget(defaultWidget);
	}
	else
	{
		if (OnGetDefaultValue.IsBound())
		{
			FJavascriptTextProperty TextProperty = OnGetDefaultValue.Execute();
			SetText(TextProperty);
		}

		MyEditableTextProperty = MakeShareable(new FJavascriptEditableText(this));

		MyEditableTextBlock = SNew(STextPropertyEditableTextBox, MyEditableTextProperty.ToSharedRef())
			.Style(&WidgetStyle)
			.WrapTextAt(WrapTextAt)
			.AutoWrapText(AutoWrapText)
			.MinDesiredWidth(MinimumDesiredWidth)
			.MaxDesiredHeight(MaximumDesiredHeight);

		return MyEditableTextBlock.ToSharedRef();
	}
}

void UJavascriptFTextBox::SetText(FJavascriptTextProperty& JavascriptTextProperty)
{
	MyTextProperty = JavascriptTextProperty;

	if (!MyTextProperty.TableId.IsNone())
	{
		TextValue = FText::FromStringTable(MyTextProperty.TableId, MyTextProperty.Key);
		DefaultTextValue = TextValue;
	}
	else if (!MyTextProperty.Value.IsEmpty())
	{
		if (MyTextProperty.Key.IsEmpty())
		{
			MyTextProperty.Key = FGuid::NewGuid().ToString();

			OnTextCommitted.Broadcast(MyTextProperty);
		}

		TextValue = FText::ChangeKey(MyTextProperty.Namespace, MyTextProperty.Key, FText::FromString(MyTextProperty.Value));
		DefaultTextValue = TextValue;
	}
	else
	{
		TextValue = FText::GetEmpty();
		DefaultTextValue = TextValue;
	}
}
#endif

#undef LOCTEXT_NAMESPACE
