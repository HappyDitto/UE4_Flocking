// Fill out your copyright notice in the Description page of Project Settings.


#include "MigrationUrge.h"
#include "EngineUtils.h"
#include "Boid.h"
#include "EvolutionController.h"

// Sets default values
AMigrationUrge::AMigrationUrge()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// visual presentations
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));
	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual Representation"));
	Visual->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereVisualAsset(TEXT("/Game/StarterContent/Shapes/Shape_NarrowCapsule.Shape_NarrowCapsule"));
	if (SphereVisualAsset.Succeeded())
	{
		Visual->SetStaticMesh(SphereVisualAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> FoundMaterial(TEXT("/Game/Assignment2/M_MigrationUrge.M_MigrationUrge"));
	if (FoundMaterial.Succeeded())
	{
		MaterialInst = UMaterialInstanceDynamic::Create((UMaterial*)FoundMaterial.Object, Visual);
		Visual->SetMaterial(0, MaterialInst);
	}

}

// Called when the game starts or when spawned
void AMigrationUrge::BeginPlay()
{
	Super::BeginPlay();
	
	// set up evolution controller reference
	SetEvoControllerRef();
}

// Called every frame
void AMigrationUrge::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// timer increases
	timer += DeltaTime;

	// for every 30 seconds passed
	if (timer >= 30.f) {
		// teleport this actor to a random position
		timer = 0;
		float randX = FMath::FRandRange(-2500, 2500);
		float randY = FMath::FRandRange(-2500, 2500);
		float randZ = FMath::FRandRange(-2500, 2500);
		FVector location(randX, randY, randZ);
		SetActorLocation(location);
		// let all boids move towards the new position
		for (int i = 0; i < evoController->population.Num(); i++) {
			evoController->population[i]->targetLocation = this->GetActorLocation();
		}
	}
}

void AMigrationUrge::SetEvoControllerRef()
{
	evoController = nullptr;
	// loop through all the actors in the world
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		// if a actor can successfully cast to evolution controller
		if (AEvolutionController* temp = Cast<AEvolutionController>(*ActorItr)) {
			// set up the reference
			evoController = temp;
		}
	}
}

