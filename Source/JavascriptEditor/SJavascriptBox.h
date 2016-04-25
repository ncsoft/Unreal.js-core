#pragma once

class SJavascriptBox
	: public SBox, public FGCObject
{
public:
	SLATE_BEGIN_ARGS(SJavascriptBox) { }
		SLATE_ARGUMENT(UWidget*, Widget)
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

		SBox::Construct(SBox::FArguments());
	}

private:
	UWidget* Widget;
};

