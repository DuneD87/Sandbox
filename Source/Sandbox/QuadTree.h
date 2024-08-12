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

UENUM(BlueprintType)
enum class NoiseType : uint8
{
    Cellular = 0,
    Perlin = 1,
    Value = 2,
    OpenSimplex2 = 3,
    ValueCubic = 4,
    OpenSimplex2S = 5
};

UENUM(BlueprintType)
enum class NoiseFractalTypes : uint8
{
    None = 0,
    FBm = 1,
    Rigid = 2,
    PingPong = 3,
    DomainWarpProgressive = 4,
    DomainWarpIndependent = 5
};
UCLASS()
class SANDBOX_API UQuadTreeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    
    UQuadTreeComponent();
    UPROPERTY(EditAnywhere, Category = "Noise", DisplayName = "NoiseType")
    NoiseType NoiseType {NoiseType::Cellular};
    
    UPROPERTY(EditAnywhere, Category = "Noise", DisplayName = "NoiseFractalType")
    NoiseFractalTypes NoiseFractalType {NoiseFractalTypes::None};
    
    UPROPERTY(EditAnywhere, Category = "Noise", DisplayName = "NoiseFrequency")
    float NoiseFrequency {0.0001};
    
    UPROPERTY(EditAnywhere, Category = "Noise", DisplayName = "CellularJitter")
    float CellularJitter {1};
    
    UPROPERTY(EditAnywhere, Category = "Noise", DisplayName = "FractalGain")
    float FractalGain {0.5};
    
    UPROPERTY(EditAnywhere, Category = "Noise", DisplayName = "FractalLacunarity")
    float FractalLacunarity {2.0};

    UPROPERTY(EditAnywhere, Category = "Noise", DisplayName = "FractalWeightedStrength")
    float FractalWeightedStrength {0};

    UPROPERTY(EditAnywhere, Category = "Noise", DisplayName = "FractalOctaves")
    int FractalOctaves {3};
    
    UPROPERTY(EditAnywhere, Category = "Noise", DisplayName = "PingPongStrength")
    float PingPongStrength {2.0};
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="QuadTreeComponent")
    float Height {5000.0f};

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="QuadTreeComponent")
    int InitialDepth {3};

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
    float DefaultSize;
};