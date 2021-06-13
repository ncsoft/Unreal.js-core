#include "JavascriptHttpRequest.h"
#include "JavascriptContext.h"
#include "Misc/FileHelper.h"
#include "Misc/Base64.h"
#include "Misc/Paths.h"

#if WITH_EDITOR
#include "TickableEditorObject.h"
typedef FTickableEditorObject FTickableRequest;
#else
#include "Tickable.h"
typedef FTickableGameObject FTickableRequest;
#endif

struct FHttpProcessor : public FTickableRequest
{
public:
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
	FHttpProcessor(TSharedPtr<IHttpRequest> InRef)
#else
	FHttpProcessor(FHttpRequestPtr InRef)
#endif
		: Ref(InRef)
	{}

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 26
	TSharedPtr<IHttpRequest> Ref;
#else
	FHttpRequestPtr Ref = nullptr;
#endif

	virtual void Tick(float DeltaTime) override
	{
		Ref->Tick(DeltaTime);
	}

	virtual bool IsTickable() const override
	{
		return true;
	}

	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(JavascriptHttpRequest, STATGROUP_Tickables);
	}	
};

UJavascriptHttpRequest::UJavascriptHttpRequest(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	if (!IsTemplate(RF_ClassDefaultObject))
	{
		Request = FHttpModule::Get().CreateRequest();
	}
}

void UJavascriptHttpRequest::BeginDestroy()
{
	Super::BeginDestroy();

	if (IsProcessing())
	{
		EndProcessing();
	}

	Request.Reset();
}

void UJavascriptHttpRequest::BeginProcessing()
{
	Processor = new FHttpProcessor(Request);
}

void UJavascriptHttpRequest::EndProcessing()
{
	delete Processor;
	Processor = nullptr;

	Request->OnProcessRequestComplete().Unbind();
	Request->OnRequestProgress().Unbind();
}

FString UJavascriptHttpRequest::GetVerb()
{
	return Request->GetVerb();
}

void UJavascriptHttpRequest::SetVerb(const FString& Verb)
{
	Request->SetVerb(Verb);
}

void UJavascriptHttpRequest::SetURL(const FString& URL)
{
	Request->SetURL(URL);
}

FString GetFileFormat(FString FileExtension)
{
	if (FileExtension == ".png")
		return "image/png";
	else if (FileExtension == ".jpg")
		return "image/jpeg";
	else if (FileExtension == ".gif")
		return "image/gif";
	else if (FileExtension == ".json")
		return "application/json";
	else
		return "application/octet-stream";
}

void UJavascriptHttpRequest::SetContentWithFiles(TArray<FString> FilePaths, FString Boundary, FString Content)
{
	TArray<uint8> Payload;

	auto MakePayloadFromString = [&](FString _c)
	{
		auto Index = Payload.Num();
		FTCHARToUTF8 Converter(*_c);
		Payload.SetNumUninitialized(Payload.Num() + Converter.Length());
		FMemory::Memcpy(Payload.GetData() + Index, (const uint8*)Converter.Get(), Converter.Length());
	};

	auto MakePayloadFromBinary = [&](const TArray<uint8>& _d)
	{
		auto Index = Payload.Num();
		Payload.SetNumUninitialized(Payload.Num() + _d.Num());
		FMemory::Memcpy(Payload.GetData() + Index, _d.GetData(), _d.Num());
	};

	MakePayloadFromString(Content);

	FString PrefixBoundry = "\r\n--" + Boundary + "\r\n";
	FString SuffixBoundary = "\r\n--" + Boundary + "--\r\n";
	for (auto FilePath : FilePaths)
	{
		TArray<uint8> FileRawData;
		auto FileFormat = GetFileFormat(FPaths::GetExtension(FilePath, true));
		if (FFileHelper::LoadFileToArray(FileRawData, *FilePath))
		{
			FString FileHeader = "Content-Disposition: form-data; name=\"file\"; filename=\"" + FilePath + "\"\r\nContent-Type: " + FileFormat + "\r\n\r\n";
			MakePayloadFromString(PrefixBoundry);
			MakePayloadFromString(FileHeader);
			MakePayloadFromBinary(FileRawData);
		}
	}
	
	MakePayloadFromString(SuffixBoundary);

	Request->SetContent(Payload);
}

void UJavascriptHttpRequest::SetContentFromMemory()
{
	TArray<uint8> Payload;
	Payload.Append((uint8*)FArrayBufferAccessor::GetData(), FArrayBufferAccessor::GetSize());
	Request->SetContent(Payload);
}

void UJavascriptHttpRequest::SetContentAsString(const FString& ContentString)
{
	Request->SetContentAsString(ContentString);
}

void UJavascriptHttpRequest::SetHeader(const FString& HeaderName, const FString& HeaderValue)
{
	Request->SetHeader(HeaderName, HeaderValue);
}

bool UJavascriptHttpRequest::ProcessRequest()
{
	if (IsProcessing()) return false;

	Request->OnProcessRequestComplete().BindLambda([&](FHttpRequestPtr, FHttpResponsePtr, bool status){
		OnComplete.ExecuteIfBound(status);
		EndProcessing();
	});

	Request->OnRequestProgress().BindLambda([&](FHttpRequestPtr, int32 sent, int32 recv){
		OnProgress.ExecuteIfBound(sent,recv);
	});
	
	if (Request->ProcessRequest())
	{
		BeginProcessing();
		return true;
	}
	else
	{
		return false;
	}
}

void UJavascriptHttpRequest::CancelRequest()
{
	Request->CancelRequest();
}

EJavascriptHttpRequestStatus::Type UJavascriptHttpRequest::GetStatus()
{
	return EJavascriptHttpRequestStatus::Type(Request->GetStatus());
}

int32 UJavascriptHttpRequest::GetResponseCode()
{
	auto res = Request->GetResponse();
	if (!res.IsValid()) return 0;

	return Request->GetResponse()->GetResponseCode();
}

FString UJavascriptHttpRequest::GetContentAsString()
{
	auto res = Request->GetResponse();
	if (!res.IsValid()) return TEXT("");

	return Request->GetResponse()->GetContentAsString();
}

int32 UJavascriptHttpRequest::GetContentLength()
{
	auto res = Request->GetResponse();
	if (!res.IsValid()) return 0;

	return Request->GetResponse()->GetContent().Num();
}

void UJavascriptHttpRequest::GetContentToMemory()
{
	auto res = Request->GetResponse();
	if (!res.IsValid()) return;

	const auto& Content = Request->GetResponse()->GetContent();

	if (FArrayBufferAccessor::GetSize() >= Content.Num())
	{
		FMemory::Memcpy(FArrayBufferAccessor::GetData(), Content.GetData(), Content.Num());
	}
}

float UJavascriptHttpRequest::GetElapsedTime()
{
	return Request->GetElapsedTime();
}