#pragma once

class SJavascriptBox
	: public SBox, public FGCObject
{
public:
	SLATE_BEGIN_ARGS(SJavascriptBox) { }
		SLATE_ARGUMENT(UWidget*, Widget)
		/** The widget content presented by the SBox */
		SLATE_DEFAULT_SLOT(FArguments, Content)
	SLATE_END_ARGS()

public:
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override
	{
		Collector.AddReferencedObject(Widget);
	}
	// End of FSerializableObject interface

	void Construct(const FArguments& InArgs)
	{
		Widget = InArgs._Widget;

		SBox::Construct(SBox::FArguments()
			[
				InArgs._Content.Widget
			]);
	}

private:
	UWidget* Widget;
};

