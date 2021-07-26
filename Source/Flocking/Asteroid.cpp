// Fill out your copyright notice in the Description page of Project Settings.

#include "Asteroid.h"
#include "ConstructorHelpers.h"
#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>

// Sets default values
AAsteroid::AAsteroid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	//RootComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootComponent"));
	AsteroidVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual Representation"));
	AsteroidVisual->SetupAttachment(RootComponent);

	//Get Sphere Mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereVisualAsset(TEXT("/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere"));
	if (SphereVisualAsset.Succeeded())
	{
		AsteroidVisual->SetStaticMesh(SphereVisualAsset.Object);
	}

	//Get Material
	static ConstructorHelpers::FObjectFinder<UMaterial> FoundMaterial(TEXT("/Game/StarterContent/Materials/M_Concrete_Grime.M_Concrete_Grime"));
	if (FoundMaterial.Succeeded())
	{
		DynamicMaterialInst = UMaterialInstanceDynamic::Create((UMaterial*)FoundMaterial.Object, AsteroidVisual);
		AsteroidVisual->SetMaterial(0, DynamicMaterialInst);
	}

	//Set Speed, Position and Target
	speed = 400.0f;
	position = this->GetActorLocation();
	SetRandomTarget();
}

// Called when the game starts or when spawned
void AAsteroid::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AAsteroid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	position = this->GetActorLocation();
	SetActorLocation(FMath::VInterpConstantTo(position, targetLocation, DeltaTime, speed));
	if (FVector::Dist(position,targetLocation) < 100.0f)
	{
		SetRandomTarget();
	}
}

void AAsteroid::SetRandomTarget()
{
	targetLocation.X = FMath::FRandRange(-3000, 3000);
	targetLocation.Y = FMath::FRandRange(-3000, 3000);
	targetLocation.Z = FMath::FRandRange(-3000, 3000);
}