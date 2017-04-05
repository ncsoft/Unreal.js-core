#include "JavascriptEditor.h"
#include "JavascriptRawMeshLibrary.h"

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
	return (*RawMesh).GetWedgePosition(WedgeIndex);
}

void UJavascriptRawMeshLibrary::CompactMaterialIndices(FJavascriptRawMesh& RawMesh)
{
	(*RawMesh).CompactMaterialIndices();
}

void UJavascriptRawMeshLibrary::SaveRawMesh(UStaticMesh* StaticMesh, int32 SourceModelIndex, FJavascriptRawMesh& InMesh)
{
	StaticMesh->SourceModels[SourceModelIndex].RawMeshBulkData->SaveRawMesh(*InMesh);
}

void UJavascriptRawMeshLibrary::LoadRawMesh(UStaticMesh* StaticMesh, int32 SourceModelIndex, FJavascriptRawMesh& OutMesh)
{
	StaticMesh->SourceModels[SourceModelIndex].RawMeshBulkData->LoadRawMesh(*OutMesh);
}

void UJavascriptRawMeshLibrary::Build(UStaticMesh* StaticMesh)
{
	StaticMesh->Build();
}

FMeshSectionInfo UJavascriptRawMeshLibrary::GetSectionInfo(UStaticMesh* StaticMesh, int32 LODIndex, int32 SectionIndex)
{
	return StaticMesh->SectionInfoMap.Get(0, SectionIndex);
}

void UJavascriptRawMeshLibrary::SetSectionInfo(UStaticMesh* StaticMesh, int32 LODIndex, int32 SectionIndex, const FMeshSectionInfo& Info)
{
	StaticMesh->SectionInfoMap.Set(LODIndex, SectionIndex, Info);
}
#endif