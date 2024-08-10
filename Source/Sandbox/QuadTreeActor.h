#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "QuadTree.h"
#include "QuadTreeActor.generated.h"

UCLASS()
class SANDBOX_API AQuadTreeActor : public AActor
{
	GENERATED_BODY()

public:
	// Constructor
	AQuadTreeActor();

protected:
	// BeginPlay override
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
public:
	// Tick override
	virtual void Tick(float DeltaTime) override;
	void UpdateQuadTree();
	virtual bool ShouldTickIfViewportsOnly() const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "QuadTree")
	UQuadTreeComponent* QuadTreeComponent;
	
	float SubdivisionThreshold;
	FVector CameraPosition;
};