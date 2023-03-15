#include "JavascriptComponent.h"
#include "JavascriptIsolate.h"
#include "JavascriptContext.h"
#include "JavascriptStats.h"
#include "Engine/World.h"
#include "Engine/Engine.h"


DECLARE_CYCLE_STAT(TEXT("Javascript Component Tick Time"), STAT_JavascriptComponentTickTime, STATGROUP_Javascript);

UJavascriptComponent::UJavascriptComponent(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = false;
	bAutoActivate = true;
	bWantsInitializeComponent = true;
}

void UJavascriptComponent::OnRegister()
{
	auto ContextOwner = GetOuter();
	if (ContextOwner && !HasAnyFlags(RF_ClassDefaultObject) && !ContextOwner->HasAnyFlags(RF_ClassDefaultObject))
	{
		if (GetWorld() && ((GetWorld()->IsGameWorld() && !GetWorld()->IsPreviewWorld()) || bActiveWithinEditor))
		{
			UJavascriptIsolate* Isolate = nullptr;
			if (!IsRunningCommandlet())
			{
				UJavascriptStaticCache* StaticGameData = Cast<UJavascriptStaticCache>(GEngine->GameSingleton);
				if (StaticGameData)
				{
					if (StaticGameData->Isolates.Num() > 0)
						Isolate = StaticGameData->Isolates.Pop();
				}
			}

			if (!Isolate)
			{
				Isolate = NewObject<UJavascriptIsolate>();
				Isolate->Init(false);
				Isolate->AddToRoot();
			}

			auto* Context = Isolate->CreateContext();
			JavascriptContext = Context;
			JavascriptIsolate = Isolate;

			Context->Expose("Root", this);
			Context->Expose("GWorld", GetWorld());
			Context->Expose("GEngine", GEngine);
		}
	}

	Super::OnRegister();
}

void UJavascriptComponent::Activate(bool bReset)
{
	Super::Activate(bReset);

	if (JavascriptContext)
	{
		JavascriptContext->RunFile(*ScriptSourceFile);

		SetComponentTickEnabled(OnTick.IsBound());
	}

	OnBeginPlay.ExecuteIfBound();
}

void UJavascriptComponent::Deactivate()
{
	OnEndPlay.ExecuteIfBound();

	Super::Deactivate();
}

void UJavascriptComponent::BeginDestroy()
{
	if (IsValid(GEngine) && !IsRunningCommandlet())
	{
		auto* StaticGameData = Cast<UJavascriptStaticCache>(GEngine->GameSingleton);
		if (StaticGameData)
		{
			StaticGameData->Isolates.Add(JavascriptIsolate);
		}
		else if (JavascriptIsolate)
		{
			JavascriptIsolate->RemoveFromRoot();
			JavascriptIsolate = nullptr;
			JavascriptContext = nullptr;
		}
	}
	if (IsActive())
	{
		Deactivate();
	}

	Super::BeginDestroy();
}

void UJavascriptComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	check(bRegistered);

	SCOPE_CYCLE_COUNTER(STAT_JavascriptComponentTickTime);

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	OnTick.ExecuteIfBound(DeltaTime);
}

void UJavascriptComponent::ForceGC()
{
	JavascriptContext->RequestV8GarbageCollection();
}

void UJavascriptComponent::Expose(FString ExposedAs, UObject* Object)
{
	JavascriptContext->Expose(ExposedAs, Object);
}

void UJavascriptComponent::Invoke(FName Name)
{
	OnInvoke.ExecuteIfBound(Name);
}

void UJavascriptComponent::ProcessEvent(UFunction* Function, void* Parms)
{
	if (JavascriptContext && JavascriptContext->CallProxyFunction(this, this, Function, Parms))
	{
		return;
	}

	Super::ProcessEvent(Function, Parms);
}

UObject* UJavascriptComponent::ResolveAsset(FName Name, bool bTryLoad)
{
	for (const auto& Item : Assets)
	{
		if (Item.Name == Name)
		{
			return bTryLoad ? Item.Asset.TryLoad() : Item.Asset.ResolveObject();
		}
	}

	return nullptr;
}

UClass* UJavascriptComponent::ResolveClass(FName Name)
{
	for (const auto& Item : ClassAssets)
	{
		if (Item.Name == Name)
		{
			return Item.Class;
		}
	}

	return nullptr;
}

