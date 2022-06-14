#include "JavascriptCurveTableEditor.h"
#include "Engine/CurveTable.h"
#include "Widgets/Layout/SBox.h"

#define LOCTEXT_NAMESPACE "UMG"


UJavascriptCurveTableEditor::UJavascriptCurveTableEditor(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{}

#if WITH_EDITOR
void UJavascriptCurveTableEditor::SetObject(UCurveTable* Object, bool bCanEdit)
{
	if (Object == nullptr)
		return;
	
	if (CurveTable.Get() != Object)
	{
		CurveTable = Object;
	}

	if (CurveTableEditor.IsValid())
	{
		CurveTableEditor->SetCurveOwner(CurveTable.Get(), bCanEdit);
	}
}

TSharedRef<SWidget> UJavascriptCurveTableEditor::RebuildWidget()
{
	if (IsDesignTime())
	{
		return RebuildDesignWidget(SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("CurveTableEditor", "CurveTableEditor"))
			]);
	}
	else
	{
		CurveTableEditor = SNew(SJavascriptCurveTableEditor);
		if (CurveTable.IsValid())
		{
			CurveTableEditor->SetCurveOwner(CurveTable.Get(), false);
		}

		return CurveTableEditor.ToSharedRef();
	}
}

void UJavascriptCurveTableEditor::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();

	Construct();
}

void UJavascriptCurveTableEditor::ReleaseSlateResources(bool bReleaseChildren)
{
	if (CanSafelyRouteEvent())
	{
		Destruct();
	}

	Super::ReleaseSlateResources(bReleaseChildren);
	CurveTable.Reset();
	CurveTableEditor.Reset();
}
#endif

#undef LOCTEXT_NAMESPACE
