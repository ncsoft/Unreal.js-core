#include "JavascriptOutputDevice.h"
#include "UObject/UObjectThreadContext.h"

/** This class is to capture all log output even if the log window is closed */
class FJavascriptOutputDevice : public FOutputDevice, public FTickableGameObject
{
public:
	UJavascriptOutputDevice* OutputDevice;

	FJavascriptOutputDevice(UJavascriptOutputDevice* InOutputDevice)
	{
		OutputDevice = InOutputDevice;

		GLog->AddOutputDevice(this);
		GLog->SerializeBacklog(this);
	}

	~FJavascriptOutputDevice()
	{
		// At shutdown, GLog may already be null
		if (GLog != nullptr)
		{
			GLog->RemoveOutputDevice(this);
		}
	}

	virtual void Tick(float DeltaTime) override
	{
		SubmitPendingMessages();
	}

	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FJavascriptOutputDevice, STATGROUP_Tickables);
	}
	
protected:
	struct FLogMessage
	{
		TSharedRef<FString> Message;
		ELogVerbosity::Type Verbosity;
		FName Category;

		FLogMessage(const TSharedRef<FString>& NewMessage, ELogVerbosity::Type NewVerbosity, FName NewCategory)
			: Message(NewMessage)
			, Verbosity(NewVerbosity)
			, Category(NewCategory)
		{
		}
	};

	virtual void Serialize(const TCHAR* V, ELogVerbosity::Type Verbosity, const class FName& Category) override
	{		
		FScopeLock PendingMessagesAccess(&PendingMessagesCriticalSection);
		PendingMessages.Add(MakeShared<FLogMessage>(MakeShared<FString>(V), Verbosity, Category));
	}

	void SubmitPendingMessages()
	{
		TArray<TSharedPtr<FLogMessage>> Messages;

		if (PendingMessagesCriticalSection.TryLock())
		{
			Messages = MoveTemp(PendingMessages);
			PendingMessages.Reset();
			PendingMessagesCriticalSection.Unlock();
		}
		else
		{
			return;
		}

		for (const auto& Message : Messages)
		{
			SubmitPendingMessage(Message->Message, Message->Verbosity, Message->Category);
		}
	}

	void SubmitPendingMessage(TSharedPtr<FString> Message, ELogVerbosity::Type Verbosity, const class FName& Category)
	{
		static bool bIsReentrant = false;
		if (bIsReentrant) return;

		TGuardValue<bool> ReentrantGuard(bIsReentrant, true);
		if (!OutputDevice->IsUnreachable() && !FUObjectThreadContext::Get().IsRoutingPostLoad)
		{
			OutputDevice->OnMessage(*Message, (ELogVerbosity_JS)Verbosity, Category);
		}
	}

private:
	TArray<TSharedPtr<FLogMessage>> PendingMessages;
	FCriticalSection PendingMessagesCriticalSection;
};

UJavascriptOutputDevice::UJavascriptOutputDevice(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		OutputDevice = MakeShareable(new FJavascriptOutputDevice(this));
	}	
}

void UJavascriptOutputDevice::BeginDestroy()
{
	Super::BeginDestroy();

	OutputDevice.Reset();
}

void UJavascriptOutputDevice::Kill()
{
	OutputDevice.Reset();	
}

void UJavascriptOutputDevice::Log(FName Category, ELogVerbosity_JS _Verbosity, const FString& Filename, int32 LineNumber, const FString& Message)
{
#if NO_LOGGING
	
#else
	auto Verbosity = (ELogVerbosity::Type)_Verbosity;	
	FMsg::Logf_Internal(TCHAR_TO_ANSI(*Filename), LineNumber, Category, Verbosity, TEXT("%s"), *Message);
	if (Verbosity == ELogVerbosity::Fatal)
	{
#ifdef UE_DEBUG_BREAK_AND_PROMPT_FOR_REMOTE
		UE_DEBUG_BREAK_AND_PROMPT_FOR_REMOTE();
#else
		_DebugBreakAndPromptForRemote();
#endif
		FDebug::AssertFailed("", TCHAR_TO_ANSI(*Filename), LineNumber, TEXT("%s"), *Message);
	}
#endif
}
