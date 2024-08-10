#include "QuadTree.h"
UQuadTreeComponent::UQuadTreeComponent()
{
    ProceduralMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralMesh"));
}

void UQuadTreeComponent::InitializeQuadTree(const FVector2D& Origin, float InitialSize)
{
    NoiseFunc = new FastNoiseLite;
    NoiseFunc->SetSeed(1337);
    NoiseFunc->SetNoiseType(FastNoiseLite::NoiseType_Cellular); // Tipo de ruido
    NoiseFunc->SetFrequency(0.0001); // Frecuencia del ruido
    
    RootNode = FQuadTreeNode(Origin, InitialSize, InitialSize);

    // Inicializar el QuadTree con un Depth de 3
  
    InitializeNodeRecursive(RootNode);

    GenerateMesh(RootNode);  // Generar la malla después de la subdivisión inicial
    IsInit = true;
}

void UQuadTreeComponent::InitializeNodeRecursive(FQuadTreeNode& Node)
{
    if (Node.Depth < InitialDepth)
    {
        Node.Subdivide();
        for (FQuadTreeNode& Child : Node.Children)
        {
            Child.Depth = Node.Depth + 1;
            Child.InitialSize = Node.InitialSize;

            InitializeNodeRecursive(Child);
        }
    }
}

void UQuadTreeComponent::UpdateQuadTree(const FVector& CameraLocation, float SubdivisionThreshold)
{
    SubdivideNode(RootNode, CameraLocation, SubdivisionThreshold);
    GenerateMesh(RootNode); 
}

void UQuadTreeComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    // Nombre de la propiedad que cambió
    FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

    // Si alguna propiedad relevante cambió, reconstruir el QuadTree
    if (PropertyName == GET_MEMBER_NAME_CHECKED(UQuadTreeComponent, InitialDepth) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(UQuadTreeComponent, Height) ||
        PropertyName == GET_MEMBER_NAME_CHECKED(UQuadTreeComponent, MaxDepth))
    {
        // Limpiar el árbol actual
        RootNode.Children.Empty();
        RootNode.Size = 10000;
        RootNode.Depth = 0;

        // Inicializar de nuevo el QuadTree
        InitializeQuadTree(FVector2D::ZeroVector, 10000);
    }
}


void FQuadTreeNode::Subdivide()
{
    float HalfSize = Size / 2.0f;
    
    Children.Add(FQuadTreeNode(FVector2D(Position.X, Position.Y), HalfSize, InitialSize));
    Children.Add(FQuadTreeNode(FVector2D(Position.X + HalfSize, Position.Y), HalfSize, InitialSize));
    Children.Add(FQuadTreeNode(FVector2D(Position.X, Position.Y + HalfSize), HalfSize, InitialSize));
    Children.Add(FQuadTreeNode(FVector2D(Position.X + HalfSize, Position.Y + HalfSize), HalfSize, InitialSize));
}

void UQuadTreeComponent::SubdivideNode(FQuadTreeNode& Node, const FVector& CameraLocation, float SubdivisionThreshold)
{
    FVector ActorLocation = GetOwner()->GetActorLocation();
    FVector2D NodeCenter = FVector2D(ActorLocation.X, ActorLocation.Y) + Node.Position + FVector2D(Node.Size / 2.0f, Node.Size / 2.0f);
    float DistanceToCamera = FVector2D::Distance(NodeCenter, FVector2D(CameraLocation.X, CameraLocation.Y));
    
    if (DistanceToCamera < SubdivisionThreshold && Node.Size > 50.0f)
    {
        if (Node.Children.Num() == 0 && Node.Depth < MaxDepth)
        {
            Node.Subdivide();
           
            for (FQuadTreeNode& Child : Node.Children)
            {
                Child.Depth = Node.Depth + 1;
                Child.InitialSize = Node.InitialSize;
            }
        }
        
        for (FQuadTreeNode& Child : Node.Children)
        {
            SubdivideNode(Child, CameraLocation, SubdivisionThreshold);
        }
    }
    else
    {
        if (Node.Children.Num() > 0)
        {
            bool ShouldCollapse = true;
            
            for (FQuadTreeNode& Child : Node.Children)
            {
                if (Child.Depth < InitialDepth || FVector2D::Distance(FVector2D(CameraLocation.X, CameraLocation.Y), Child.Position) < SubdivisionThreshold)
                {
                    ShouldCollapse = false;
                    break;
                }
            }

            if (ShouldCollapse)
            {
                Node.Children.Empty();
            }
            else
            {
                for (FQuadTreeNode& Child : Node.Children)
                {
                    SubdivideNode(Child, CameraLocation, SubdivisionThreshold);
                }
            }
        }
    }
}

void UQuadTreeComponent::GenerateMesh(FQuadTreeNode& Node)
{
    TArray<FVector> AccumulatedVertices;
    TArray<int32> AccumulatedIndices;
    TMap<FVector, int32> VertexMap; // Declara el mapa de vértices

    GenerateMeshRecursive(Node, AccumulatedVertices, AccumulatedIndices, VertexMap);

    ProceduralMesh->CreateMeshSection(0, AccumulatedVertices, AccumulatedIndices, TArray<FVector>(), TArray<FVector2D>(), TArray<FColor>(), TArray<FProcMeshTangent>(), true);
}

void UQuadTreeComponent::AddVertex(const FVector& Vertex, TArray<FVector>& OutVertices, TMap<FVector, int32>& VertexMap, int32& OutVertexIndex)
{
    int32* ExistingIndex = VertexMap.Find(Vertex);
    if (ExistingIndex)
    {
        OutVertexIndex = *ExistingIndex;
    }
    else
    {
        OutVertexIndex = OutVertices.Num();
        VertexMap.Add(Vertex, OutVertexIndex);
        OutVertices.Add(Vertex);
    }
}

void UQuadTreeComponent::GenerateMeshRecursive(FQuadTreeNode& Node, TArray<FVector>& OutVertices, TArray<int32>& OutIndices, TMap<FVector, int32>& VertexMap)
{
    if (Node.Children.Num() == 0)
    {
        int32 VertexIndex;

        FVector BottomLeft = FVector(Node.Position, 0.0f);
        FVector BottomRight = FVector(Node.Position + FVector2D(Node.Size, 0.0f), 0.0f);
        FVector TopLeft = FVector(Node.Position + FVector2D(0.0f, Node.Size), 0.0f);
        FVector TopRight = FVector(Node.Position + FVector2D(Node.Size, Node.Size), 0.0f);

        // Generar valores de Z usando el ruido
        BottomLeft.Z = NoiseFunc->GetNoise(BottomLeft.X, BottomLeft.Y) * Height; 
        BottomRight.Z = NoiseFunc->GetNoise(BottomRight.X, BottomRight.Y) * Height;
        TopLeft.Z = NoiseFunc->GetNoise(TopLeft.X, TopLeft.Y) * Height;
        TopRight.Z = NoiseFunc->GetNoise(TopRight.X, TopRight.Y) * Height;

        // Añadir vértices
        AddVertex(BottomLeft, OutVertices, VertexMap, VertexIndex);
        int32 BottomLeftIndex = VertexIndex;

        AddVertex(BottomRight, OutVertices, VertexMap, VertexIndex);
        int32 BottomRightIndex = VertexIndex;

        AddVertex(TopLeft, OutVertices, VertexMap, VertexIndex);
        int32 TopLeftIndex = VertexIndex;

        AddVertex(TopRight, OutVertices, VertexMap, VertexIndex);
        int32 TopRightIndex = VertexIndex;

        // Añadir índices para los triángulos
        OutIndices.Add(BottomLeftIndex);
        OutIndices.Add(TopLeftIndex);
        OutIndices.Add(BottomRightIndex);

        OutIndices.Add(TopLeftIndex);
        OutIndices.Add(TopRightIndex);
        OutIndices.Add(BottomRightIndex);
    }

    for (FQuadTreeNode& Child : Node.Children)
    {
        GenerateMeshRecursive(Child, OutVertices, OutIndices, VertexMap);
    }
}


