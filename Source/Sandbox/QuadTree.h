#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProceduralMeshComponent.h"
#include "FastNoiseLite.h"

#include "QuadTree.generated.h"

USTRUCT()
struct FQuadTreeNode
{
    GENERATED_BODY()

    FVector2D Position;
    float Size;
    float InitialSize;
    int32 Depth;
    TArray<FQuadTreeNode> Children;
    FQuadTreeNode()
           : Position(FVector2D(0.0f, 0.0f)), Size(0.0f), InitialSize(0.0f), Depth(0)
    {}

    FQuadTreeNode(FVector2D InPosition, float InSize, float initialSize)
        : Position(InPosition), Size(InSize), InitialSize(initialSize), Depth(0) 
    {}

    void Subdivide();
};

UCLASS()
class SANDBOX_API UQuadTreeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuadTreeComponent();
    bool IsInit = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="QuadTreeComponent")
    float Height {10000.0f};

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="QuadTreeComponent")
    int InitialDepth {4};

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="QuadTreeComponent")
    int MaxDepth {8};
    UPROPERTY(VisibleAnywhere)
    class UProceduralMeshComponent* ProceduralMesh;

    void InitializeQuadTree(const FVector2D& Origin, float InitialSize);
    void UpdateQuadTree(const FVector& CameraLocation, float SubdivisionThreshold);
    void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    

private:
    void InitializeNodeRecursive(FQuadTreeNode& Node);
    FQuadTreeNode RootNode;
    void SubdivideNode(FQuadTreeNode& Node, const FVector& CameraLocation, float SubdivisionThreshold);
    void GenerateMesh(FQuadTreeNode& Node);
    void GenerateMeshRecursive(FQuadTreeNode& Node, TArray<FVector>& OutVertices, TArray<int32>& OutIndices, TMap<FVector, int32>& VertexMap);
    void AddVertex(const FVector& Vertex, TArray<FVector>& OutVertices, TMap<FVector, int32>& VertexMap, int32& OutVertexIndex);
    FastNoiseLite* NoiseFunc;
};