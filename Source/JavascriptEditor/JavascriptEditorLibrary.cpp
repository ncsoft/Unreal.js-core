#include "JavascriptEditorLibrary.h"
#include "LandscapeComponent.h"

// WORKAROUND for 4.15
#ifndef WITH_KISSFFT
#define WITH_KISSFFT 0
#endif

#include "JavascriptEditorModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Editor/LandscapeEditor/Private/LandscapeEdModeTools.h"
#include "JavascriptContext.h"
#include "DynamicMeshBuilder.h"
#include "BSPOps.h"
#include "Misc/HotReloadInterface.h"
#include "JavascriptUMG/JavascriptWindow.h"
#include "JavascriptUMG/JavascriptUMGLibrary.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "LevelEditor.h"
#include "Editor/AnimationBlueprintEditor/Private/AnimationBlueprintEditorModule.h"
#include "IAnimationEditorModule.h"
#include "ISkeletalMeshEditorModule.h"
#include "StaticMeshEditorModule.h"
#include "BlueprintEditorModule.h"
#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "LandscapeEdit.h"

#include "Engine/BrushBuilder.h"
#include "Engine/Selection.h"
#include "EngineUtils.h"
#include "GameFramework/Volume.h"
#include "Components/BrushComponent.h"

#include "../../Launch/Resources/Version.h"
#include "HAL/PlatformFileManager.h"
#include "HAL/FileManager.h"
#include "AI/NavDataGenerator.h"
#include "Framework/Application/SlateApplication.h"
#include "Engine/LevelStreaming.h"
#include "VisualLogger/VisualLogger.h"
#include "JavascriptUICommands.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "NavigationSystem.h"

#include "Developer/MessageLog/Public/MessageLogModule.h"
#include "Developer/MessageLog/Public/IMessageLogListing.h"

#include "Kismet2/KismetEditorUtilities.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimTypes.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Curves/RichCurve.h"

#include "Engine/DataTable.h"
#include "Engine/EngineTypes.h"
#if ENGINE_MAJOR_VERSION == 4
	#include "Toolkits/AssetEditorManager.h"
#endif
#include "Stats/StatsData.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "ISourceControlModule.h"
#include "ISourceControlProvider.h"
#include "JavascriptEditorObjectManager.h"
#include "JavascriptEditorModule.h"
#include "SocketSubsystem.h"
#include "Sockets.h"
#include "IPAddress.h"
#include "UObject/SavePackage.h"
#include "CreateBlueprintFromActorDialog.h"

#if WITH_EDITOR
ULandscapeInfo* UJavascriptEditorLibrary::GetLandscapeInfo(ALandscape* Landscape, bool bSpawnNewActor)
{
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 13
	return Landscape ? Landscape->GetLandscapeInfo(bSpawnNewActor) : nullptr;
#else
	return Landscape ? Landscape->GetLandscapeInfo() : nullptr;
#endif
}

void UJavascriptEditorLibrary::SetHeightmapDataFromMemory(ULandscapeInfo* LandscapeInfo, int32 MinX, int32 MinY, int32 MaxX, int32 MaxY)
{
	const int32 SizeX = (1 + MaxX - MinX);
	const int32 SizeY = (1 + MaxY - MinY);

	if (SizeX * SizeY * 2 == FArrayBufferAccessor::GetSize())
	{
		FHeightmapAccessor<false> Accessor(LandscapeInfo);
		Accessor.SetData(MinX, MinY, MaxX, MaxY, (uint16*)FArrayBufferAccessor::GetData());
	}
}

void UJavascriptEditorLibrary::GetHeightmapDataToMemory(ULandscapeInfo* LandscapeInfo, int32 MinX, int32 MinY, int32 MaxX, int32 MaxY)
{
	const int32 SizeX = (1 + MaxX - MinX);
	const int32 SizeY = (1 + MaxY - MinY);

	if (SizeX * SizeY * 2 == FArrayBufferAccessor::GetSize())
	{
		auto Buffer = (uint16*)FArrayBufferAccessor::GetData();

		FHeightmapAccessor<false> Accessor(LandscapeInfo);

		TMap<FIntPoint, uint16> Data;
		Accessor.GetData(MinX, MinY, MaxX, MaxY, Data);

		FMemory::Memzero(Buffer, SizeX * SizeY * 2);

		for (auto it = Data.CreateConstIterator(); it; ++it)
		{
			const auto& Point = it.Key();
			Buffer[Point.X + Point.Y * SizeX] = it.Value();
		}
	}
}

void UJavascriptEditorLibrary::SetAlphamapDataFromMemory(ULandscapeInfo* LandscapeInfo, ULandscapeLayerInfoObject* LayerInfo, int32 MinX, int32 MinY, int32 MaxX, int32 MaxY, ELandscapeLayerPaintingRestriction PaintingRestriction)
{
	if (LayerInfo == nullptr)
	{
		return;
	}

	const int32 SizeX = (1 + MaxX - MinX);
	const int32 SizeY = (1 + MaxY - MinY);

	if (SizeX * SizeY * 1 == FArrayBufferAccessor::GetSize())
	{
		FAlphamapAccessor<false,false> Accessor(LandscapeInfo, LayerInfo);
		Accessor.SetData(MinX, MinY, MaxX, MaxY, (uint8*)FArrayBufferAccessor::GetData(), PaintingRestriction);
	}
}

void UJavascriptEditorLibrary::GetAlphamapDataToMemory(ULandscapeInfo* LandscapeInfo, ULandscapeLayerInfoObject* LayerInfo, int32 MinX, int32 MinY, int32 MaxX, int32 MaxY)
{
	if (LayerInfo == nullptr)
	{
		return;
	}

	const int32 SizeX = (1 + MaxX - MinX);
	const int32 SizeY = (1 + MaxY - MinY);

	if (SizeX * SizeY * 1 == FArrayBufferAccessor::GetSize())
	{
		auto Buffer = (uint8*)FArrayBufferAccessor::GetData();

		FAlphamapAccessor<false, false> Accessor(LandscapeInfo, LayerInfo);

		TMap<FIntPoint, uint8> Data;
		Accessor.GetData(MinX, MinY, MaxX, MaxY, Data);

		FMemory::Memzero(Buffer, SizeX * SizeY);

		for (auto it = Data.CreateConstIterator(); it; ++it)
		{
			const auto& Point = it.Key();
			Buffer[Point.X + Point.Y * SizeX] = it.Value();
		}
	}
}

bool UJavascriptEditorLibrary::GetLandscapeExtent(ULandscapeInfo* LandscapeInfo, int32& MinX, int32& MinY, int32& MaxX, int32& MaxY)
{
	if (!LandscapeInfo) return false;

	return LandscapeInfo->GetLandscapeExtent(MinX, MinY, MaxX, MaxY);
}

ULandscapeLayerInfoObject* UJavascriptEditorLibrary::GetLayerInfoByName(ULandscapeInfo* LandscapeInfo, FName LayerName, ALandscapeProxy* Owner)
{
	return LandscapeInfo ? LandscapeInfo->GetLayerInfoByName(LayerName, Owner) : nullptr;
}

void UJavascriptEditorLibrary::GetAllTagsByAssetData(const FAssetData& AssetData, TArray<FName>& OutArray)
{
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 27
	AssetData.TagsAndValues.GetKeys(OutArray);
#else
	AssetData.TagsAndValues.CopyMap().GetKeys(OutArray);
#endif
}

bool UJavascriptEditorLibrary::GetTagValueByAssetData(const FAssetData& AssetData, const FName& Name, FString& OutValue)
{
#if ENGINE_MAJOR_VERSION > 4
	auto Value = AssetData.TagsAndValues.FindTag(Name);
	if (Value.IsSet())
	{
		OutValue = Value.GetValue();
		return true;
	}
#else
	auto Value = AssetData.TagsAndValues.GetMap().Find(Name);
	if (Value)
	{
		OutValue = *Value;
		return true;
	}
#endif
	else
	{
		return false;
	}
}


void UJavascriptEditorLibrary::GetAllTags(const FJavascriptAssetData& AssetData, TArray<FName>& OutArray)
{
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 27
	AssetData.SourceAssetData.TagsAndValues.GetKeys(OutArray);
#else
	AssetData.SourceAssetData.TagsAndValues.CopyMap().GetKeys(OutArray);
#endif
}

bool UJavascriptEditorLibrary::GetTagValue(const FJavascriptAssetData& AssetData, const FName& Name, FString& OutValue)
{
#if ENGINE_MAJOR_VERSION > 4
	auto Value = AssetData.SourceAssetData.TagsAndValues.FindTag(Name);
	if (Value.IsSet())
	{
		OutValue = Value.GetValue();
		return true;
	}
#else
	auto Value = AssetData.SourceAssetData.TagsAndValues.GetMap().Find(Name);
	if (Value)
	{
		OutValue = *Value;
		return true;
	}
#endif
	else
	{
		return false;
	}
}

UClass* UJavascriptEditorLibrary::GetClass(const FJavascriptAssetData& AssetData)
{
	return AssetData.SourceAssetData.GetClass();
}

UPackage* UJavascriptEditorLibrary::GetPackage(const FJavascriptAssetData& AssetData)
{
	return AssetData.SourceAssetData.GetPackage();
}

UObject* UJavascriptEditorLibrary::GetAsset(const FJavascriptAssetData& AssetData)
{
	return AssetData.SourceAssetData.GetAsset();
}

bool UJavascriptEditorLibrary::IsAssetLoaded(const FJavascriptAssetData& AssetData)
{
	return AssetData.SourceAssetData.IsAssetLoaded();
}

bool UJavascriptEditorLibrary::EditorDestroyActor(UWorld* World, AActor* Actor, bool bShouldModifyLevel)
{
	if (World && ::IsValid(Actor) && Actor->IsValidLowLevelFast())
	{
		IJavascriptEditorModule* JSEditorModule = FModuleManager::GetModulePtr<IJavascriptEditorModule>("JavascriptEditor");

		if (JSEditorModule)
		{
			UJavascriptContext* JSContext = JSEditorModule->GetJavascriptContext();

			if (JSContext )
			{
				JSContext->RemoveObjectInJavacontext(Actor);
			}
		}

		if (!(Actor->HasAnyFlags(RF_BeginDestroyed) || Actor->HasAnyFlags(RF_FinishDestroyed)))
		{
			return World->EditorDestroyActor(Actor, bShouldModifyLevel);
		}
	}
	return false;
}

bool UJavascriptEditorLibrary::ConditionalBeginDestroybyUObject(UObject* TargetObject)
{
	if (::IsValid(TargetObject) && TargetObject->IsValidLowLevel())
	{
		TargetObject->ConditionalBeginDestroy();
		return true;
	}

	return false;
}

void UJavascriptEditorLibrary::SetIsTemporarilyHiddenInEditor(AActor* Actor, bool bIsHidden)
{
	Actor->SetIsTemporarilyHiddenInEditor(bIsHidden);
}

ABrush* UJavascriptEditorLibrary::GetDefaultBrush(UWorld* World)
{
	return World->GetDefaultBrush();
}

bool UJavascriptEditorLibrary::Build(UBrushBuilder* Builder, UWorld* InWorld, ABrush* InBrush)
{
	return Builder->Build(InWorld, InBrush);
}

void UJavascriptEditorLibrary::Select(USelection* Selection, UObject* InObject)
{
	Selection->Select(InObject);
}

void UJavascriptEditorLibrary::Deselect(USelection* Selection, UObject* InObject)
{
	Selection->Deselect(InObject);
}

void UJavascriptEditorLibrary::ToggleSelect(USelection* Selection, UObject* InObject)
{
	Selection->ToggleSelect(InObject);
}

void UJavascriptEditorLibrary::DeselectAll(USelection* Selection, UClass* InClass)
{
	Selection->DeselectAll(InClass);
}

int32 UJavascriptEditorLibrary::GetSelectedObjects(USelection* Selection, TArray<UObject*>& Out)
{
	return Selection->GetSelectedObjects(Out);
}

ABrush* UJavascriptEditorLibrary::csgAdd(ABrush* DefaultBrush, int32 PolyFlags, EBrushType BrushType)
{
	return FBSPOps::csgAddOperation(DefaultBrush, PolyFlags, BrushType);
}

void UJavascriptEditorLibrary::ModifyObject(UObject* Object, bool bAlwaysMarkDirty)
{
	Object->Modify(bAlwaysMarkDirty);
}

void UJavascriptEditorLibrary::InvalidateModelGeometry(UWorld* World, ULevel* InLevel)
{
	World->InvalidateModelGeometry(InLevel);
}

void UJavascriptEditorLibrary::UpdateModelComponents(ULevel* Level)
{
	Level->UpdateModelComponents();
}

FJavascriptWorkspaceItem UJavascriptEditorLibrary::AddGroup(FJavascriptWorkspaceItem Parent, const FText& DisplayName)
{
	FJavascriptWorkspaceItem Out;

	if (Parent.Handle.IsValid())
	{
		Out.Handle = Parent.Handle->AddGroup(DisplayName);
	}

	return Out;
}

FJavascriptWorkspaceItem UJavascriptEditorLibrary::GetGroup(const FString& Name)
{
	FJavascriptWorkspaceItem Out;

	if (Name == TEXT("Root"))
	{
		Out.Handle = WorkspaceMenu::GetMenuStructure().GetStructureRoot();
	}
	else if (Name == TEXT("DeveloperToolsMisc"))
	{
		Out.Handle = WorkspaceMenu::GetMenuStructure().GetDeveloperToolsMiscCategory();
	}

	return Out;
}

FIntPoint UJavascriptEditorLibrary::GetClickPos(const FJavascriptViewportClick& Click)
{
	return Click.Click->GetClickPos();
}

FKey UJavascriptEditorLibrary::GetKey(const FJavascriptViewportClick& Click)
{
	return Click.Click->GetKey();
}

EInputEvent UJavascriptEditorLibrary::GetEvent(const FJavascriptViewportClick& Click)
{
	return Click.Click->GetEvent();
}

bool UJavascriptEditorLibrary::IsControlDown(const FJavascriptViewportClick& Click)
{
	return Click.Click->IsControlDown();
}

bool UJavascriptEditorLibrary::IsShiftDown(const FJavascriptViewportClick& Click)
{
	return Click.Click->IsShiftDown();
}

bool UJavascriptEditorLibrary::IsAltDown(const FJavascriptViewportClick& Click)
{
	return Click.Click->IsAltDown();
}

FVector UJavascriptEditorLibrary::GetOrigin(const FJavascriptViewportClick& Click)
{
	return Click.Click->GetOrigin();
}

FVector UJavascriptEditorLibrary::GetDirection(const FJavascriptViewportClick& Click)
{
	return Click.Click->GetDirection();
}

bool UJavascriptEditorLibrary::GetWorldPositionFromViewportClick(const AActor* Actor, const FJavascriptViewportClick& Click, FHitResult& OutHitResult)
{
	if (::IsValid(Actor))
	{
		FVector Start = GetOrigin(Click);
		FVector End = Start + GetDirection(Click) * HALF_WORLD_MAX;
		return Actor->ActorLineTraceSingle(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility, FCollisionQueryParams());
	}

	return false;
}

void UJavascriptEditorLibrary::DrawWireBox(const FJavascriptPDI& PDI, const FBox& Box, const FLinearColor& Color, ESceneDepthPriorityGroup DepthPriority, float Thickness, float DepthBias, bool bScreenSpace)
{
	::DrawWireBox(PDI.PDI, Box, Color, DepthPriority, Thickness, DepthBias, bScreenSpace);
}
void UJavascriptEditorLibrary::DrawWireBox2(const FJavascriptPDI& PDI, const FTransform& Matrix, const FBox& Box, const FLinearColor& Color, ESceneDepthPriorityGroup DepthPriority, float Thickness, float DepthBias, bool bScreenSpace)
{
	::DrawWireBox(PDI.PDI, Matrix.ToMatrixWithScale(), Box, Color, DepthPriority, Thickness, DepthBias, bScreenSpace);
}
void UJavascriptEditorLibrary::DrawCircle(const FJavascriptPDI& PDI, const FVector& Base, const FVector& X, const FVector& Y, const FLinearColor& Color, float Radius, int32 NumSides, ESceneDepthPriorityGroup DepthPriority, float Thickness, float DepthBias, bool bScreenSpace)
{
	::DrawCircle(PDI.PDI, Base, X, Y, Color, Radius, NumSides, DepthPriority, Thickness, DepthBias, bScreenSpace);
}
void UJavascriptEditorLibrary::DrawArc(const FJavascriptPDI& PDI, const FVector Base, const FVector X, const FVector Y, const float MinAngle, const float MaxAngle, const float Radius, const int32 Sections, const FLinearColor& Color, ESceneDepthPriorityGroup DepthPriority)
{
	::DrawArc(PDI.PDI, Base, X, Y, MinAngle, MaxAngle, Radius, Sections, Color, DepthPriority);
}
void UJavascriptEditorLibrary::DrawWireSphere(const FJavascriptPDI& PDI, const FVector& Base, const FLinearColor& Color, float Radius, int32 NumSides, ESceneDepthPriorityGroup DepthPriority, float Thickness, float DepthBias, bool bScreenSpace)
{
	::DrawWireSphere(PDI.PDI, Base, Color, Radius, NumSides, DepthPriority, Thickness, DepthBias, bScreenSpace);
}
void UJavascriptEditorLibrary::DrawWireSphere2(const FJavascriptPDI& PDI, const FTransform& Transform, const FLinearColor& Color, float Radius, int32 NumSides, ESceneDepthPriorityGroup DepthPriority, float Thickness, float DepthBias, bool bScreenSpace)
{
	::DrawWireSphere(PDI.PDI, Transform, Color, Radius, NumSides, DepthPriority, Thickness, DepthBias, bScreenSpace);
}
void UJavascriptEditorLibrary::DrawWireSphereAutoSides(const FJavascriptPDI& PDI, const FVector& Base, const FLinearColor& Color, float Radius, ESceneDepthPriorityGroup DepthPriority, float Thickness, float DepthBias, bool bScreenSpace)
{
	::DrawWireSphereAutoSides(PDI.PDI, Base, Color, Radius, DepthPriority, Thickness, DepthBias, bScreenSpace);
}
void UJavascriptEditorLibrary::DrawWireSphereAutoSides2(const FJavascriptPDI& PDI, const FTransform& Transform, const FLinearColor& Color, float Radius, ESceneDepthPriorityGroup DepthPriority, float Thickness, float DepthBias, bool bScreenSpace)
{
	::DrawWireSphereAutoSides(PDI.PDI, Transform, Color, Radius, DepthPriority, Thickness, DepthBias, bScreenSpace);
}
void UJavascriptEditorLibrary::DrawWireCylinder(const FJavascriptPDI& PDI, const FVector& Base, const FVector& X, const FVector& Y, const FVector& Z, const FLinearColor& Color, float Radius, float HalfHeight, int32 NumSides, ESceneDepthPriorityGroup DepthPriority, float Thickness, float DepthBias, bool bScreenSpace)
{
	::DrawWireCylinder(PDI.PDI, Base, X, Y, Z, Color, Radius, HalfHeight, NumSides, DepthPriority, Thickness, DepthBias, bScreenSpace);
}
void UJavascriptEditorLibrary::DrawWireCapsule(const FJavascriptPDI& PDI, const FVector& Base, const FVector& X, const FVector& Y, const FVector& Z, const FLinearColor& Color, float Radius, float HalfHeight, int32 NumSides, ESceneDepthPriorityGroup DepthPriority, float Thickness, float DepthBias, bool bScreenSpace)
{
	::DrawWireCapsule(PDI.PDI, Base, X, Y, Z, Color, Radius, HalfHeight, NumSides, DepthPriority, Thickness, DepthBias, bScreenSpace);
}
void UJavascriptEditorLibrary::DrawWireChoppedCone(const FJavascriptPDI& PDI, const FVector& Base, const FVector& X, const FVector& Y, const FVector& Z, const FLinearColor& Color, float Radius, float TopRadius, float HalfHeight, int32 NumSides, ESceneDepthPriorityGroup DepthPriority)
{
	::DrawWireChoppedCone(PDI.PDI, Base, X, Y, Z, Color, Radius, TopRadius, HalfHeight, NumSides, DepthPriority);
}
void UJavascriptEditorLibrary::DrawWireCone(const FJavascriptPDI& PDI, TArray<FVector>& Verts, const FTransform& Transform, float ConeRadius, float ConeAngle, int32 ConeSides, const FLinearColor& Color, ESceneDepthPriorityGroup DepthPriority, float Thickness, float DepthBias, bool bScreenSpace)
{
	::DrawWireCone(PDI.PDI, Verts, Transform.ToMatrixWithScale(), ConeRadius, ConeAngle, ConeSides, Color, DepthPriority, Thickness, DepthBias, bScreenSpace);
}
void UJavascriptEditorLibrary::DrawWireSphereCappedCone(const FJavascriptPDI& PDI, const FTransform& Transform, float ConeRadius, float ConeAngle, int32 ConeSides, int32 ArcFrequency, int32 CapSegments, const FLinearColor& Color, ESceneDepthPriorityGroup DepthPriority)
{
	::DrawWireSphereCappedCone(PDI.PDI, Transform, ConeRadius, ConeAngle, ConeSides, ArcFrequency, CapSegments, Color, DepthPriority);
}
void UJavascriptEditorLibrary::DrawOrientedWireBox(const FJavascriptPDI& PDI, const FVector& Base, const FVector& X, const FVector& Y, const FVector& Z, FVector Extent, const FLinearColor& Color, ESceneDepthPriorityGroup DepthPriority, float Thickness, float DepthBias, bool bScreenSpace)
{
	::DrawOrientedWireBox(PDI.PDI, Base, X, Y, Z, Extent, Color, DepthPriority, Thickness, DepthBias, bScreenSpace);
}
void UJavascriptEditorLibrary::DrawDirectionalArrow(const FJavascriptPDI& PDI, const FTransform& ArrowToWorld, const FLinearColor& InColor, float Length, float ArrowSize, ESceneDepthPriorityGroup DepthPriority, float Thickness)
{
	::DrawDirectionalArrow(PDI.PDI, ArrowToWorld.ToMatrixWithScale(), InColor, Length, ArrowSize, DepthPriority, Thickness);
}
void UJavascriptEditorLibrary::DrawConnectedArrow(const FJavascriptPDI& PDI, const FTransform& ArrowToWorld, const FLinearColor& Color, float ArrowHeight, float ArrowWidth, ESceneDepthPriorityGroup DepthPriority, float Thickness, int32 NumSpokes)
{
	::DrawConnectedArrow(PDI.PDI, ArrowToWorld.ToMatrixWithScale(), Color, ArrowHeight, ArrowWidth, DepthPriority, Thickness, NumSpokes);
}
void UJavascriptEditorLibrary::DrawWireStar(const FJavascriptPDI& PDI, const FVector& Position, float Size, const FLinearColor& Color, ESceneDepthPriorityGroup DepthPriority)
{
	::DrawWireStar(PDI.PDI, Position, Size, Color, DepthPriority);
}
void UJavascriptEditorLibrary::DrawDashedLine(const FJavascriptPDI& PDI, const FVector& Start, const FVector& End, const FLinearColor& Color, float DashSize, ESceneDepthPriorityGroup DepthPriority, float DepthBias)
{
	::DrawDashedLine(PDI.PDI, Start, End, Color, DashSize, DepthPriority, DepthBias);
}
void UJavascriptEditorLibrary::DrawWireDiamond(const FJavascriptPDI& PDI, const FTransform& Transform, float Size, const FLinearColor& InColor, ESceneDepthPriorityGroup DepthPriority)
{
	::DrawWireDiamond(PDI.PDI, Transform.ToMatrixWithScale(), Size, InColor, DepthPriority);
}
void UJavascriptEditorLibrary::DrawPolygon(const FJavascriptPDI& PDI, const TArray<FVector>& Verts, const FLinearColor& InColor, ESceneDepthPriorityGroup DepthPriority, EJavascriptRHIFeatureLevel::Type RHIFeatureLevel)
{
	FDynamicMeshBuilder MeshBuilder((ERHIFeatureLevel::Type)RHIFeatureLevel);

	FColor Color = InColor.ToFColor(false);

	for (const auto& V : Verts)
	{
		MeshBuilder.AddVertex(FVector3f(V), FVector2f(0, 0), FVector3f(1, 0, 0), FVector3f(0, 1, 0), FVector3f(0, 0, 1), Color);
	}

	for (int32 Index = 0; Index < Verts.Num() - 2; ++Index)
	{
		MeshBuilder.AddTriangle(0, Index + 1, Index + 2);
	}

	static auto TransparentPlaneMaterialXY = (UMaterial*)StaticLoadObject(UMaterial::StaticClass(), nullptr, TEXT("/Engine/EditorMaterials/WidgetVertexColorMaterial.WidgetVertexColorMaterial"), nullptr, LOAD_None, nullptr);
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 22
	MeshBuilder.Draw(PDI.PDI, FMatrix::Identity, TransparentPlaneMaterialXY->GetRenderProxy(false), DepthPriority, 0.f);
#else
	MeshBuilder.Draw(PDI.PDI, FMatrix::Identity, TransparentPlaneMaterialXY->GetRenderProxy(), DepthPriority, 0.f);
#endif
}

struct HJavascriptHitProxy : public HHitProxy
{
	DECLARE_HIT_PROXY(JAVASCRIPTEDITOR_API);

	FName Name;

	HJavascriptHitProxy(FName InName) :
		HHitProxy(HPP_UI),
		Name(InName)
	{}

	virtual EMouseCursor::Type GetMouseCursor() override
	{
		return EMouseCursor::Crosshairs;
	}

	/**
	* Method that specifies whether the hit proxy *always* allows translucent primitives to be associated with it or not,
	* regardless of any other engine/editor setting. For example, if translucent selection was disabled, any hit proxies
	*  returning true would still allow translucent selection. In this specific case, true is always returned because geometry
	* mode hit proxies always need to be selectable or geometry mode will not function correctly.
	*
	* @return	true if translucent primitives are always allowed with this hit proxy; false otherwise
	*/
	virtual bool AlwaysAllowsTranslucentPrimitives() const override
	{
		return true;
	}
};

IMPLEMENT_HIT_PROXY(HJavascriptHitProxy, HHitProxy);

void UJavascriptEditorLibrary::SetHitProxy(const FJavascriptPDI& PDI, const FName& Name)
{
	if (Name.IsNone())
	{
		PDI.PDI->SetHitProxy(nullptr);
	}
	else
	{
		PDI.PDI->SetHitProxy(new HJavascriptHitProxy(Name));
	}
}

AActor* UJavascriptEditorLibrary::GetActor(const FJavascriptHitProxy& Proxy)
{
	if (Proxy.HitProxy && Proxy.HitProxy->IsA(HActor::StaticGetType()))
	{
		HActor* ActorHit = static_cast<HActor*>(Proxy.HitProxy);
		if (ActorHit->Actor != nullptr)
		{
			return ActorHit->Actor;
		}
	}

	return nullptr;
}

FName UJavascriptEditorLibrary::GetName(const FJavascriptHitProxy& Proxy)
{
	if (Proxy.HitProxy && Proxy.HitProxy->IsA(HJavascriptHitProxy::StaticGetType()))
	{
		HJavascriptHitProxy* ActorHit = static_cast<HJavascriptHitProxy*>(Proxy.HitProxy);
		return ActorHit->Name;
	}

	return FName();
}

FString UJavascriptEditorLibrary::GetActorLabel(AActor* Actor)
{
	return Actor->GetActorLabel();
}

void UJavascriptEditorLibrary::SetActorLabel(AActor* Actor, const FString& NewActorLabel, bool bMarkDirty)
{
	Actor->SetActorLabel(NewActorLabel, bMarkDirty);
}

void UJavascriptEditorLibrary::ClearActorLabel(AActor* Actor)
{
	Actor->ClearActorLabel();
}

bool UJavascriptEditorLibrary::SetActorLocation(AActor* Actor, FVector NewLocation, bool bSweep, FHitResult& SweepHitResult, bool bTeleport)
{
	if (IsValid(Actor))
	{
		return Actor->SetActorLocation(NewLocation, bSweep, (bSweep ? &SweepHitResult : nullptr), TeleportFlagToEnum(bTeleport));
	}
	return false;
}

FVector UJavascriptEditorLibrary::GetActorLocation(AActor* Actor)
{
	if (IsValid(Actor))
	{
		return Actor->GetActorLocation();
	}
	return FVector::ZeroVector;
}

FRotator UJavascriptEditorLibrary::GetActorRotation(AActor* Actor)
{
	return IsValid(Actor) ? Actor->GetActorRotation() : FRotator::ZeroRotator;
}

bool UJavascriptEditorLibrary::IsActorLabelEditable(AActor* Actor)
{
	return Actor->IsActorLabelEditable();
}

void UJavascriptEditorLibrary::SetFolderPath(AActor* Actor, const FName& NewFolderPath)
{
	Actor->SetFolderPath(NewFolderPath);
}

void UJavascriptEditorLibrary::SetFolderPath_Recursively(AActor* Actor, const FName& NewFolderPath)
{
	Actor->SetFolderPath_Recursively(NewFolderPath);
}

FName UJavascriptEditorLibrary::GetFolderPath(AActor* Actor)
{
	return Actor->GetFolderPath();
}

void UJavascriptEditorLibrary::BroadcastHotReload()
{
	// Register to have Populate called when doing a Hot Reload.
	FCoreUObjectDelegates::ReloadCompleteDelegate.Broadcast(EReloadCompleteReason::HotReloadManual);
}

void UJavascriptEditorLibrary::BroadcastAssetCreated(UObject* NewAsset)
{
	FAssetRegistryModule::AssetCreated(NewAsset);
}

bool UJavascriptEditorLibrary::IsActive(UTransactor* Transactor)
{
	return Transactor->IsActive();
}

int32 UJavascriptEditorLibrary::GetQueueLength(UTransactor* Transactor)
{
	return Transactor->GetQueueLength();
}

FJavascriptTransaction UJavascriptEditorLibrary::GetTransaction(UTransactor* Transactor, int32 QueueIndex)
{
	FJavascriptTransaction Out;
	Out.Transaction = Transactor->GetTransaction(QueueIndex);
	return Out;
}

FText UJavascriptEditorLibrary::GetTitle(const FJavascriptTransaction& Transaction)
{
	return Transaction->GetContext().Title;
}

FString UJavascriptEditorLibrary::GetContext(const FJavascriptTransaction& Transaction)
{
	return Transaction->GetContext().Context;
}

UObject* UJavascriptEditorLibrary::GetPrimaryObject(const FJavascriptTransaction& Transaction)
{
	return Transaction->GetContext().PrimaryObject;
}

void UJavascriptEditorLibrary::EditorAddModalWindow(FJavascriptSlateWidget Widget)
{
	auto Window = StaticCastSharedPtr<SWindow>(Widget.Widget);
	if (Window.IsValid())
	{
		GEditor->EditorAddModalWindow(Window.ToSharedRef());
	}
}

FJavascriptSlateWidget UJavascriptEditorLibrary::GetRootWindow()
{
	return { StaticCastSharedPtr<SWidget>(FGlobalTabmanager::Get()->GetRootWindow()) };
}

void UJavascriptEditorLibrary::CreatePropertyEditorToolkit(TArray<UObject*> ObjectsForPropertiesMenu)
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
#if ENGINE_MAJOR_VERSION > 4 
	PropertyEditorModule.CreatePropertyEditorToolkit(TSharedPtr<IToolkitHost>(), ObjectsForPropertiesMenu);
#else
	PropertyEditorModule.CreatePropertyEditorToolkit(EToolkitMode::Standalone, TSharedPtr<IToolkitHost>(), ObjectsForPropertiesMenu);
#endif
}

static FName NAME_LevelEditor("LevelEditor");
static FName NAME_MaterialEditor("MaterialEditor");
static FName NAME_AnimationBlueprintEditor("AnimationBlueprintEditor");
static FName NAME_AnimationEditor("AnimationEditor");
static FName NAME_BlueprintEditor("Kismet");
static FName NAME_SkeletalMeshEditor("SkeletalMeshEditor");
static FName NAME_StaticMeshEditor("StaticMeshEditor");

FJavascriptExtensibilityManager UJavascriptEditorLibrary::GetMenuExtensibilityManager(FName What)
{
	if (What == NAME_LevelEditor)
	{
		FLevelEditorModule& LevelEditor = FModuleManager::LoadModuleChecked<FLevelEditorModule>(NAME_LevelEditor);
		return {LevelEditor.GetMenuExtensibilityManager()};
	}
	else if (What == NAME_BlueprintEditor)
	{
		FBlueprintEditorModule& BlueprintEditor = FModuleManager::LoadModuleChecked<FBlueprintEditorModule>(NAME_BlueprintEditor);
		return { BlueprintEditor.GetMenuExtensibilityManager() };
	}
	else if (What == NAME_AnimationBlueprintEditor)
	{
		FAnimationBlueprintEditorModule& AnimationEditor = FModuleManager::LoadModuleChecked<FAnimationBlueprintEditorModule>(NAME_AnimationBlueprintEditor);
		return { AnimationEditor.GetMenuExtensibilityManager() };
	}
	else if (What == NAME_AnimationEditor)
	{
		IAnimationEditorModule& AnimationEditor = FModuleManager::LoadModuleChecked<IAnimationEditorModule>(NAME_AnimationEditor);
		return { AnimationEditor.GetMenuExtensibilityManager() };
	}
	return FJavascriptExtensibilityManager();
}

FJavascriptExtensibilityManager UJavascriptEditorLibrary::GetToolBarExtensibilityManager(FName What)
{
	if (What == NAME_LevelEditor)
	{
		FLevelEditorModule& LevelEditor = FModuleManager::LoadModuleChecked<FLevelEditorModule>(NAME_LevelEditor);
		return{ LevelEditor.GetToolBarExtensibilityManager() };
	}
	else if (What == NAME_BlueprintEditor)
	{
		FBlueprintEditorModule& BlueprintEditor = FModuleManager::LoadModuleChecked<FBlueprintEditorModule>(NAME_BlueprintEditor);
		return { BlueprintEditor.GetMenuExtensibilityManager() };
	}
	else if (What == NAME_AnimationBlueprintEditor)
	{
		FAnimationBlueprintEditorModule& AnimationEditor = FModuleManager::LoadModuleChecked<FAnimationBlueprintEditorModule>(NAME_AnimationBlueprintEditor);
		return { AnimationEditor.GetToolBarExtensibilityManager() };
	}
	else if (What == NAME_AnimationEditor)
	{
		IAnimationEditorModule& AnimationEditor = FModuleManager::LoadModuleChecked<IAnimationEditorModule>(NAME_AnimationEditor);
		return { AnimationEditor.GetToolBarExtensibilityManager() };
	}
	else if (What == NAME_SkeletalMeshEditor)
	{
		ISkeletalMeshEditorModule& SkeletalMeshEditor = FModuleManager::LoadModuleChecked<ISkeletalMeshEditorModule>(NAME_SkeletalMeshEditor);
		return { SkeletalMeshEditor.GetToolBarExtensibilityManager() };
	}
	else if (What == NAME_StaticMeshEditor)
	{
		IStaticMeshEditorModule& StaticMeshEditor = FModuleManager::LoadModuleChecked<IStaticMeshEditorModule>(NAME_StaticMeshEditor);
		return { StaticMeshEditor.GetToolBarExtensibilityManager() };
	}
	return FJavascriptExtensibilityManager();
}

FJavascriptUICommandList UJavascriptEditorLibrary::GetLevelEditorActions()
{
	FLevelEditorModule& LevelEditor = FModuleManager::LoadModuleChecked<FLevelEditorModule>(NAME_LevelEditor);
	TSharedRef<FUICommandList> LevelEditorActions = LevelEditor.GetGlobalLevelEditorActions();

	FJavascriptUICommandList CommandList;
	CommandList.Handle = LevelEditorActions;
	return CommandList;
}

void UJavascriptEditorLibrary::AddExtender(FJavascriptExtensibilityManager Manager, FJavascriptExtender Extender)
{
	if (Manager.Handle.IsValid() && Extender.Handle.IsValid())
	{
		Manager->AddExtender(Extender.Handle);
	}
}

void UJavascriptEditorLibrary::RemoveExtender(FJavascriptExtensibilityManager Manager, FJavascriptExtender Extender)
{
	if (Manager.Handle.IsValid() && Extender.Handle.IsValid())
	{
		Manager->RemoveExtender(Extender.Handle);
	}
}

void UJavascriptEditorLibrary::AddLazyExtender(FJavascriptExtensibilityManager Manager, UJavascriptLazyExtenderDelegates* Delegates)
{
	if (Manager.Handle.IsValid())
	{
		Delegates->AddToRoot();
		Manager->GetExtenderDelegates().Add(FAssetEditorExtender::CreateLambda([=](const TSharedRef<FUICommandList> CommandList, const TArray<UObject*> EditingObjects)
		{
			//@hack: instead of returnless javascript.call
			auto Extender = Delegates->GetExtender.Execute({ CommandList }, EditingObjects).Handle.ToSharedRef();
			return Extender;
		}));
		Manager.LazyExtenders.Add(Delegates);
	}
}

void UJavascriptEditorLibrary::RemoveAllLazyExtender(FJavascriptExtensibilityManager Manager)
{
	if (Manager.Handle.IsValid())
	{
		//@todo : remove from lazyextenders
		for (auto* Delegates : Manager.LazyExtenders)
		{
			Delegates->RemoveFromRoot();
		}
		Manager.LazyExtenders.Empty();
		Manager->GetExtenderDelegates().Empty();
	}
}

bool UJavascriptEditorLibrary::SavePackage(UPackage* Package, FString FileName)
{
	if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*FileName))
	{
		FPlatformFileManager::Get().GetPlatformFile().SetReadOnly(*FileName, false);
	}

	UWorld* World = UWorld::FindWorldInPackage(Package);
	bool bSavedCorrectly;

	if (World)
	{
		const FSavePackageArgs SaveArgs = { nullptr,
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
			nullptr,
#endif
			RF_NoFlags, 0U, false,
			true, true, FDateTime::MinValue(), GError };
		bSavedCorrectly = UPackage::SavePackage(Package, World, *FileName, SaveArgs);
	}
	else
	{
		const FSavePackageArgs SaveArgs = { nullptr,
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
			nullptr,
#endif
			RF_Standalone, 0U, false,
			true, true, FDateTime::MinValue(), GError };
		bSavedCorrectly =  UPackage::SavePackage(Package, nullptr,  *FileName, SaveArgs);
	}
	return bSavedCorrectly;
}

bool UJavascriptEditorLibrary::DeletePackage(UPackage* Package)
{
	FString PackageName = Package->GetName();
	FString BasePackageFileName = FPackageName::LongPackageNameToFilename(PackageName);
	if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*BasePackageFileName))
	{
		return IFileManager::Get().Delete(*BasePackageFileName, false, true);
	}
	return false;
}

void UJavascriptEditorLibrary::CreateBrushForVolumeActor(AVolume* NewActor, UBrushBuilder* BrushBuilder)
{
	if (NewActor != nullptr)
	{
		// this code builds a brush for the new actor
		NewActor->PreEditChange(nullptr);

		NewActor->PolyFlags = 0;
		NewActor->Brush = NewObject<UModel>(NewActor, NAME_None, RF_Transactional);
		NewActor->Brush->Initialize(nullptr, true);
		NewActor->Brush->Polys = NewObject<UPolys>(NewActor->Brush, NAME_None, RF_Transactional);
		NewActor->GetBrushComponent()->Brush = NewActor->Brush;
		if (BrushBuilder != nullptr)
		{
			NewActor->BrushBuilder = DuplicateObject<UBrushBuilder>(BrushBuilder, NewActor);
		}

		BrushBuilder->Build(NewActor->GetWorld(), NewActor);

		FBSPOps::csgPrepMovingBrush(NewActor);

		// Set the texture on all polys to nullptr.  This stops invisible textures
		// dependencies from being formed on volumes.
		if (NewActor->Brush)
		{
			for (int32 poly = 0; poly < NewActor->Brush->Polys->Element.Num(); ++poly)
			{
				FPoly* Poly = &(NewActor->Brush->Polys->Element[poly]);
				Poly->Material = nullptr;
			}
		}

		NewActor->PostEditChange();
	}
}

UWorld* UJavascriptEditorLibrary::FindWorldInPackage(UPackage* Package)
{
	if (::IsValid(Package))
	{
		return UWorld::FindWorldInPackage(Package);
	}
	return nullptr;
}

FString UJavascriptEditorLibrary::ExportNavigation(UWorld* InWorld, FString Name)
{
	InWorld->WorldType = EWorldType::Editor;
	InWorld->AddToRoot();
	if (!InWorld->bIsWorldInitialized)
	{
		UWorld::InitializationValues IVS;
		IVS.EnableTraceCollision(true);
		IVS.CreateNavigation(true);

		InWorld->InitWorld(IVS);
		InWorld->PersistentLevel->UpdateModelComponents();
		InWorld->UpdateWorldComponents(true, false);
		InWorld->UpdateLevelStreaming();
		//InWorld->LoadSecondaryLevels(true, nullptr);
	}

	FWorldContext &WorldContext = GEditor->GetEditorWorldContext(true);
	WorldContext.SetCurrentWorld(InWorld);
	GWorld = InWorld;

//	UGameplayStatics::LoadStreamLevel(InWorld, FName(TEXT("BackgroundMountains")), true, true, FLatentActionInfo());
// 	UWorld::InitializationValues().ShouldSimulatePhysics(false).EnableTraceCollision(true).CreateNavigation(InWorldType == EWorldType::Editor).CreateAISystem(InWorldType == EWorldType::Editor);
// 	UNavigationSystem::InitializeForWorld(InWorld, FNavigationSystemRunMode::GameMode);
	if (UNavigationSystemV1* Nav = Cast<UNavigationSystemV1>(InWorld->GetNavigationSystem()))
	{
		Nav->InitializeForWorld(*InWorld, FNavigationSystemRunMode::EditorMode);
		Nav->Build();
		if (const ANavigationData* NavData = Nav->GetDefaultNavDataInstance(FNavigationSystem::ECreateIfEmpty::Create))
		{
			if (const FNavDataGenerator* Generator = NavData->GetGenerator())
			{
				//const FString Name = NavData->GetName();
				//auto fname = FString::Printf(TEXT("%s/%s"), *FPaths::GameSavedDir(), *Name);
				Generator->ExportNavigationData(Name);
				InWorld->RemoveFromRoot();
				return Name;
			}
			else
			{
				UE_LOG(LogNavigation, Error, TEXT("Failed to export navigation data due to missing generator"));
			}
		}
		else
		{
			UE_LOG(LogNavigation, Error, TEXT("Failed to export navigation data due to navigation data"));
		}
	}
	else
	{
		UE_LOG(LogNavigation, Error, TEXT("Failed to export navigation data due to missing navigation system"));
	}

	InWorld->RemoveFromRoot();

	return FString("");
}

void UJavascriptEditorLibrary::RequestEndPlayMapInPIE()
{
	if (GEditor->PlayWorld)
	{
		GEditor->RequestEndPlayMap();
	}
}

void UJavascriptEditorLibrary::RemoveLevelInstance(UWorld* World)
{
	// Clean up existing world and remove it from root set so it can be garbage collected.
	World->bIsLevelStreamingFrozen = false;
	World->SetShouldForceUnloadStreamingLevels(true);
	World->SetShouldForceVisibleStreamingLevels(false);
	for (ULevelStreaming* StreamingLevel : World->GetStreamingLevels())
	{
		StreamingLevel->SetIsRequestingUnloadAndRemoval(true);
	}
	World->RefreshStreamingLevels();
}

void UJavascriptEditorLibrary::AddWhitelistedObject(UObject* InObject)
{
	FVisualLogger::Get().AddObjectToAllowList(*InObject);
}

void UJavascriptEditorLibrary::PostEditChange(UObject* InObject)
{
	if (InObject)
	{
		InObject->PostEditChange();
	}
}

bool UJavascriptEditorLibrary::MarkPackageDirty(UObject* InObject)
{
	return InObject && InObject->MarkPackageDirty();
}

void UJavascriptEditorLibrary::CreateLogListing(const FName& InLogName, const FText& InLabel)
{
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	FMessageLogInitializationOptions InitOptions;
	InitOptions.bShowFilters = true;
	InitOptions.bShowPages = true;
	TSharedRef<class IMessageLogListing> Listing = MessageLogModule.CreateLogListing(InLogName, InitOptions);
	Listing->SetLabel(InLabel);
	MessageLogModule.RegisterLogListing(InLogName, InLabel, InitOptions);
}

FJavascriptSlateWidget UJavascriptEditorLibrary::CreateLogListingWidget(const FName& InLogName)
{
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	FJavascriptSlateWidget Out;
	Out.Widget = MessageLogModule.CreateLogListingWidget(MessageLogModule.GetLogListing(InLogName));
	return Out;
}

void UJavascriptEditorLibrary::AddLogListingMessage(const FName& InLogName, EJavascriptMessageSeverity::Type InSeverity, const FString& LogText)
{
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	TSharedRef<FTokenizedMessage> Line = FTokenizedMessage::Create((EMessageSeverity::Type)(InSeverity));
	Line->AddToken(FTextToken::Create(FText::FromString(LogText)));
	MessageLogModule.GetLogListing(InLogName)->AddMessage(Line, false);
}

UEditorEngine* UJavascriptEditorLibrary::GetEngine()
{
	return Cast<UEditorEngine>(GEngine);
}

UClass* UJavascriptEditorLibrary::GetParentClassOfBlueprint(UBlueprint* Blueprint)
{
	return Cast<UClass>(Blueprint->ParentClass);
}

USCS_Node* FindSCSNode(const TArray<USCS_Node*>& Nodes, UActorComponent* Component)
{
	for (auto* Node : Nodes)
	{
		if (Node->ComponentTemplate == Component)
			return Node;

		auto* Ret = FindSCSNode(Node->ChildNodes, Component);
		if (Ret)
			return Ret;
	}
	return nullptr;
}

void UJavascriptEditorLibrary::AddComponentsToBlueprint(UBlueprint* Blueprint, const TArray<UActorComponent*>& Components, bool bHarvesting, UActorComponent* OptionalNewRootComponent, bool bKeepMobility)
{
	auto* OptionalNewRootNode = FindSCSNode(Blueprint->SimpleConstructionScript->GetRootNodes(), OptionalNewRootComponent);

	FKismetEditorUtilities::FAddComponentsToBlueprintParams Params;
	Params.HarvestMode = (bHarvesting ? FKismetEditorUtilities::EAddComponentToBPHarvestMode::Harvest_UseComponentName : FKismetEditorUtilities::EAddComponentToBPHarvestMode::None);
	Params.OptionalNewRootNode = OptionalNewRootNode;
	Params.bKeepMobility = bKeepMobility;
	FKismetEditorUtilities::AddComponentsToBlueprint(Blueprint, Components, Params);
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
}

void UJavascriptEditorLibrary::RemoveComponentFromBlueprint(UBlueprint* Blueprint, UActorComponent* RemoveComponent, bool bPromoteChildren)
{
	auto* RemoveNode = FindSCSNode(Blueprint->SimpleConstructionScript->GetRootNodes(), RemoveComponent);
	if (RemoveNode)
	{
		FBlueprintEditorUtils::RemoveVariableNodes(Blueprint, RemoveNode->GetVariableName());
		if (bPromoteChildren)
			Blueprint->SimpleConstructionScript->RemoveNodeAndPromoteChildren(RemoveNode);
		else
			Blueprint->SimpleConstructionScript->RemoveNode(RemoveNode);

		// Clear the delegate
		RemoveNode->SetOnNameChanged(FSCSNodeNameChanged());
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	}
}

void UJavascriptEditorLibrary::CompileBlueprint(UBlueprint* Blueprint)
{
	//FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
	FKismetEditorUtilities::CompileBlueprint(Blueprint);
 	//Blueprint->Modify();
 	//Blueprint->PostEditChange();
}

bool UJavascriptEditorLibrary::OpenEditorForAsset(UObject* Asset)
{
	if (auto* SubSystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
	{
		return SubSystem->OpenEditorForAsset(Asset);
	}
	return false;
}

void UJavascriptEditorLibrary::OpenEditorForAssetByPath(const FString& AssetPathName, const FString& ObjectName)
{
	// An asset needs loading
	UPackage* Package = LoadPackage(nullptr, *AssetPathName, LOAD_NoRedirects);
	if (Package)
	{
		Package->FullyLoad();

		UObject* Object = FindObject<UObject>(Package, *ObjectName);
		if (Object != nullptr)
		{
			if (auto* SubSystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
			{
				SubSystem->OpenEditorForAsset(Object);
			}
		}
	}
}

TArray<FAssetData> UJavascriptEditorLibrary::GetAssetsByType(const TArray<FString>& Types, bool bRecursiveClasses)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	FARFilter Filter;
	for (auto& Type : Types)
	{
#if ENGINE_MAJOR_VERSION >= 5 && ENGINE_MINOR_VERSION >= 1
		Filter.ClassPaths.Add(FTopLevelAssetPath(Type));
#else
		Filter.ClassNames.Add(FName(*Type));
#endif
	}
	Filter.bRecursiveClasses = bRecursiveClasses;
	TArray<FAssetData> AssetList;
	AssetRegistryModule.Get().GetAssets(Filter, /*out*/AssetList);

	return AssetList;
}

FAnimNotifyEvent& CreateNewNotify(UAnimSequenceBase* Sequence, FString NewNotifyName, UClass* NotifyClass, int32 TrackIndex, float StartTime)
{
	// Insert a new notify record and spawn the new notify object
	int32 NewNotifyIndex = Sequence->Notifies.Add(FAnimNotifyEvent());
	FAnimNotifyEvent& NewEvent = Sequence->Notifies[NewNotifyIndex];
	NewEvent.NotifyName = FName(*NewNotifyName);

	NewEvent.Link(Sequence, StartTime);
	NewEvent.TriggerTimeOffset = GetTriggerTimeOffsetForType(Sequence->CalculateOffsetForNotify(StartTime));
	NewEvent.TrackIndex = TrackIndex;

	if (NotifyClass)
	{
		class UObject* AnimNotifyClass = NewObject<UObject>(Sequence, NotifyClass, NAME_None, RF_Transactional);
		NewEvent.NotifyStateClass = Cast<UAnimNotifyState>(AnimNotifyClass);
		NewEvent.Notify = Cast<UAnimNotify>(AnimNotifyClass);

		// Set default duration to 1 frame for AnimNotifyState.
		if (NewEvent.NotifyStateClass)
		{
			NewEvent.NotifyName = FName(*NewEvent.NotifyStateClass->GetNotifyName());
			NewEvent.SetDuration(1 / 30.f);
			NewEvent.EndLink.Link(Sequence, NewEvent.EndLink.GetTime());
		}
		else
		{
			NewEvent.NotifyName = FName(*NewEvent.Notify->GetNotifyName());
		}
	}
	else
	{
		NewEvent.Notify = nullptr;
		NewEvent.NotifyStateClass = nullptr;
	}

	if (NewEvent.Notify)
	{
		NewEvent.Notify->OnAnimNotifyCreatedInEditor(NewEvent);
	}
	else if (NewEvent.NotifyStateClass)
	{
		NewEvent.NotifyStateClass->OnAnimNotifyCreatedInEditor(NewEvent);
	}

	Sequence->MarkPackageDirty();

	return NewEvent;
}

int32 UJavascriptEditorLibrary::ReplaceAnimNotifyClass(UAnimSequenceBase* Sequence, FString NotifyName, FString NewNotifyName, UObject* NewNotifyClassTemplate)
{
	UClass* NewNotifyClass = NewNotifyClassTemplate->GetClass();
	Sequence->Modify(true);

	TArray<int32> Replaces;
	for (auto i = 0; i < Sequence->Notifies.Num(); i++)
	{
		if (FName(*NotifyName) == Sequence->Notifies[i].NotifyName)
			Replaces.Insert(i, 0);
	}

	///@note : notifies.add�� ���ÿ� ����ǹǷ� replaces�� index�� �������� ���� �Ѵ�. 3->2->1������ �ְ� push_back���� �߰��ǵ���.
	for (auto Index : Replaces)
	{
		auto* OldEvent = &Sequence->Notifies[Index];
		if (OldEvent)
		{
			float BeginTime = OldEvent->GetTime();
			float Length = OldEvent->GetDuration();
			int32 TargetTrackIndex = OldEvent->TrackIndex;
			float TriggerTimeOffset = OldEvent->TriggerTimeOffset;
			float EndTriggerTimeOffset = OldEvent->EndTriggerTimeOffset;
			int32 SlotIndex = OldEvent->GetSlotIndex();
			int32 EndSlotIndex = OldEvent->EndLink.GetSlotIndex();
			int32 SegmentIndex = OldEvent->GetSegmentIndex();
			int32 EndSegmentIndex = OldEvent->GetSegmentIndex();
			EAnimLinkMethod::Type LinkMethod = OldEvent->GetLinkMethod();
			EAnimLinkMethod::Type EndLinkMethod = OldEvent->EndLink.GetLinkMethod();

			// Delete old one before creating new one to avoid potential array re-allocation when array temporarily increases by 1 in size
			for (int32 i = 0; i < Sequence->Notifies.Num(); ++i)
			{
				if (OldEvent == &(Sequence->Notifies[i]))
				{
					Sequence->Notifies.RemoveAt(i);
					Sequence->MarkPackageDirty();
					break;
				}
			}

			FAnimNotifyEvent& NewEvent = CreateNewNotify(Sequence, NewNotifyName, NewNotifyClass, TargetTrackIndex, BeginTime);

			NewEvent.TriggerTimeOffset = TriggerTimeOffset;
			NewEvent.ChangeSlotIndex(SlotIndex);
			NewEvent.SetSegmentIndex(SegmentIndex);
			NewEvent.ChangeLinkMethod(LinkMethod);

			// For Anim Notify States, handle the end time and link
			if (NewEvent.NotifyStateClass != nullptr)
			{
				NewEvent.SetDuration(Length);
				NewEvent.EndTriggerTimeOffset = EndTriggerTimeOffset;
				NewEvent.EndLink.ChangeSlotIndex(EndSlotIndex);
				NewEvent.EndLink.SetSegmentIndex(EndSegmentIndex);
				NewEvent.EndLink.ChangeLinkMethod(EndLinkMethod);
			}

			NewEvent.Update();
		}
	}

	return Replaces.Num();
}

#include "Blueprint/AsyncTaskDownloadImage.h"
#include "Modules/ModuleManager.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture2DDynamic.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Misc/FileHelper.h"

//----------------------------------------------------------------------//
// UAsyncTaskDownloadImage
//----------------------------------------------------------------------//

#if !UE_SERVER

static void WriteRawToTexture_RenderThread(FTexture2DDynamicResource* TextureResource, const TArray64<uint8>& RawData, bool bUseSRGB = true)
{
	check(IsInRenderingThread());

	FRHITexture2D* TextureRHI = TextureResource->GetTexture2DRHI().GetReference();

	int32 Width = TextureRHI->GetSizeX();
	int32 Height = TextureRHI->GetSizeY();

	uint32 DestStride = 0;
	uint8* DestData = reinterpret_cast<uint8*>(RHILockTexture2D(TextureRHI, 0, RLM_WriteOnly, DestStride, false, false));

	for (int32 y = 0; y < Height; y++)
	{
		uint8* DestPtr = &DestData[(Height - 1 - y) * DestStride];

		const FColor* SrcPtr = &((FColor*)(RawData.GetData()))[(Height - 1 - y) * Width];
		for (int32 x = 0; x < Width; x++)
		{
			*DestPtr++ = SrcPtr->B;
			*DestPtr++ = SrcPtr->G;
			*DestPtr++ = SrcPtr->R;
			*DestPtr++ = SrcPtr->A;
			SrcPtr++;
		}
	}

	RHIUnlockTexture2D(TextureRHI, 0, false, false);
}

#endif


void UJavascriptEditorLibrary::DownloadImageFromUrl(const FString& ImageUrl, class UAsyncTaskDownloadImage* Callback)
{
	Callback->Start(ImageUrl);
}

bool UJavascriptEditorLibrary::LoadImageFromDiskAsync(const FString& ImagePath, UAsyncTaskDownloadImage* Callback)
{
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(TEXT("ImageWrapper"));

	if (!FPaths::FileExists(ImagePath))
	{
		///@log error
		Callback->OnFail.Broadcast(nullptr);
		return false;
	}

	TArray<uint8> FileData;
	if (!FFileHelper::LoadFileToArray(FileData, *ImagePath))
	{
		///@log error
		Callback->OnFail.Broadcast(nullptr);
		return false;
	}

	EImageFormat ImageFormat = ImageWrapperModule.DetectImageFormat(FileData.GetData(), FileData.Num());
	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(ImageFormat);

	if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(FileData.GetData(), FileData.Num()))
	{
		TArray64<uint8> RawData;
		if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawData))
		{
			if (UTexture2DDynamic* Texture = UTexture2DDynamic::Create(ImageWrapper->GetWidth(), ImageWrapper->GetHeight()))
			{
				Texture->SRGB = true;
				Texture->UpdateResource();
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 22
				ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
					FWriteRawDataToTexture,
					FTexture2DDynamicResource*, TextureResource, static_cast<FTexture2DDynamicResource*>(Texture->Resource),
					TArray<uint8>, RawData, *RawData,
					{
						WriteRawToTexture_RenderThread(TextureResource, RawData);
					});
#else
				FTexture2DDynamicResource* TextureResource = static_cast<FTexture2DDynamicResource*>(Texture->GetResource());
				TArray64<uint8> RawDataCopy = RawData;
				ENQUEUE_RENDER_COMMAND(FWriteRawDataToTexture)(
					[TextureResource, RawDataCopy](FRHICommandListImmediate& RHICmdList)
				{
					WriteRawToTexture_RenderThread(TextureResource, RawDataCopy);
				});
#endif
				Callback->OnSuccess.Broadcast(Texture);
				return true;
			}
		}

	}

	Callback->OnFail.Broadcast(nullptr);
	return false;
}

#include "IDesktopPlatform.h"
#include "DesktopPlatformModule.h"
#include "EditorDirectories.h"
bool UJavascriptEditorLibrary::OpenFileDialog(const UJavascriptWindow* SubWindow, const FString& DialogTitle, const FString& DefaultPath, const FString& DefaultFile, const FString& FileTypes, int32 Flags, TArray<FString>& OutFilenames)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		const void* ParentWindowWindowHandle = nullptr;
		if (SubWindow)
			ParentWindowWindowHandle = nullptr;
		else
			ParentWindowWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);

		int OutFilterIndex = 0;
		return DesktopPlatform->OpenFileDialog(
			ParentWindowWindowHandle,
			DialogTitle,
			//FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_IMPORT),
			DefaultPath,
			TEXT(""),
			FileTypes,
			EFileDialogFlags::Type(Flags),
			OutFilenames,
			OutFilterIndex
		);
	}

	return false;
}

bool UJavascriptEditorLibrary::OpenDirectoryDialog(const UJavascriptWindow* SubWindow, const FString& DialogTitle, const FString& DefaultPath, FString& OutFolderName)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		const void* ParentWindowWindowHandle = nullptr;
		if (SubWindow)
			ParentWindowWindowHandle = nullptr;
		else
			ParentWindowWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);

		return DesktopPlatform->OpenDirectoryDialog(
			ParentWindowWindowHandle,
			DialogTitle,
			//FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_IMPORT),
			DefaultPath,
			OutFolderName
		);
	}

	return false;
}

bool UJavascriptEditorLibrary::SaveFileDialog(const UJavascriptWindow* SubWindow, const FString& DialogTitle, const FString& DefaultPath, const FString& DefaultFile, const FString& FileTypes, int32 Flags, TArray<FString>& OutFilenames)
{
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		const void* ParentWindowWindowHandle = nullptr;
		if (SubWindow)
		{
			ParentWindowWindowHandle = nullptr;
		}
		else
		{
			ParentWindowWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);
		}

		return DesktopPlatform->SaveFileDialog(
			ParentWindowWindowHandle,
			DialogTitle,
			//FEditorDirectories::Get().GetLastDirectory(ELastDirectory::GENERIC_IMPORT),
			DefaultPath,
			TEXT(""),
			FileTypes,
			EFileDialogFlags::Type(Flags),
			OutFilenames
		);
	}

	return false;
}

void UJavascriptEditorLibrary::OpenCreateBlueprintFromActorDialog(AActor* Actor)
{
	if (::IsValid(Actor))
	{
		FCreateBlueprintFromActorDialog::OpenDialog(ECreateBlueprintFromActorMode::Subclass, Actor);
	}
}

bool UJavascriptEditorLibrary::LoadFileToIntArray(FString Path, TArray<uint8>& FileData)
{
	return FFileHelper::LoadFileToArray(FileData, *Path);
}

bool UJavascriptEditorLibrary::LoadFileToString(FString Path, FString& Data)
{
	return FFileHelper::LoadFileToString(Data, *Path);
}

FString UJavascriptEditorLibrary::GetKeyNameByKeyEvent(const FKeyEvent& Event)
{
	return Event.GetKey().GetFName().ToString();
}

bool UJavascriptEditorLibrary::GetIsControlDownByKeyEvent(const FKeyEvent& Event)
{
	return Event.IsControlDown();
}

bool UJavascriptEditorLibrary::GetIsShiftDownByKeyEvent(const FKeyEvent& Event)
{
	return Event.IsShiftDown();
}

bool UJavascriptEditorLibrary::GetIsAltDownByKeyEvent(const FKeyEvent& Event)
{
	return Event.IsAltDown();
}

FString UJavascriptEditorLibrary::GetDataTableAsJSON(UDataTable* InDataTable, uint8 InDTExportFlags)
{
	if (InDataTable == nullptr)
		return TEXT("");

	return InDataTable->GetTableAsJSON((EDataTableExportFlags)InDTExportFlags);
}

void UJavascriptEditorLibrary::AddRichCurve(UCurveTable* InCurveTable, const FName& Key, const FRichCurve& InCurve)
{
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION < 22
	FRichCurve* NewCurve = new FRichCurve();
	NewCurve->SetKeys(InCurve.GetConstRefOfKeys());
	NewCurve->PreInfinityExtrap = InCurve.PreInfinityExtrap;
	NewCurve->PostInfinityExtrap = InCurve.PostInfinityExtrap;
	NewCurve->DefaultValue = InCurve.DefaultValue;
	InCurveTable->RowMap.Remove(Key);
	InCurveTable->RowMap.Add(Key, NewCurve);
#else
	FRichCurve& NewCurve = InCurveTable->AddRichCurve(Key);
	NewCurve.SetKeys(InCurve.GetConstRefOfKeys());
	NewCurve.PreInfinityExtrap = InCurve.PreInfinityExtrap;
	NewCurve.PostInfinityExtrap = InCurve.PostInfinityExtrap;
	NewCurve.DefaultValue = InCurve.DefaultValue;
#endif
}

bool UJavascriptEditorLibrary::FindRichCurve(UCurveTable* InCurveTable, const FName& Key, FRichCurve& OutCurve)
{
	if (FRichCurve* FoundedCurve = InCurveTable->FindRichCurve(Key, TEXT("UJavascriptEditorLibrary::FindRichCurve"), false))
	{
		OutCurve = *FoundedCurve;
		return true;
	}

	return false;
}

void UJavascriptEditorLibrary::NotifyUpdateCurveTable(UCurveTable* InCurveTable)
{
	InCurveTable->OnCurveTableChanged().Broadcast();
}

bool UJavascriptEditorLibrary::HasMetaData(UField* Field, const FString& Key)
{
	return Field->HasMetaData(*Key);
}

UWorld* UJavascriptEditorLibrary::GetEditorPlayWorld()
{
	return GEditor->PlayWorld;
}

bool UJavascriptEditorLibrary::ToggleIsExecuteTestModePIE()
{
	auto* StaticGameData = Cast<UJavascriptStaticCache>(GEngine->GameSingleton);
	if (StaticGameData)
	{
		StaticGameData->bExecuteTestModePIE ^= true;
		return StaticGameData->bExecuteTestModePIE;
	}

	return false;
}

bool UJavascriptEditorLibrary::GetIsExecuteTestModePIE()
{
	auto* StaticGameData = Cast<UJavascriptStaticCache>(GEngine->GameSingleton);
	if (StaticGameData)
	{
		return StaticGameData->bExecuteTestModePIE;
	}

	return false;
}

int32 UJavascriptEditorLibrary::GetUniqueID(UObject * InObject)
{
	if (InObject != nullptr)
	{
		return InObject->GetUniqueID();
	}
	else
	{
		return -1;
	}
}

void UJavascriptEditorLibrary::SetActorLabelUnique(AActor* Actor, const FString& NewActorLabel, const TArray<FString>& InExistingActorLabels)
{
	FCachedActorLabels ActorLabels;
	for (auto& ExistingActorLabel : InExistingActorLabels)
	{
		ActorLabels.Add(ExistingActorLabel);
	}

	FActorLabelUtilities::SetActorLabelUnique(Actor, NewActorLabel, &ActorLabels);
}

bool UJavascriptEditorLibrary::EditorExec(UWorld* World, FString Cmd)
{
	return GEditor->Exec(World, *Cmd, *GLog);
}

FJavascriptTextProperty UJavascriptEditorLibrary::FromStringTable(const FName InTableId, const FString& InKey)
{
	FText TmpText = FText::FromStringTable(InTableId, InKey);
	FJavascriptTextProperty Result;
	Result.TableId = InTableId;
	Result.Namespace = FTextInspector::GetNamespace(TmpText).Get(FString());
	Result.Key = FTextInspector::GetKey(TmpText).Get(FString());
	Result.Value = TmpText.ToString();
	return Result;
}

float UJavascriptEditorLibrary::GetAverageFPS()
{
	extern ENGINE_API float GAverageFPS;
	return GAverageFPS;
}

float UJavascriptEditorLibrary::GetAverageMS()
{
	extern ENGINE_API float GAverageMS;
	return GAverageMS;
}

bool UJavascriptEditorLibrary::CheckActivatedStatGroup(FName GroupName)
{
#if STATS
	if (FGameThreadStatsData* StatsData = FLatestGameThreadStatsData::Get().Latest)
	{
		const FString GroupNameString = FString(TEXT("STATGROUP_")) + GroupName.ToString();
		const FName GroupNameFull = FName(*GroupNameString, EFindName::FNAME_Find);

		if (StatsData->GroupNames.Contains(GroupNameFull))
		{
			return true;
		}
	}

#endif
	return false;
}

FString UJavascriptEditorLibrary::GetSourceControlStatusText()
{
	ISourceControlModule& SourceControlModule = FModuleManager::LoadModuleChecked<ISourceControlModule>("SourceControl");
	return SourceControlModule.GetProvider().GetStatusText().BuildSourceString();
}

UJavascriptEditorObjectManager* UJavascriptEditorLibrary::GetEditorObjectManager()
{
	UJavascriptEditorObjectManager* EditorObjectManager = nullptr;
	if (IJavascriptEditorModule* JSEditorModule = FModuleManager::GetModulePtr<IJavascriptEditorModule>("JavascriptEditor"))
	{
		EditorObjectManager = JSEditorModule->GetEditorObjectManager();
	}
	return EditorObjectManager;
}

FString UJavascriptEditorLibrary::GetHostName()
{
	ISocketSubsystem* const SocketSubSystem = ISocketSubsystem::Get();
	FString HostName = "";

	if (SocketSubSystem)
	{
		SocketSubSystem->GetHostName(HostName);
	}

	return HostName;
}

FString UJavascriptEditorLibrary::GetIPAddress()
{
	ISocketSubsystem* const SocketSubSystem = ISocketSubsystem::Get();
	FString IPAddress = "";

	if (SocketSubSystem)
	{
		bool canBind = false;
		TSharedRef<FInternetAddr> localIp = SocketSubSystem->GetLocalHostAddr(*GLog, canBind);
		if (localIp->IsValid()) IPAddress = localIp->ToString(false);
	}

	return IPAddress;
}

#endif
