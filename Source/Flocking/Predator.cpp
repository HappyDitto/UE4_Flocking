// Fill out your copyright notice in the Description page of Project Settings.


#include "Predator.h"
#include "ConstructorHelpers.h"
#include "EvolutionController.h"
#include "Boid.h"
#include "EngineUtils.h"
#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>

// Sets default values
APredator::APredator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set up the visual representations
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));
	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual Representation"));
	Visual->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereVisualAsset(TEXT("/Game/StarterContent/Shapes/Shape_QuadPyramid.Shape_QuadPyramid"));
	if (SphereVisualAsset.Succeeded())
	{
		Visual->SetStaticMesh(SphereVisualAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> FoundMaterial(TEXT("/Game/Assignment2/M_Predator.M_Predator"));
	if (FoundMaterial.Succeeded())
	{
		MaterialInst = UMaterialInstanceDynamic::Create((UMaterial*)FoundMaterial.Object, Visual);
		Visual->SetMaterial(0, MaterialInst);
	}

	// initialize some values
	speed = 300.0f;
	timer = 0.f;
	SetActorScale3D(FVector(0.5, 0.5, 1.0));
}

// Called when the game starts or when spawned
void APredator::BeginPlay()
{
	Super::BeginPlay();

	// set up the evolution controller reference
	SetEvoControllerRef();
}

// Called every frame
void APredator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// if the predator is eating and the timer is less than 5
	if (eating && timer < 5)
	{
		// do nothing but just increase the timer
		timer += DeltaTime;
		return;
	}
	
	// if the target is nullptr or timer > 5 or target is died but the predator is not eating
	if (!target || timer >= 5 || (target->isDied && !eating)) {
		// clear the target pointer and the timer, reset the eating flag as false
		SetTarget();
		timer = 0;
		eating = false;
	}

	// move the actor
	SetActorLocation(FMath::VInterpConstantTo(GetActorLocation(), target->GetActorLocation(), DeltaTime, speed));
}

void APredator::SetTarget()
{
	target = nullptr;
	
	// loop through the populations and randomly choose an alive boid as the target
	while (!target) {
		int targetIndex = FMath::RandRange(0, evoController->population.Num() - 1);
		if (!(evoController->population[targetIndex]->isDied)) {
			target = evoController->population[targetIndex];
		}
	}
}

void APredator::SetEvoControllerRef()
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

