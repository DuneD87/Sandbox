#include "QuadTreeActor.h"

#include "Editor.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/UnrealEditorSubsystem.h"

AQuadTreeActor::AQuadTreeActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	
	QuadTreeComponent = CreateDefaultSubobject<UQuadTreeComponent>(TEXT("QuadTreeComponent"));
	RootComponent = QuadTreeComponent->ProceduralMesh;
	SubdivisionThreshold = 10000.0f;
}

void AQuadTreeActor::BeginPlay()
{
	Super::BeginPlay();
}

void AQuadTreeActor::OnConstruction(const FTransform& Transform)
{
	if (QuadTreeComponent)
	{
		QuadTreeComponent->InitializeQuadTree(FVector2D(0.0f, 0.0f), 100000.0f);
	}
	Super::OnConstruction(Transform);
}

void AQuadTreeActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateQuadTree();
}

void AQuadTreeActor::UpdateQuadTree()
{
	FVector CurrCameraPosition;
	FRotator CurrCameraRotator;
	
	if (GEditor)
	{
		GEditor->GetEditorSubsystem<UUnrealEditorSubsystem>()->GetLevelViewportCameraInfo(CurrCameraPosition, CurrCameraRotator);
	}
	else
	{
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
		if (PlayerController)
		{
			CurrCameraPosition = PlayerController->PlayerCameraManager->GetCameraLocation();
		}
	}

	if (CameraPosition != CurrCameraPosition)
	{
		CameraPosition = CurrCameraPosition;
		QuadTreeComponent->UpdateQuadTree(CameraPosition, SubdivisionThreshold);
	}
}

bool AQuadTreeActor::ShouldTickIfViewportsOnly() const
{
	return true;
}
