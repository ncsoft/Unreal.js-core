#include "JavascriptRawMeshLibrary.h"
#include "PhysicsEngine/BodySetup.h"
#include "Engine/SkeletalMesh.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Components/StaticMeshComponent.h"

#if WITH_EDITOR
void UJavascriptRawMeshLibrary::Empty(FJavascriptRawMesh& RawMesh)
{
	(*RawMesh).Empty();
}

bool UJavascriptRawMeshLibrary::IsValid(const FJavascriptRawMesh& RawMesh)
{
	return (*RawMesh).IsValid();
}

bool UJavascriptRawMeshLibrary::IsValidOrFixable(const FJavascriptRawMesh& RawMesh)
{
	return (*RawMesh).IsValidOrFixable();
}

FVector UJavascriptRawMeshLibrary::GetWedgePosition(const FJavascriptRawMesh& RawMesh, int32 WedgeIndex)
{
	return FVector((*RawMesh).GetWedgePosition(WedgeIndex));
}

void UJavascriptRawMeshLibrary::CompactMaterialIndices(FJavascriptRawMesh& RawMesh)
{
	(*RawMesh).CompactMaterialIndices();
}

void UJavascriptRawMeshLibrary::SaveRawMesh(UStaticMesh* StaticMesh, int32 SourceModelIndex, FJavascriptRawMesh& InMesh)
{
	StaticMesh->GetSourceModel(SourceModelIndex).RawMeshBulkData->SaveRawMesh(*InMesh);
}

void UJavascriptRawMeshLibrary::LoadRawMesh(UStaticMesh* StaticMesh, int32 SourceModelIndex, FJavascriptRawMesh& OutMesh)
{
	StaticMesh->GetSourceModel(SourceModelIndex).RawMeshBulkData->LoadRawMesh(*OutMesh);
}

void UJavascriptRawMeshLibrary::Build(UStaticMesh* StaticMesh)
{
	StaticMesh->Build();
}

UBodySetup* UJavascriptRawMeshLibrary::GetPhysicsBodySetupFromMesh(USkeletalMesh* InSkeletalMesh, FString InName)
{
	check(InSkeletalMesh);

	UPhysicsAsset* const PhysicsAsset = InSkeletalMesh->GetPhysicsAsset();
	if (::IsValid(PhysicsAsset))
	{
		int32 BodyIndex = PhysicsAsset->FindBodyIndex(FName(*InName));
		if (BodyIndex != INDEX_NONE)
		{
			return PhysicsAsset->SkeletalBodySetups[BodyIndex];
		}
	}

	return nullptr;
}

UBodySetup* UJavascriptRawMeshLibrary::GetPhysicsBodySetupFromStaticMesh(UStaticMesh* InStaticMesh)
{
	if (!::IsValid(InStaticMesh))
	{
		return nullptr;
	}

	return InStaticMesh->GetBodySetup();
}


UBodySetup* UJavascriptRawMeshLibrary::GetPhysicsBodySetup(USkeletalMeshComponent* InSkeletalMeshComp, FString InName)
{
	check(InSkeletalMeshComp);

	UPhysicsAsset* const PhysicsAsset = InSkeletalMeshComp->GetPhysicsAsset();
	if (PhysicsAsset)
	{
		int32 BodyIndex = PhysicsAsset->FindBodyIndex(FName(*InName));
		if (BodyIndex != INDEX_NONE)
		{
			return PhysicsAsset->SkeletalBodySetups[BodyIndex];
		}
	}

	return nullptr;
}

UBodySetup* UJavascriptRawMeshLibrary::GetPhysicsBodySetupFromStaticMeshComponent(UStaticMeshComponent* InStaticMeshComp)
{
	return ::IsValid(InStaticMeshComp) ? InStaticMeshComp->GetBodySetup() : nullptr;
}

FMeshSectionInfo UJavascriptRawMeshLibrary::GetSectionInfo(UStaticMesh* StaticMesh, int32 LODIndex, int32 SectionIndex)
{
	return StaticMesh->GetSectionInfoMap().Get(0, SectionIndex);
}

void UJavascriptRawMeshLibrary::SetSectionInfo(UStaticMesh* StaticMesh, int32 LODIndex, int32 SectionIndex, const FMeshSectionInfo& Info)
{
	StaticMesh->GetSectionInfoMap().Set(LODIndex, SectionIndex, Info);
}
#endif
