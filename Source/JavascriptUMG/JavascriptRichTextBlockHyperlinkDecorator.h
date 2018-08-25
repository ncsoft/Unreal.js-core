#pragma once

#include "Components/RichTextBlockDecorator.h"
#include "Framework/Text/SlateHyperlinkRun.h"
#include "JavascriptRichTextBlockHyperlinkDecorator.generated.h"

class UJavascriptContext;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FJavascriptHyperlinkSignature, UJavascriptRichTextBlockHyperlinkDecorator*, Self);

UCLASS(Experimental)
class JAVASCRIPTUMG_API UJavascriptRichTextBlockHyperlinkDecorator : public URichTextBlockDecorator
{
	GENERATED_BODY()

public:	
	UPROPERTY(EditAnywhere, Category = "Scripting | Javascript")	
	FString HyperlinkId;

	UPROPERTY(BlueprintAssignable, Category = "Scripting | Javascript")
	FJavascriptHyperlinkSignature OnClick;

	virtual TSharedPtr<ITextDecorator> CreateDecorator(URichTextBlock* InOwner) override;

	void HandleClick(const FSlateHyperlinkRun::FMetadata& Metadata);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	FString GetMetadata(const FString& Key);

	const FSlateHyperlinkRun::FMetadata* Current{ nullptr };
};
