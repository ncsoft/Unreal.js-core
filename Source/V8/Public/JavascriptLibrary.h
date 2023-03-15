#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameFramework/GameModeBase.h"	
#include "JavascriptProfile.h"
#include "JavascriptIsolate.h"
#include "Engine/StreamableManager.h"
#include "IPAddress.h"
#include "NavMesh/RecastNavMesh.h"
#include "JavascriptLibrary.generated.h"

USTRUCT(BlueprintType)
struct FJavascriptRow
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FString> Values;
};

USTRUCT(BlueprintType)
struct FReadStringFromFileHandle
{
	GENERATED_BODY()

	TSharedPtr<class FReadStringFromFileThread> Runnable;
};

USTRUCT(BlueprintType)
struct FReadStringFromFileAsyncData
{
	GENERATED_BODY()

	UPROPERTY()
	FString String;
};

USTRUCT(BlueprintType)
struct V8_API FDirectoryItem
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Scripting | Javascript")
	FString Name;

	UPROPERTY(BlueprintReadOnly, Category = "Scripting | Javascript")
	bool bIsDirectory = false;
};

#if STATS
struct FJavascriptThreadSafeStaticStatBase : FThreadSafeStaticStatBase
{
	friend class UJavascriptLibrary;

	FJavascriptThreadSafeStaticStatBase() 
	{
		HighPerformanceEnable = nullptr;
	}

	FORCEINLINE_STATS TStatId GetStatId() const
	{
		return *(TStatId*)(&HighPerformanceEnable);
	}
};
#endif

USTRUCT(BlueprintType)
struct FJavascriptStat
{
	GENERATED_BODY()

#if STATS
	TSharedPtr<FJavascriptThreadSafeStaticStatBase> Instance;
#endif
};

/**
* What the type of the payload is
*/
UENUM()
enum class EJavascriptStatDataType : uint8
{
	Invalid,
	/** Not defined. */
	ST_None,
	/** int64. */
	ST_int64,
	/** double. */
	ST_double,
	/** FName. */
	ST_FName,
	/** Memory pointer, stored as uint64. */
	ST_Ptr,
};

UENUM()
namespace EJavascriptEncodingOptions
{
	/** Ordered according to their severity */
	enum Type
	{
		AutoDetect = 0,
		ForceAnsi,
		ForceUnicode,
		ForceUTF8,
		ForceUTF8WithoutBOM
	};
}

/**
* The operation being performed by this message
*/
UENUM()
enum class EJavascriptStatOperation : uint8
{
	Invalid,
	/** Indicates metadata message. */
	SetLongName,
	/** Special message for advancing the stats frame from the game thread. */
	AdvanceFrameEventGameThread,
	/** Special message for advancing the stats frame from the render thread. */
	AdvanceFrameEventRenderThread,
	/** Indicates begin of the cycle scope. */
	CycleScopeStart,
	/** Indicates end of the cycle scope. */
	CycleScopeEnd,
	/** This is not a regular stat operation, but just a special message marker to determine that we encountered a special data in the stat file. */
	SpecialMessageMarker,
	/** Set operation. */
	Set,
	/** Clear operation. */
	Clear,
	/** Add operation. */
	Add,
	/** Subtract operation. */
	Subtract,

	// these are special ones for processed data
	ChildrenStart,
	ChildrenEnd,
	Leaf,
	MaxVal,

	/** This is a memory operation. @see EMemoryOperation. */
	Memory,
};

UENUM()
enum class ELogVerbosity_JS : uint8
{
	/** Not used */
	NoLogging,

	/** Always prints s fatal error to console (and log file) and crashes (even if logging is disabled) */
	Fatal,

	/**
	* Prints an error to console (and log file).
	* Commandlets and the editor collect and report errors. Error messages result in commandlet failure.
	*/
	Error,

	/**
	* Prints a warning to console (and log file).
	* Commandlets and the editor collect and report warnings. Warnings can be treated as an error.
	*/
	Warning,

	/** Prints a message to console (and log file) */
	Display,

	/** Prints a message to a log file (does not print to console) */
	Log,

	/**
	* Prints a verbose message to a log file (if Verbose logging is enabled for the given category,
	* usually used for detailed logging)
	*/
	Verbose,

	/**
	* Prints a verbose message to a log file (if VeryVerbose logging is enabled,
	* usually used for detailed logging that would otherwise spam output)
	*/
	VeryVerbose
};

UENUM()
enum class EFileRead_JS : uint8
{
	FILEREAD_None = 0,
	FILEREAD_NoFail = 1,
	FILEREAD_Silent = 2,
	FILEREAD_NotUsedDummy = 3,
	FILEREAD_AllowWrite = 4
};

USTRUCT(BlueprintType)
struct FJavascriptLogCategory
{
	GENERATED_USTRUCT_BODY()

#if !NO_LOGGING
	TSharedPtr<FLogCategoryBase> Handle;
#endif
};

USTRUCT(BlueprintType)
struct FJavascriptStreamableManager
{
	GENERATED_USTRUCT_BODY()

	FStreamableManager* operator -> () const
	{
		return Handle.Get();
	}

	TSharedPtr<FStreamableManager> Handle;
};

USTRUCT(BlueprintType)
struct FJavascriptStubStruct
{
	GENERATED_BODY()
};

struct FPrivateSocketHandle;

USTRUCT(BlueprintType)
struct FJavascriptSocket
{
	GENERATED_BODY()

	TSharedPtr<FPrivateSocketHandle> Handle;
};

USTRUCT(BlueprintType)
struct FJavascriptInternetAddr
{
	GENERATED_BODY()

	TSharedPtr<FInternetAddr> Handle;
};

USTRUCT(BlueprintType)
struct FJavscriptProperty
{
	GENERATED_BODY()

	UPROPERTY()
	FString Type;

	UPROPERTY()
	FString Name;
};

USTRUCT(BlueprintType)
struct FJavascriptText
{
	GENERATED_BODY()

	UPROPERTY()
	FString String;

	UPROPERTY()
	FString Namespace;

	UPROPERTY()
	FString Key;

	UPROPERTY()
	FName TableId;

	FText Handle;
};

// copy from STextPropertyEditableTextBox.h
enum class ETextPropertyEditAction : uint8
{
	EditedNamespace,
	EditedKey,
	EditedSource,
};

UCLASS()
class V8_API UJavascriptLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:	
	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static FJavascriptSocket CreateSocket(FName SocketType, FString Description, bool bForceUDP = false);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static FJavascriptInternetAddr CreateInternetAddr();

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static bool ResolveIp(FString HostName, FString& OutIp);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static void SetIp(FJavascriptInternetAddr& Addr, FString ResolvedAddress, bool& bValid);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static void SetPort(FJavascriptInternetAddr& Addr, int32 Port);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static bool SendMemoryTo(FJavascriptSocket& Socket, const FJavascriptInternetAddr& ToAddr, int32 NumBytes, int32& BytesSent);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static FJavascriptStreamableManager CreateStreamableManager();

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static void SimpleAsyncLoad(const FJavascriptStreamableManager& Manager, FSoftObjectPath const& Target, int32 Priority);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static void RequestAsyncLoad(const FJavascriptStreamableManager& Manager, const TArray<FSoftObjectPath>& TargetsToStream, FJavascriptFunction DelegateToCall, int32 Priority);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static bool V8_IsEnableHotReload();

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static void V8_SetFlagsFromString(const FString& V8Flags);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static void V8_SetIdleTaskBudget(float BudgetInSeconds);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static void Unload(const FJavascriptStreamableManager& Manager, FSoftObjectPath const& Target);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static bool IsAsyncLoadComplete(const FJavascriptStreamableManager& Manager, FSoftObjectPath const& Target);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static FJavascriptLogCategory CreateLogCategory(const FString& CategoryName, ELogVerbosity_JS InDefaultVerbosity);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static void Log(const FJavascriptLogCategory& Category, ELogVerbosity_JS Verbosity, const FString& Message, const FString& FileName, int32 LineNumber);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static bool IsSuppressed(const FJavascriptLogCategory& Category, ELogVerbosity_JS Verbosity);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static FName GetCategoryName(const FJavascriptLogCategory& Category);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static void SetMobile(USceneComponent* SceneComponent);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static void SetMobility(USceneComponent* SceneComponent, EComponentMobility::Type Type);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static UWorld* Actor_GetWorld(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static UClass* GetBlueprintGeneratedClass(UBlueprint* Blueprint);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static UClass* GetBlueprintGeneratedClassFromPath(FString Path);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static UObject* GetOuter(UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static UObject* GetOutermost(UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static UObject* Duplicate(UObject* Object, UObject* Outer, FName Name);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static bool HasAnyFlags(UObject* Object, int32 Flags);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static bool HasAnyPackageFlags(UPackage* Package, int32 Flags);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript", meta = (BlueprintInternalUseOnly = "true"))
	static FString GetName(UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static bool IsPlayInEditor(UWorld* World);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static bool IsPlayInPreview(UWorld* World);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static bool IsGameWorld(UWorld* World);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static void SetClientTravel(UEngine* Engine, UWorld *InWorld, FString NextURL, ETravelType InTravelType);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static UPackage* CreatePackage(UObject* Outer, FString PackageName);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static UPackage* FindPackage(UObject* InOuter, FString PackageName);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static UPackage* LoadPackage(UPackage* InOuter, FString PackageName);
	
	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static void AddDynamicBinding(UClass* Outer, UDynamicBlueprintBinding* BindingObject);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static UDynamicBlueprintBinding* GetDynamicBinding(UClass* Outer, TSubclassOf<UDynamicBlueprintBinding> BindingObjectClass);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")	
	static void HandleSeamlessTravelPlayer(class AGameModeBase* GameMode, AController*& C);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static void SetRootComponent(AActor* Actor, USceneComponent* Component);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static int32 GetFileSize(UObject* Object, FString Filename);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static bool ReadFile(UObject* Object, FString Filename);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static bool WriteFile(UObject* Object, FString Filename);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static FReadStringFromFileHandle ReadStringFromFileAsync(UObject* Object, FString Filename, FJavascriptFunction Function);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static FString ReadStringFromFile(UObject* Object, FString Filename, EFileRead_JS ReadFlags = EFileRead_JS::FILEREAD_None);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static bool WriteStringToFile(UObject* Object, FString Filename, const FString& Data, EJavascriptEncodingOptions::Type EncodingOptions);

	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static FString GetDir(UObject* Object, FString WhichDir);
	
	UFUNCTION(BlueprintCallable, Category = "Scripting|Javascript")
	static FString ConvertRelativePathToFull(UObject* Object, FString RelativePath);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static bool HasUndo(UEngine* Engine);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static bool ReadDirectory(UObject* Object, FString Directory, TArray<FDirectoryItem>& OutItems);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void ReregisterComponent(UActorComponent* ActorComponent);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void RegisterComponent(UActorComponent* ActorComponent);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void UnregisterComponent(UActorComponent* ActorComponent);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static bool IsRegistered(UActorComponent* ActorComponent);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void GetAllActorsOfClassAndTags(UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, const TArray<FName>& Tags_Accept, const TArray<FName>& Tags_Deny, TArray<AActor*>& OutActors);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static void GetAllActorsOfClassAndTagsInCurrentLevel(UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, const TArray<FName>& Tags_Accept, const TArray<FName>& Tags_Deny, TArray<AActor*>& OutActors);

	UFUNCTION(BlueprintCallable, Category = "Javascript | Editor")
	static int32 GetCurrentProcessId();

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static UModel* GetModel(UWorld* World);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static ULevel* GetLevel(AActor* Actor);	

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static TArray<ULevel*> GetLevels(UWorld* World);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FString GetFunctionName(FJavascriptProfileNode Node);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static int32 GetScriptId(FJavascriptProfileNode Node);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FString GetScriptResourceName(FJavascriptProfileNode Node);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static int32 GetLineNumber(FJavascriptProfileNode Node);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static int32 GetColumnNumber(FJavascriptProfileNode Node);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static int32 GetHitLineCount(FJavascriptProfileNode Node);


	//static bool GetLineTicks(LineTick* entries, unsigned int length) const;
	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FString GetBailoutReason(FJavascriptProfileNode Node);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static int32 GetHitCount(FJavascriptProfileNode Node);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static int32 GetNodeId(FJavascriptProfileNode Node);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static int32 GetChildrenCount(FJavascriptProfileNode Node);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptProfileNode GetChild(FJavascriptProfileNode Node,int32 index);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static int32 GetDeoptInfosCount(FJavascriptProfileNode Node, int32 index);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FString GetDeoptInfo_Reason(FJavascriptProfileNode Node, int32 index);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FString GetDeoptInfo_Stack(FJavascriptProfileNode Node, int32 index);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FString GetArchetypePathName(UObject* Object);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FString GetClassPathName(UClass* Class);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void SetObjectFlags(UObject* Obj, int32 Flags);
	
	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void SetActorFlags(AActor* Actor, int32 Flags);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static float GetLastRenderTime(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static UEnum* CreateEnum(UObject* Outer, FName Name, TArray<FName> DisplayNames, const TArray<FString>& Flags);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void MarkRenderStateDirty(UActorComponent* Component);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void ReregisterAllComponents(AActor* Actor);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool SegmentIntersection2D(const FVector& SegmentStartA, const FVector& SegmentEndA, const FVector& SegmentStartB, const FVector& SegmentEndB, FVector& IntersectionPoint);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool FileExists(const FString& Filename);
    
    UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
    static bool DeleteFile(const FString& Filename, bool ReadOnly = false);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool DirectoryExists(const FString& InDirectory);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool MakeDirectory(const FString& Path, bool Tree);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool DeleteDirectory(const FString& Path, bool RequireExists, bool Tree);
	
	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void GetObjectsWithOuter(const class UObject* Outer, TArray<UObject *>& Results, bool bIncludeNestedObjects, int32 ExclusionFlags, int32 ExclusionInternalFlags);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static class UObject* FindObjectWithOuter(class UObject* Outer, class UClass* ClassToLookFor, FName NameToLookFor);
	
	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void GetObjectsOfClass(UClass* ClassToLookFor, TArray<UObject *>& Results, bool bIncludeDerivedClasses, int32 ExcludeFlags, int32 ExclusionInternalFlags);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void GetDerivedClasses(UClass* ClassToLookFor, TArray<UClass *>& Results, bool bRecursive);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FString GetPlatformName();

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static UObject* TryLoadByPath(FString Path);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void GenerateNavigation(UWorld* InWorld, ARecastNavMesh* NavData);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static const FString& GetMetaData(UField* Field, const FString Key);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static TArray<UField*> GetFields(const UObject* Object, bool bIncludeSuper);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static TArray<FJavscriptProperty> GetStructProperties(const FString StructName, bool bIncludeSuper);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static TArray<FString> GetEnumListByEnumName(const FString EnumName);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static int32 GetFunctionParmsSize(UFunction* Function);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void ClipboardCopy(const FString& String);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FString ClipboardPaste();

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FJavascriptStat NewStat(
		FName InStatName,
		const FString& InStatDesc,
		FName InGroupName,
		FName InGroupCategory,
		const FString& InGroupDesc,
		bool bDefaultEnable,
		bool bShouldClearEveryFrame,
		EJavascriptStatDataType InStatType,
		bool bCycleStat,
		bool bSortByName);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void AddMessage(FJavascriptStat Stat, EJavascriptStatOperation InStatOperation);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void AddMessage_int(FJavascriptStat Stat, EJavascriptStatOperation InStatOperation, int Value, bool bIsCycle);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void AddMessage_float(FJavascriptStat Stat, EJavascriptStatOperation InStatOperation, float Value, bool bIsCycle);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static TArray<UClass*> GetSuperClasses(UClass* InClass);
	
	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool IsGeneratedByBlueprint(UClass* InClass);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool IsPendingKill(AActor* InActor);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FBox GetWorldBounds(UWorld* InWorld);

	UFUNCTION(BlueprintCallable, CustomThunk, Category = "Scripting | Javascript", meta = (CustomStructureParam = "CustomStruct"))
	static void CallJS(FJavascriptFunction Function, const FJavascriptStubStruct& CustomStruct);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool ReadCSV(const FString& InPath, TArray<FJavascriptRow>& OutData, EFileRead_JS ReadFlags);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool WriteCSV(const FString& InPath, TArray<FJavascriptRow>& InData, EJavascriptEncodingOptions::Type EncodingOptions);

#if USE_STABLE_LOCALIZATION_KEYS
	// copy from STextPropertyEditableTextBox.cpp
	static void IssueStableTextId(UPackage* InPackage, const ETextPropertyEditAction InEditAction, const FString& InTextSource, const FString& InProposedNamespace, const FString& InProposedKey, FString& OutStableNamespace, FString& OutStableKey);
#endif // USE_STABLE_LOCALIZATION_KEYS
	static FText UpdateLocalizationText(const FJavascriptText& JText, const struct IPropertyOwner& Owner);

	DECLARE_FUNCTION(execCallJS)
	{
		PARAM_PASSED_BY_VAL(Function, FStructProperty, FJavascriptFunction);

		Stack.MostRecentPropertyAddress = nullptr;
		Stack.MostRecentProperty = nullptr;

		Stack.StepCompiledIn<FStructProperty>(nullptr);
		void* SrcStructAddr = Stack.MostRecentPropertyAddress;
		auto SrcStructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);

		if (SrcStructAddr && SrcStructProperty)
		{
			Function.Execute(SrcStructProperty->Struct, SrcStructAddr);
		}

		P_FINISH;
	}

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static TArray<UActorComponent*> GetComponentsByClass(AActor* Actor, TSubclassOf<UActorComponent> ComponentClass);
};
