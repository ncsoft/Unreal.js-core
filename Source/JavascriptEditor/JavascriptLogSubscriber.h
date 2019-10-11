#pragma once

#include "Misc/OutputDeviceHelper.h"
#include "JavascriptLogSubscriber.generated.h"

UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptLogSubscriber : public UObject, public FOutputDevice
{
	GENERATED_BODY()

public:

	UJavascriptLogSubscriber();
	~UJavascriptLogSubscriber();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnNewLogMessage, FString, Message, FString, Type, const FName&, Category);

	UPROPERTY(EditAnywhere, Category = LogMessages, meta = (IsBindableEvent = "True"))
	FOnNewLogMessage OnNewLogMessage;

	using UObject::Serialize;
	virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const FName& Category) override;
};