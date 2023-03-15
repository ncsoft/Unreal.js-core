#include "JavascriptLogSubscriber.h"
#include "Logging/LogVerbosity.h"

UJavascriptLogSubscriber::UJavascriptLogSubscriber()
{
	GLog->AddOutputDevice(this);
	GLog->SerializeBacklog(this);
}

UJavascriptLogSubscriber::~UJavascriptLogSubscriber()
{
	if (GLog != nullptr)
	{
		GLog->RemoveOutputDevice(this);
	}
}

void UJavascriptLogSubscriber::Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category)
{
	OnNewLogMessage.Broadcast(FString(V), FString(::ToString(Verbosity)), Category);
}