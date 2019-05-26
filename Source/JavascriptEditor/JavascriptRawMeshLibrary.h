#pragma once

#include "RawMesh.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/StaticMesh.h"
#include "JavascriptRawMeshLibrary.generated.h"

USTRUCT(BlueprintType)
struct FJavascriptRawMesh
{
	GENERATED_BODY()

	FRawMesh& operator * ()
	{
		return *reinterpret_cast<FRawMesh*>(this);
	}

	const FRawMesh& operator * () const
	{
		return *reinterpret_cast<const FRawMesh*>(this);
	}

	/** Material index. Array[FaceId] = int32 */
	UPROPERTY()
	TArray<int32> FaceMaterialIndices;

	/** Smoothing mask. Array[FaceId] = uint32 */
	UPROPERTY()
	TArray<uint32> FaceSmoothingMasks;

	/** Position in local space. Array[VertexId] = float3(x,y,z) */
	UPROPERTY()
	TArray<FVector> VertexPositions;

	/** Index of the vertex at this wedge. Array[WedgeId] = VertexId */
	UPROPERTY()
	TArray<uint32> WedgeIndices;

	/** Tangent, U direction. Array[WedgeId] = float3(x,y,z) */
	UPROPERTY()
	TArray<FVector>	WedgeTangentX;

	/** Tangent, V direction. Array[WedgeId] = float3(x,y,z) */
	UPROPERTY()
	TArray<FVector>	WedgeTangentY;

	/** Normal. Array[WedgeId] = float3(x,y,z) */
	UPROPERTY()
	TArray<FVector>	WedgeTangentZ;

	/** Texture coordinates. Array[UVId][WedgeId]=float2(u,v) */
	UPROPERTY()
	TArray<FVector2D> WedgeTexCoords_0;

	UPROPERTY()
	TArray<FVector2D> WedgeTexCoords_1;

	UPROPERTY()
	TArray<FVector2D> WedgeTexCoords_2;

	UPROPERTY()
	TArray<FVector2D> WedgeTexCoords_3;

	UPROPERTY()
	TArray<FVector2D> WedgeTexCoords_4;

	UPROPERTY()
	TArray<FVector2D> WedgeTexCoords_5;

	UPROPERTY()
	TArray<FVector2D> WedgeTexCoords_6;

	UPROPERTY()
	TArray<FVector2D> WedgeTexCoords_7;
	
	/** Color. Array[WedgeId]=float3(r,g,b,a) */
	UPROPERTY()
	TArray<FColor> WedgeColors;

	/**
	* Map from material index -> original material index at import time. It's
	* valid for this to be empty in which case material index == original
	* material index.
	*/
	UPROPERTY()
	TArray<int32> MaterialIndexToImportIndex;
};

UCLASS()
class JAVASCRIPTEDITOR_API UJavascriptRawMeshLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void Empty(FJavascriptRawMesh& RawMesh);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool IsValid(const FJavascriptRawMesh& RawMesh);

	/**
	* Returns true if the mesh contains valid information or slightly invalid information that we can fix.
	*  - Validates that stream sizes match.
	*  - Validates that there is at least one texture coordinate.
	*  - Validates that indices are valid positions in the vertex stream.
	*/
	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static bool IsValidOrFixable(const FJavascriptRawMesh& RawMesh);

	/** Helper for getting the position of a wedge. */
	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static FVector GetWedgePosition(const FJavascriptRawMesh& RawMesh, int32 WedgeIndex);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void CompactMaterialIndices(FJavascriptRawMesh& RawMesh);

	/** Store a new raw mesh in the bulk data. */
	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void SaveRawMesh(UStaticMesh* StaticMesh, int32 SourceModelIndex, FJavascriptRawMesh& InMesh);

	/** Load the raw mesh from bulk data. */
	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void LoadRawMesh(UStaticMesh* StaticMesh, int32 SourceModelIndex, FJavascriptRawMesh& OutMesh);

	UFUNCTION(BlueprintInternalUseOnly, Category = "Scripting | Javascript")
	static FMeshSectionInfo GetSectionInfo(UStaticMesh* StaticMesh, int32 LODIndex, int32 SectionIndex);
	
	UFUNCTION(BlueprintInternalUseOnly, Category = "Scripting | Javascript")
	static void SetSectionInfo(UStaticMesh* StaticMesh, int32 LODIndex, int32 SectionIndex, const FMeshSectionInfo& Info);

	UFUNCTION(BlueprintCallable, Category = "Scripting | Javascript")
	static void Build(UStaticMesh* StaticMesh);
#endif
};
