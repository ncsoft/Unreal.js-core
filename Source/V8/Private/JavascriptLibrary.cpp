#include "JavascriptLibrary.h"
#include "Engine/DynamicBlueprintBinding.h"
#include "JavascriptContext.h"
#include "IV8.h"
#include "SocketSubsystem.h"
#include "GameFramework/GameMode.h"
#include "Sockets.h"
#include "EngineUtils.h"
#include "AI/Navigation//NavigationSystem.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Modules/ModuleVersion.h"

struct FPrivateSocketHandle
{
	FPrivateSocketHandle(ISocketSubsystem* InSocketSub, FSocket* InSocket)
		: SocketSub(InSocketSub), Socket(InSocket)
	{}

	~FPrivateSocketHandle()
	{
		SocketSub->DestroySocket(Socket);
	}

	ISocketSubsystem* SocketSub;
	FSocket* Socket;
};

FJavascriptSocket UJavascriptLibrary::CreateSocket(FName SocketType, FString Description, bool bForceUDP)
{
	auto SocketSub = ISocketSubsystem::Get();
	return{ MakeShared<FPrivateSocketHandle>(SocketSub, SocketSub->CreateSocket(SocketType, Description, bForceUDP)) };
}

FJavascriptInternetAddr UJavascriptLibrary::CreateInternetAddr()
{
	auto SocketSub = ISocketSubsystem::Get();
	return{ SocketSub->CreateInternetAddr() };
}

bool UJavascriptLibrary::ResolveIp(FString HostName, FString& OutIp)
{
	auto SocketSub = ISocketSubsystem::Get();
	TSharedRef<FInternetAddr> HostAddr = SocketSub->CreateInternetAddr();
	ESocketErrors HostResolveError = SocketSub->GetHostByName(TCHAR_TO_ANSI(*HostName), *HostAddr);
	if (HostResolveError == SE_NO_ERROR || HostResolveError == SE_EWOULDBLOCK)
	{
		OutIp = HostAddr->ToString(false);
		return true;
	}
	return false;
}

void UJavascriptLibrary::SetIp(FJavascriptInternetAddr& Addr, FString ResolvedAddress, bool& bValid)
{
	Addr.Handle->SetIp(*ResolvedAddress, bValid);
}

void UJavascriptLibrary::SetPort(FJavascriptInternetAddr& Addr, int32 Port)
{
	Addr.Handle->SetPort(Port);
}

bool UJavascriptLibrary::SendMemoryTo(FJavascriptSocket& Socket, const FJavascriptInternetAddr& ToAddr, int32 NumBytes, int32& BytesSent)
{
	auto Buffer = FArrayBufferAccessor::GetData();
	auto Size = FArrayBufferAccessor::GetSize();

	if (NumBytes > Size) return false;
	if (!Socket.Handle.IsValid()) return false;
	if (!ToAddr.Handle.IsValid()) return false;

	return Socket.Handle->Socket->SendTo(reinterpret_cast<const uint8*>(Buffer), NumBytes, BytesSent, *ToAddr.Handle);
}

void UJavascriptLibrary::SetMobile(USceneComponent* SceneComponent)
{
	if (SceneComponent)
	{
		SceneComponent->SetMobility(EComponentMobility::Movable);
	}
}

void UJavascriptLibrary::SetMobility(USceneComponent* SceneComponent, EComponentMobility::Type Type)
{
	if (SceneComponent)
	{
		SceneComponent->SetMobility(Type);
	}
}

UWorld* UJavascriptLibrary::Actor_GetWorld(AActor* Actor)
{
	return Actor ? Actor->GetWorld() : nullptr;
}

UClass* UJavascriptLibrary::GetBlueprintGeneratedClass(UBlueprint* Blueprint)
{
	UE_LOG(Javascript, Warning, TEXT("GetBlueprintGeneratedClass will be deprecated : Use instead Blueprint.GeneratedClass"));
	return Blueprint ? Blueprint->GeneratedClass : nullptr;
}

UClass* UJavascriptLibrary::GetBlueprintGeneratedClassFromPath(FString Path)
{
	UE_LOG(Javascript, Warning, TEXT("GetBlueprintGeneratedClassFromPath will be deprecated : Use instead Blueprint.Load(Path).GeneratedClass"));
	return GetBlueprintGeneratedClass(Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, *Path)));
}

UObject* UJavascriptLibrary::GetOuter(UObject* Object)
{
	return Object ? Object->GetOuter() : nullptr;
}

UObject* UJavascriptLibrary::GetOutermost(UObject* Object)
{
	return Object ? Object->GetOutermost() : nullptr;
}

UObject* UJavascriptLibrary::Duplicate(UObject* Object, UObject* Outer, FName Name)
{
	return DuplicateObject<UObject>(Object, Outer, *Name.ToString());
}

bool UJavascriptLibrary::HasAnyFlags(UObject* Object, int32 Flags)
{
	return Object && Object->HasAnyFlags((EObjectFlags)Flags);
}

bool UJavascriptLibrary::HasAnyPackageFlags(UPackage* Package, int32 Flags)
{
	return Package && Package->HasAnyPackageFlags((EPackageFlags)Flags);
}

FString UJavascriptLibrary::GetName(UObject* Object)
{
	return Object ? Object->GetName() : TEXT("");
}

bool UJavascriptLibrary::IsPlayInEditor(UWorld* World)
{
	return World && World->IsPlayInEditor();
}

bool UJavascriptLibrary::IsPlayInPreview(UWorld* World)
{
	return World && World->IsPlayInPreview();
}

bool UJavascriptLibrary::IsGameWorld(UWorld* World)
{
	return World && World->IsGameWorld();
}

void UJavascriptLibrary::SetClientTravel(UEngine* Engine, UWorld *InWorld, FString NextURL, ETravelType InTravelType)
{
	if (Engine)
	{
		Engine->SetClientTravel(InWorld, *NextURL, InTravelType);
	}
}

UPackage* UJavascriptLibrary::CreatePackage(UObject* Outer, FString PackageName)
{
	return ::CreatePackage(Outer, *PackageName);
}

UPackage* UJavascriptLibrary::FindPackage(UObject* InOuter, FString PackageName)
{
	return ::FindPackage(InOuter, *PackageName);
}

UPackage* UJavascriptLibrary::LoadPackage(UPackage* InOuter, FString PackageName)
{
	return ::LoadPackage(InOuter, *PackageName, LOAD_None);
}


void UJavascriptLibrary::AddDynamicBinding(UClass* Outer, UDynamicBlueprintBinding* BindingObject)
{
	if (Cast<UBlueprintGeneratedClass>(Outer) && BindingObject)
	{
		Cast<UBlueprintGeneratedClass>(Outer)->DynamicBindingObjects.Add(BindingObject);
	}	
}

UDynamicBlueprintBinding* UJavascriptLibrary::GetDynamicBinding(UClass* Outer, TSubclassOf<UDynamicBlueprintBinding> BindingObjectClass)
{
	if (auto bpg = Cast<UBlueprintGeneratedClass>(Outer))
	{
		for (auto BindingObject : bpg->DynamicBindingObjects)
		{
			if (BindingObject->IsA(BindingObjectClass))
			{
				return BindingObject;
			}
		}
	}

	return nullptr;
}

void UJavascriptLibrary::HandleSeamlessTravelPlayer(AGameModeBase* GameMode, AController*& C)
{
	GameMode->HandleSeamlessTravelPlayer(C);
}

void UJavascriptLibrary::SetRootComponent(AActor* Actor, USceneComponent* Component)
{
	Actor->SetRootComponent(Component);
}

int32 UJavascriptLibrary::GetFileSize(UObject* Object, FString Filename)
{
	auto size = IFileManager::Get().FileSize(*Filename);
	if (size > INT_MAX) return -1;
	return (int32)size;
}

bool UJavascriptLibrary::ReadFile(UObject* Object, FString Filename)
{
	FArchive* Reader = IFileManager::Get().CreateFileReader(*Filename);
	if (!Reader)
	{
		return false;
	}

	int32 Size = Reader->TotalSize();
	if (Size != FArrayBufferAccessor::GetSize())
	{
		return false;
	}

	Reader->Serialize(FArrayBufferAccessor::GetData(), Size);
	return Reader->Close();	
}

bool UJavascriptLibrary::WriteFile(UObject* Object, FString Filename)
{
	FArchive* Writer = IFileManager::Get().CreateFileWriter(*Filename);
	if (!Writer)
	{
		return false;
	}
		
	Writer->Serialize(FArrayBufferAccessor::GetData(), FArrayBufferAccessor::GetSize());
	return Writer->Close();
}

FString UJavascriptLibrary::ReadStringFromFile(UObject* Object, FString Filename)
{
	FString Result;
	FFileHelper::LoadFileToString(Result, *Filename);
	return Result;
}

bool UJavascriptLibrary::WriteStringToFile(UObject* Object, FString Filename, const FString& Data)
{
	return FFileHelper::SaveStringToFile(*Data, *Filename, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}

FString UJavascriptLibrary::GetDir(UObject* Object, FString WhichDir)
{
	if (WhichDir == TEXT("Launch")) return FPaths::LaunchDir();
	else if (WhichDir == TEXT("Engine")) return FPaths::EngineDir();
	else if (WhichDir == TEXT("EngineUser")) return FPaths::EngineUserDir();
	else if (WhichDir == TEXT("EngineVersionAgnosticUser")) return FPaths::EngineVersionAgnosticUserDir();
	else if (WhichDir == TEXT("EngineContent")) return FPaths::EngineContentDir();
	else if (WhichDir == TEXT("EngineConfig")) return FPaths::EngineConfigDir();
	else if (WhichDir == TEXT("EngineIntermediate")) return FPaths::EngineIntermediateDir();
	else if (WhichDir == TEXT("EngineSaved")) return FPaths::EngineSavedDir();
	else if (WhichDir == TEXT("EnginePlugins")) return FPaths::EnginePluginsDir();
	else if (WhichDir == TEXT("Root")) return FPaths::RootDir();
	else if (WhichDir == TEXT("Game")) return FPaths::ProjectDir();
	else if (WhichDir == TEXT("GameUser")) return FPaths::ProjectUserDir();
	else if (WhichDir == TEXT("GameContent")) return FPaths::ProjectContentDir();
	else if (WhichDir == TEXT("GameConfig")) return FPaths::ProjectConfigDir();
	else if (WhichDir == TEXT("GameSaved")) return FPaths::ProjectSavedDir();
	else if (WhichDir == TEXT("GameIntermediate")) return FPaths::ProjectIntermediateDir();
	else if (WhichDir == TEXT("GamePlugins")) return FPaths::ProjectPluginsDir();
	else if (WhichDir == TEXT("SourceConfig")) return FPaths::SourceConfigDir();
	else if (WhichDir == TEXT("GeneratedConfig")) return FPaths::GeneratedConfigDir();
	else if (WhichDir == TEXT("Sandboxes")) return FPaths::SandboxesDir();
	else if (WhichDir == TEXT("Profiling")) return FPaths::ProfilingDir();
	else if (WhichDir == TEXT("ScreenShot")) return FPaths::ScreenShotDir();
	else if (WhichDir == TEXT("BugIt")) return FPaths::BugItDir();
	else if (WhichDir == TEXT("VideoCapture")) return FPaths::VideoCaptureDir();
	else if (WhichDir == TEXT("GameLog")) return FPaths::ProjectLogDir();
	else if (WhichDir == TEXT("Automation")) return FPaths::AutomationDir();
	else if (WhichDir == TEXT("AutomationTransient")) return FPaths::AutomationTransientDir();
	else if (WhichDir == TEXT("AutomationLog")) return FPaths::AutomationLogDir();
	else if (WhichDir == TEXT("Cloud")) return FPaths::CloudDir();
	else if (WhichDir == TEXT("GameDevelopers")) return FPaths::GameDevelopersDir();
	else if (WhichDir == TEXT("GameUserDeveloper")) return FPaths::GameUserDeveloperDir();
	else if (WhichDir == TEXT("Diff")) return FPaths::DiffDir();
	else return TEXT("");
}

bool UJavascriptLibrary::HasUndo(UEngine* Engine)
{
	return !!GUndo;
}

bool UJavascriptLibrary::ReadDirectory(UObject* Object, FString Directory, TArray<FDirectoryItem>& OutItems)
{
	struct FLocalVisitor : IPlatformFile::FDirectoryVisitor
	{
		FLocalVisitor(TArray<FDirectoryItem>& InResult)
			: Result(InResult)
		{}

		TArray<FDirectoryItem>& Result;

		virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
		{
			auto Item = new(Result)FDirectoryItem;
			Item->Name = FilenameOrDirectory;
			Item->bIsDirectory = bIsDirectory;
			return true;
		}
	} Visitor(OutItems);

	return IPlatformFile::GetPlatformPhysical().IterateDirectory(*Directory, Visitor);	
}

void UJavascriptLibrary::ReregisterComponent(UActorComponent* ActorComponent)
{
	ActorComponent->ReregisterComponent();
}

void UJavascriptLibrary::RegisterComponent(UActorComponent* ActorComponent)
{
	ActorComponent->RegisterComponent();
}

void UJavascriptLibrary::UnregisterComponent(UActorComponent* ActorComponent)
{
	ActorComponent->UnregisterComponent();
}

bool UJavascriptLibrary::IsRegistered(UActorComponent* ActorComponent)
{
	return ActorComponent->IsRegistered();
}

void UJavascriptLibrary::GetAllActorsOfClassAndTags(UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, const TArray<FName>& Tags_Accept, const TArray<FName>& Tags_Deny, TArray<AActor*>& OutActors)
{
	OutActors.Empty();

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);

	// We do nothing if not class provided, rather than giving ALL actors!
	if (ActorClass != NULL && World != nullptr)
	{
		for (TActorIterator<AActor> It(World, ActorClass); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor->IsPendingKill())
			{
				bool bReject{ false };
				bool bAccept{ false };
				for (const auto& Tag : Actor->Tags)
				{
					if (Tags_Deny.Contains(Tag))
					{
						bReject = true;
						break;
					}
					if (!bAccept && Tags_Accept.Contains(Tag))
					{
						bAccept = true;
					}
				}
				if (bAccept && !bReject)
				{
					OutActors.Add(Actor);
				}				
			}
		}
	}
}

int32 UJavascriptLibrary::GetCurrentProcessId()
{
	return FPlatformProcess::GetCurrentProcessId();
}

UModel* UJavascriptLibrary::GetModel(UWorld* World)
{
	return World ? World->GetModel() : nullptr;
}

ULevel* UJavascriptLibrary::GetLevel(AActor* Actor)
{
	return Actor ? Actor->GetLevel() : nullptr;
}

TArray<ULevel*> UJavascriptLibrary::GetLevels(UWorld* World)
{
	return World->GetLevels();
}

FString UJavascriptLibrary::GetArchetypePathName(UObject* Object)
{
	return Object->GetArchetype()->GetPathName();
}

FString UJavascriptLibrary::GetClassPathName(UClass* Class)
{
	return Class->GetPathName();
}

void UJavascriptLibrary::SetObjectFlags(UObject* Obj, int32 Flags)
{
	Obj->SetFlags((EObjectFlags)Flags);
}

void UJavascriptLibrary::SetActorFlags(AActor* Actor, int32 Flags)
{
	TArray<AActor*> TargetActors;
	TargetActors.Push(Actor);
	while (TargetActors.Num() > 0)
	{
		auto TargetActor = TargetActors.Pop();
		TargetActor->SetFlags(RF_Transient);

		TArray<UActorComponent*> OutComponents;
		TargetActor->GetComponents(OutComponents, true);
		for (const auto& Component : OutComponents)
		{
			Component->SetFlags(RF_Transient);
		}

		TargetActor->GetAllChildActors(TargetActors, true);
	}
}

float UJavascriptLibrary::GetLastRenderTime(AActor* Actor)
{
	return Actor->GetLastRenderTime();
}

UEnum* UJavascriptLibrary::CreateEnum(UObject* Outer, FName Name, TArray<FName> DisplayNames)
{
	UEnum* Enum = NewObject<UEnum>(Outer,Name,RF_Public);

	if (NULL != Enum)
	{
#if ENGINE_MINOR_VERSION > 14
		TArray<TPair<FName, int64>> Names;
#else
		TArray<TPair<FName, uint8>> Names;
#endif

		int32 Index = 0;

		for (auto DisplayName : DisplayNames)
		{
			Names.Add(TPairInitializer<FName, uint8>(DisplayName, Index));
			Index++;
		}
		Enum->SetEnums(Names, UEnum::ECppForm::Namespaced);
	}

	return Enum;
}

void UJavascriptLibrary::MarkRenderStateDirty(UActorComponent* Component)
{
	Component->MarkRenderStateDirty();
}

void UJavascriptLibrary::ReregisterAllComponents(AActor* Actor)
{
	Actor->ReregisterAllComponents();
}

bool UJavascriptLibrary::SegmentIntersection2D(const FVector& SegmentStartA, const FVector& SegmentEndA, const FVector& SegmentStartB, const FVector& SegmentEndB, FVector& IntersectionPoint)
{
	return FMath::SegmentIntersection2D(SegmentStartA, SegmentEndA, SegmentStartB, SegmentEndB, IntersectionPoint);
}

bool UJavascriptLibrary::FileExists(const FString& Filename)
{
	return IFileManager::Get().FileExists(*Filename);
}

bool UJavascriptLibrary::DeleteFile(const FString& Filename, bool ReadOnly)
{
    return IFileManager::Get().Delete(*Filename, false, ReadOnly);
}

bool UJavascriptLibrary::DirectoryExists(const FString& InDirectory)
{
	return IFileManager::Get().DirectoryExists(*InDirectory);
}

bool UJavascriptLibrary::MakeDirectory(const FString& Path, bool Tree)
{
	return IFileManager::Get().MakeDirectory(*Path, Tree);
}

bool UJavascriptLibrary::DeleteDirectory(const FString& Path, bool RequireExists, bool Tree)
{
	return IFileManager::Get().DeleteDirectory(*Path, RequireExists, Tree);
}

void UJavascriptLibrary::GetObjectsWithOuter(const class UObject* Outer, TArray<UObject *>& Results, bool bIncludeNestedObjects, int32 ExclusionFlags, int32 ExclusionInternalFlags)
{
	::GetObjectsWithOuter(Outer, Results, bIncludeNestedObjects, (EObjectFlags)ExclusionFlags, (EInternalObjectFlags)ExclusionInternalFlags);
}

class UObject* UJavascriptLibrary::FindObjectWithOuter(class UObject* Outer, class UClass* ClassToLookFor, FName NameToLookFor)
{
	return static_cast<UObject*>(::FindObjectWithOuter(Outer, ClassToLookFor, NameToLookFor));
}

void UJavascriptLibrary::GetObjectsOfClass(UClass* ClassToLookFor, TArray<UObject *>& Results, bool bIncludeDerivedClasses, int32 ExcludeFlags, int32 ExclusionInternalFlags)
{
	::GetObjectsOfClass(ClassToLookFor, Results, bIncludeDerivedClasses, (EObjectFlags)ExcludeFlags, (EInternalObjectFlags)ExclusionInternalFlags);
}

void UJavascriptLibrary::GetDerivedClasses(UClass* ClassToLookFor, TArray<UClass *>& Results, bool bRecursive)
{
	::GetDerivedClasses(ClassToLookFor, Results, bRecursive);
}

FString UJavascriptLibrary::GetPlatformName()
{
	return FPlatformProperties::PlatformName();
}

FJavascriptLogCategory UJavascriptLibrary::CreateLogCategory(const FString& CategoryName, ELogVerbosity_JS InDefaultVerbosity)
{
#if NO_LOGGING
	return FJavascriptLogCategory();
#else
	return { MakeShareable<FLogCategoryBase>(new FLogCategoryBase(*CategoryName, (ELogVerbosity::Type)InDefaultVerbosity, ELogVerbosity::All )) };
#endif
}

void UJavascriptLibrary::Log(const FJavascriptLogCategory& Category, ELogVerbosity_JS _Verbosity, const FString& Message, const FString& FileName, int32 LineNumber)
{
#if NO_LOGGING
#else
	auto Verbosity = (ELogVerbosity::Type)_Verbosity;

	if (!Category.Handle->IsSuppressed(Verbosity))
	{
		FMsg::Logf_Internal(TCHAR_TO_ANSI(*FileName), LineNumber, Category.Handle->GetCategoryName(), Verbosity, *Message);
		if (Verbosity == ELogVerbosity::Fatal) 
		{
			_DebugBreakAndPromptForRemote();
			FDebug::AssertFailed("", TCHAR_TO_ANSI(*FileName), LineNumber, *Message);
		}
	}
#endif
}

bool UJavascriptLibrary::IsSuppressed(const FJavascriptLogCategory& Category, ELogVerbosity_JS _Verbosity)
{
#if NO_LOGGING
	return true;
#else
	auto Verbosity = (ELogVerbosity::Type)_Verbosity;
	return Category.Handle->IsSuppressed(Verbosity);
#endif
}

FName UJavascriptLibrary::GetCategoryName(const FJavascriptLogCategory& Category)
{
#if NO_LOGGING
	return FName();
#else
	return Category.Handle->GetCategoryName();
#endif
}

FJavascriptStreamableManager UJavascriptLibrary::CreateStreamableManager()
{
	return{ MakeShareable<FStreamableManager>(new FStreamableManager) };
}

void UJavascriptLibrary::SimpleAsyncLoad(const FJavascriptStreamableManager& Manager, FStringAssetReference const& Target, int32 Priority)
{
	Manager->RequestAsyncLoad(Target, FStreamableDelegate(), Priority, true);
}

void UJavascriptLibrary::Unload(const FJavascriptStreamableManager& Manager, FStringAssetReference const& Target)
{
	Manager->Unload(Target);
}

bool UJavascriptLibrary::IsAsyncLoadComplete(const FJavascriptStreamableManager& Manager, FStringAssetReference const& Target)
{
	return Manager->IsAsyncLoadComplete(Target);
}

void UJavascriptLibrary::RequestAsyncLoad(const FJavascriptStreamableManager& Manager, const TArray<FStringAssetReference>& TargetsToStream, FJavascriptFunction DelegateToCall, int32 Priority)
{
	auto Copy = new FJavascriptFunction;
	*Copy = DelegateToCall;
	Manager->RequestAsyncLoad(TargetsToStream, [=]() {
		Copy->Execute();
		delete Copy;
	}, Priority);
}

void UJavascriptLibrary::V8_SetFlagsFromString(const FString& V8Flags)
{
	IV8::Get().SetFlagsFromString(V8Flags);
}

void UJavascriptLibrary::V8_SetIdleTaskBudget(float BudgetInSeconds)
{
	IV8::Get().SetIdleTaskBudget(BudgetInSeconds);
}

UObject* UJavascriptLibrary::TryLoadByPath(FString Path)
{
	return FStringAssetReference(*Path).TryLoad();
}

void UJavascriptLibrary::GenerateNavigation(UWorld* world, ARecastNavMesh* NavData )
{
	UNavigationSystem::InitializeForWorld(world, FNavigationSystemRunMode::PIEMode);
	NavData->RebuildAll();
}

const FString& UJavascriptLibrary::GetMetaData(UField* Field, const FString Key)
{
	UPackage* Package = Field->GetOutermost();
	check(Package);

	UMetaData* MetaData = Package->GetMetaData();
	check(MetaData);

	const FString& MetaDataString = MetaData->GetValue(Field, *Key);

	return MetaDataString;
}

TArray<UField*> UJavascriptLibrary::GetFields(const UObject* Object, bool bIncludeSuper)
{
	auto Class = Object->GetClass();
	TArray<UField*> Fields;
	for (TFieldIterator<UField> FieldIt(Class, bIncludeSuper ? EFieldIteratorFlags::IncludeSuper : EFieldIteratorFlags::ExcludeSuper); FieldIt; ++FieldIt)
	{
		Fields.Add(*FieldIt);
	}
	return Fields;
}

TArray<FJavscriptProperty> UJavascriptLibrary::GetStructProperties(const FString StructName, bool bIncludeSuper)
{
	TArray<FJavscriptProperty> Properties;

	UStruct* Struct = FindObjectFast<UStruct>(NULL, *StructName, false, true);
	if (Struct != nullptr)
	{
		// Make sure each field gets allocated into the array
		for (TFieldIterator<UField> FieldIt(Struct, bIncludeSuper ? EFieldIteratorFlags::IncludeSuper : EFieldIteratorFlags::ExcludeSuper); FieldIt; ++FieldIt)
		{
			UField* Field = *FieldIt;

			// Make sure functions also do their parameters and children first
			if (UProperty* Property = dynamic_cast<UProperty*>(Field))
			{
				FJavscriptProperty JavascriptProperty;
				
				FString Type = Property->GetCPPType();
				if (auto p = Cast<UArrayProperty>(Property))
				{
					Type += TEXT("/") + p->Inner->GetCPPType();
				}
				JavascriptProperty.Type = Type;
				JavascriptProperty.Name = Property->GetName();
                
				Properties.Add(JavascriptProperty);
			}
		}
	}
    return Properties;
}

int32 UJavascriptLibrary::GetFunctionParmsSize(UFunction* Function)
{
	return Function->ParmsSize;
}

void UJavascriptLibrary::ClipboardCopy(const FString& String)
{
	FPlatformApplicationMisc::ClipboardCopy(*String);
}

FString UJavascriptLibrary::ClipboardPaste()
{
	FString OutString;
	FPlatformApplicationMisc::ClipboardPaste(OutString);
	return OutString;
}

FJavascriptStat UJavascriptLibrary::NewStat(
	FName InStatName,
	const FString& InStatDesc,
	FName InGroupName,
	FName InGroupCategory,
	const FString& InGroupDesc,
	bool bDefaultEnable,
	bool bShouldClearEveryFrame,
	EJavascriptStatDataType InStatType,
	bool bCycleStat)
{
	FJavascriptStat Out;
#if STATS
	Out.Instance = MakeShareable(new FJavascriptThreadSafeStaticStatBase);
	Out.Instance->DoSetup(
		InStatName.GetPlainANSIString(),
		*InStatDesc, 
		InGroupName.GetPlainANSIString(),
		InGroupCategory.GetPlainANSIString(),
		*InGroupDesc, 
		bDefaultEnable, 
		bShouldClearEveryFrame, 
		(EStatDataType::Type)InStatType, 
		bCycleStat, 
		FPlatformMemory::EMemoryCounterRegion::MCR_Invalid);
#endif

	return Out;
}

void UJavascriptLibrary::AddMessage(FJavascriptStat Stat, EJavascriptStatOperation InStatOperation)
{
#if STATS
	FThreadStats::AddMessage(Stat.Instance->GetStatId().GetName(), (EStatOperation::Type)InStatOperation);
#endif
}

void UJavascriptLibrary::AddMessage_int(FJavascriptStat Stat, EJavascriptStatOperation InStatOperation, int Value, bool bIsCycle)
{
#if STATS
	if (!Stat.Instance.IsValid()) return;

	FThreadStats::AddMessage(Stat.Instance->GetStatId().GetName(), (EStatOperation::Type)InStatOperation, (int64)Value, bIsCycle);
#endif
}

void UJavascriptLibrary::AddMessage_float(FJavascriptStat Stat, EJavascriptStatOperation InStatOperation, float Value, bool bIsCycle)
{
#if STATS
	if (!Stat.Instance.IsValid()) return;

	FThreadStats::AddMessage(Stat.Instance->GetStatId().GetName(), (EStatOperation::Type)InStatOperation, (double)Value, bIsCycle);
#endif
}

TArray<UClass*> UJavascriptLibrary::GetSuperClasses(UClass* InClass)
{
	TArray<UClass*> Classes;
	auto Klass = InClass;
	while (auto Cls = Klass->GetSuperClass())
	{
		Classes.Add(Cls);
		Klass = Cls;
	}

	return Classes;
}

bool UJavascriptLibrary::IsGeneratedByBlueprint(UClass* InClass)
{
	return NULL != Cast<UBlueprint>(InClass->ClassGeneratedBy);
}

bool UJavascriptLibrary::IsPendingKill(AActor* InActor)
{
	if (InActor != nullptr)
		return InActor->IsPendingKill();
	return true;
}