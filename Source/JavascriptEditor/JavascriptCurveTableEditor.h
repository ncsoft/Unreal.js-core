#pragma once

#include "Components/Widget.h"
#if WITH_EDITOR
#include "Editor/UnrealEd/Public/SCurveEditor.h"
#endif
#include "JavascriptCurveTableEditor.generated.h"

class SJavascriptCurveTableEditor : public SCurveEditor
{
	SLATE_BEGIN_ARGS(SJavascriptCurveTableEditor) {}
	SLATE_END_ARGS()

		void Construct(const FArguments& InArgs)
	{
		SCurveEditor::Construct(SCurveEditor::FArguments()
			.DesiredSize(FVector2D(128.0f, 64.0f)));
	}

	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override
	{
		if (MouseEvent.IsShiftDown())
		{
			SCurveEditor::OnMouseWheel(MyGeometry, MouseEvent);
			return FReply::Handled();
		}

		return FReply::Unhandled();
	}
};

UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptCurveTableEditor : public UWidget
{
	GENERATED_UCLASS_BODY()

#if WITH_EDITOR
	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "CurveTable")
	void Construct();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic, Category = "CurveTable")
	void Destruct();

	UFUNCTION(BlueprintCallable, Category = "CurveTable")
	void SetObject(UCurveTable* Object, bool bForceRefresh);

public:
	// UVisual interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	// End of UVisual interface

protected:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void OnWidgetRebuilt() override;
	// End of UWidget interface

protected:
	TSharedPtr<SJavascriptCurveTableEditor> CurveTableEditor;
	TWeakObjectPtr<UCurveTable> CurveTable;

#endif	
};
