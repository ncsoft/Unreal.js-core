#pragma once

#include "Misc/OutputDeviceHelper.h"
#include "JavascriptLogSubscriber.generated.h"

UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptLogSubscriber : public UObject, public FOutputDevice
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnNewLogMessage, FString, Message, FString, Type, const FName&, Category);

	UPROPERTY(EditAnywhere, Category = LogMessages, meta = (IsBindableEvent = "True"))
	FOnNewLogMessage OnNewLogMessage;

	UJavascriptLogSubscriber()
	{
		GLog->AddOutputDevice(this);
		GLog->SerializeBacklog(this);
	}

	~UJavascriptLogSubscriber()
	{
		if (GLog != NULL)
		{
			GLog->RemoveOutputDevice(this);
		}
	}

protected:

	virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category) override
	{
		OnNewLogMessage.Broadcast(FString(V), FString(FOutputDeviceHelper::VerbosityToString(Verbosity)), Category);
	}
};