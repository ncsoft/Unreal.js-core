#include "JavascriptEditorLibrary.h"
#include "LandscapeComponent.h"

// WORKAROUND for 4.15
#ifndef WITH_KISSFFT
#define WITH_KISSFFT 0
#endif

#include "Editor/LandscapeEditor/Private/LandscapeEdModeTools.h"
#include "JavascriptContext.h"
#include "DynamicMeshBuilder.h"
#include "BSPOps.h"
#include "HotReloadInterface.h"
#include "JavascriptWindow.h"
#include "Editor/PropertyEditor/Public/PropertyEditorModule.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "LevelEditor.h"
#include "Landscape.h"
#include "LandscapeDataAccess.h"
#include "LandscapeEdit.h"
#include "Engine/BrushBuilder.h"
#include "Engine/Selection.h"
#include "EngineUtils.h"
#include "GameFramework/Volume.h"
#include "Components/BrushComponent.h"
#include "../../Launch/Resources/Version.h"
#include "PlatformFileManager.h"
#include "FileManager.h"
#include "NavDataGenerator.h"
#include "SlateApplication.h"
#include "Engine/LevelStreaming.h"
#include "VisualLogger/VisualLogger.h"

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

void UJavascriptEditorLibrary::OpenPopupWindow(UWidget* Widget, const FVector2D& PopupDesiredSize, const FText& HeadingText)
{
	// Create the contents of the popup
	TSharedRef<SWidget> ActualWidget = Widget->TakeWidget();

	// Wrap the picker widget in a multibox-style menu body
	FMenuBuilder MenuBuilder(/*BShouldCloseAfterSelection=*/ false, /*CommandList=*/ nullptr);
	MenuBuilder.BeginSection("OpenPopupWindow", HeadingText);
	MenuBuilder.AddWidget(ActualWidget, FText::GetEmpty(), /*bNoIndent=*/ true);
	MenuBuilder.EndSection();

	auto WindowContents = MenuBuilder.MakeWidget();

	// Determine where the pop-up should open
	TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
	FVector2D WindowPosition = FSlateApplication::Get().GetCursorPos();
	if (!ParentWindow.IsValid())
	{
		return;
	}

	if (ParentWindow.IsValid())
	{
		FSlateRect ParentMonitorRect = ParentWindow->GetFullScreenInfo();
		const FVector2D MonitorCenter((ParentMonitorRect.Right + ParentMonitorRect.Left) * 0.5f, (ParentMonitorRect.Top + ParentMonitorRect.Bottom) * 0.5f);
		WindowPosition = MonitorCenter - PopupDesiredSize * 0.5f;

		// Open the pop-up
		FPopupTransitionEffect TransitionEffect(FPopupTransitionEffect::None);
		auto Menu = FSlateApplication::Get().PushMenu(ParentWindow.ToSharedRef(), FWidgetPath(), WindowContents, WindowPosition, TransitionEffect, /*bFocusImmediately=*/ true);
	}
}

void UJavascriptEditorLibrary::GetAllTags(const FJavascriptAssetData& AssetData, TArray<FName>& OutArray)
{
	AssetData.SourceAssetData.TagsAndValues.GetKeys(OutArray);
}

bool UJavascriptEditorLibrary::GetTagValue(const FJavascriptAssetData& AssetData, const FName& Name, FString& OutValue)
{
	auto Value = AssetData.SourceAssetData.TagsAndValues.Find(Name);

	if (Value)
	{
		OutValue = *Value;
		return true;
	}
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
	return World->EditorDestroyActor(Actor, bShouldModifyLevel);
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
void UJavascriptEditorLibrary::DrawPolygon(const FJavascriptPDI& PDI, const TArray<FVector>& Verts, const FLinearColor& InColor, ESceneDepthPriorityGroup DepthPriority)
{
	FDynamicMeshBuilder MeshBuilder;

	FColor Color = InColor.ToFColor(false);

	for (const auto& V : Verts)
	{
		MeshBuilder.AddVertex(V, FVector2D(0, 0), FVector(1, 0, 0), FVector(0, 1, 0), FVector(0, 0, 1), Color);
	}

	for (int32 Index = 0; Index < Verts.Num() - 2; ++Index)
	{
		MeshBuilder.AddTriangle(0, Index + 1, Index + 2);		
	}
	
	static auto TransparentPlaneMaterialXY = (UMaterial*)StaticLoadObject(UMaterial::StaticClass(), NULL, TEXT("/Engine/EditorMaterials/WidgetVertexColorMaterial.WidgetVertexColorMaterial"), NULL, LOAD_None, NULL);
	MeshBuilder.Draw(PDI.PDI, FMatrix::Identity, TransparentPlaneMaterialXY->GetRenderProxy(false), DepthPriority, 0.f);
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
		if (ActorHit->Actor != NULL)
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
	IHotReloadInterface& HotReloadSupport = FModuleManager::LoadModuleChecked<IHotReloadInterface>("HotReload");
	HotReloadSupport.OnHotReload().Broadcast(false);
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
	return {StaticCastSharedPtr<SWidget>(FGlobalTabmanager::Get()->GetRootWindow())};
}

void UJavascriptEditorLibrary::CreatePropertyEditorToolkit(TArray<UObject*> ObjectsForPropertiesMenu)
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.CreatePropertyEditorToolkit(EToolkitMode::Standalone, TSharedPtr<IToolkitHost>(), ObjectsForPropertiesMenu);
}

static FName NAME_LevelEditor("LevelEditor");
static FName NAME_MaterialEditor("MaterialEditor");

FJavascriptExtensibilityManager UJavascriptEditorLibrary::GetMenuExtensibilityManager(FName What)
{
	if (What == NAME_LevelEditor)
	{
		FLevelEditorModule& LevelEditor = FModuleManager::LoadModuleChecked<FLevelEditorModule>(NAME_LevelEditor);
		return {LevelEditor.GetMenuExtensibilityManager()};
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
	return FJavascriptExtensibilityManager();
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

bool UJavascriptEditorLibrary::SavePackage(UPackage* Package, FString FileName)
{
	UWorld* World = UWorld::FindWorldInPackage(Package);
	bool bSavedCorrectly;

	if (World) 
	{
		bSavedCorrectly = UPackage::SavePackage(Package, World, RF_NoFlags, *FileName, GError, NULL, false, true);
	}
	else
	{
		bSavedCorrectly =  UPackage::SavePackage(Package, NULL, RF_Standalone, *FileName, GError, NULL, false, true);
	}
	return bSavedCorrectly;
}

bool UJavascriptEditorLibrary::DeletePackage(UPackage* Package)
{
	FString PackageName = Package->GetName();
	FString BasePackageFileName = FPackageName::LongPackageNameToFilename(PackageName);
	if (FPlatformFileManager::Get().GetPlatformFile().FileExists(*BasePackageFileName))
	{
		return IFileManager::Get().Delete(*BasePackageFileName);
	}
	return false;
}

void UJavascriptEditorLibrary::CreateBrushForVolumeActor(AVolume* NewActor, UBrushBuilder* BrushBuilder)
{
	if (NewActor != NULL)
	{
		// this code builds a brush for the new actor
		NewActor->PreEditChange(NULL);

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

		// Set the texture on all polys to NULL.  This stops invisible textures
		// dependencies from being formed on volumes.
		if (NewActor->Brush)
		{
			for (int32 poly = 0; poly < NewActor->Brush->Polys->Element.Num(); ++poly)
			{
				FPoly* Poly = &(NewActor->Brush->Polys->Element[poly]);
				Poly->Material = NULL;
			}
		}

		NewActor->PostEditChange();
	}
}

UWorld* UJavascriptEditorLibrary::FindWorldInPackage(UPackage* Package)
{
	return UWorld::FindWorldInPackage(Package);
}

FString UJavascriptEditorLibrary::ExportNavigation(UWorld* InWorld, FString Name)
{
	InWorld->WorldType = EWorldType::Editor;
	InWorld->AddToRoot();
	if (!InWorld->bIsWorldInitialized)
	{
		UWorld::InitializationValues IVS;
		IVS.EnableTraceCollision(true);
		IVS.CreateNavigation(false);

		InWorld->InitWorld(IVS);
		InWorld->PersistentLevel->UpdateModelComponents();
		InWorld->UpdateWorldComponents(true, false);
		InWorld->UpdateLevelStreaming();
		//InWorld->LoadSecondaryLevels(true, NULL);
	}

	UNavigationSystem::InitializeForWorld(InWorld, FNavigationSystemRunMode::EditorMode);
	FWorldContext &WorldContext = GEditor->GetEditorWorldContext(true);
	WorldContext.SetCurrentWorld(InWorld);
	GWorld = InWorld;

//	UGameplayStatics::LoadStreamLevel(InWorld, FName(TEXT("BackgroundMountains")), true, true, FLatentActionInfo());
// 	UWorld::InitializationValues().ShouldSimulatePhysics(false).EnableTraceCollision(true).CreateNavigation(InWorldType == EWorldType::Editor).CreateAISystem(InWorldType == EWorldType::Editor);
// 	UNavigationSystem::InitializeForWorld(InWorld, FNavigationSystemRunMode::GameMode);
	if (InWorld->GetNavigationSystem())
	{
		InWorld->GetNavigationSystem()->Build();
		if (const ANavigationData* NavData = InWorld->GetNavigationSystem()->GetMainNavData())
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
	World->bShouldForceUnloadStreamingLevels = true;
	World->bShouldForceVisibleStreamingLevels = false;
	for (ULevelStreaming* StreamingLevel : World->StreamingLevels)
	{
		StreamingLevel->bIsRequestingUnloadAndRemoval = true;
	}
	World->RefreshStreamingLevels();
}

void UJavascriptEditorLibrary::AddWhitelistedObject(UObject* InObject)
{
	FVisualLogger::Get().AddWhitelistedObject(*InObject);
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
#endif