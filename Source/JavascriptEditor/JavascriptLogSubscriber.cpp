#include "JavascriptLogSubscriber.h"

UJavascriptLogSubscriber::UJavascriptLogSubscriber()
{
	GLog->AddOutputDevice(this);
	GLog->SerializeBacklog(this);
}

UJavascriptLogSubscriber::~UJavascriptLogSubscriber()
{
	if (GLog != NULL)
	{
		GLog->RemoveOutputDevice(this);
	}
}

void UJavascriptLogSubscriber::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category)
{
	OnNewLogMessage.Broadcast(FString(V), FString(FOutputDeviceHelper::VerbosityToString(Verbosity)), Category);
}