// Fill out your copyright notice in the Description page of Project Settings.


#include "Boid.h"
#include "ConstructorHelpers.h"
#include "Asteroid.h"
#include "Predator.h"
#include "Gene.h"
#include "MigrationUrge.h"
#include "EngineUtils.h"
#include "Kismet/KismetMathLibrary.h"
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include <EngineGlobals.h>

// Sets default values
ABoid::ABoid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetActorEnableCollision(true);

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root Component"));
	VisualComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual Representation"));
	VisualComponent->SetupAttachment(RootComponent);

	//Get quadPyramid Mesh
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereVisualAsset(TEXT("/Game/StarterContent/Shapes/Shape_QuadPyramid.Shape_QuadPyramid"));
	if (SphereVisualAsset.Succeeded())
	{
		VisualComponent->SetStaticMesh(SphereVisualAsset.Object);
	}

	//Get Material
	static ConstructorHelpers::FObjectFinder<UMaterial> FoundMaterial(TEXT("/Game/StarterContent/Materials/M_Red.M_Red"));
	if (FoundMaterial.Succeeded())
	{
		DynamicMaterialInst = UMaterialInstanceDynamic::Create((UMaterial*)FoundMaterial.Object, VisualComponent);
		VisualComponent->SetMaterial(0, DynamicMaterialInst);
	}

	//Set Personal Space
	personalSpaceRadius = 500.f;

	personalSpace = CreateDefaultSubobject<USphereComponent>(TEXT("Personal Space Sphere"));
	personalSpace->InitSphereRadius(personalSpaceRadius);
	personalSpace->SetupAttachment(RootComponent);
	personalSpace->SetRelativeLocation(FVector(0, 0, 50));
	personalSpace->OnComponentBeginOverlap.AddDynamic(this, &ABoid::OnOverlapBegin);
	personalSpace->OnComponentEndOverlap.AddDynamic(this, &ABoid::OnOverlapEnd);
	personalSpace->SetGenerateOverlapEvents(true);

	//Set Visible Space
	visibleSpaceRadius = 1000.f;

	visibleSpace = CreateDefaultSubobject<USphereComponent>(TEXT("Visible Space Sphere"));
	visibleSpace->InitSphereRadius(visibleSpaceRadius);
	visibleSpace->SetupAttachment(RootComponent);
	visibleSpace->SetRelativeLocation(FVector(0, 0, 50));
	visibleSpace->OnComponentBeginOverlap.AddDynamic(this, &ABoid::OnOverlapBegin);
	visibleSpace->OnComponentEndOverlap.AddDynamic(this, &ABoid::OnOverlapEnd);
	visibleSpace->SetGenerateOverlapEvents(true);
	
	//Set Death Space
	deathSpaceRadius = 150.f;

	deathSpace = CreateDefaultSubobject<USphereComponent>(TEXT("Death Space Sphere"));
	deathSpace->InitSphereRadius(deathSpaceRadius);
	deathSpace->SetupAttachment(RootComponent);
	deathSpace->SetRelativeLocation(FVector(0, 0, 50));
	deathSpace->OnComponentBeginOverlap.AddDynamic(this, &ABoid::OnOverlapBegin);
	deathSpace->OnComponentEndOverlap.AddDynamic(this, &ABoid::OnOverlapEnd);
	deathSpace->SetGenerateOverlapEvents(true);


	// Initialize some values
	speed = 200.f;
	maxSpeed = 250.f;
	minSpeed = 150.f;

	gene = new Gene();
	GenotypeUpdate();

	death = DEATH_METHOD::COUNTER;

	SetRandomTarget();
	SetActorScale3D(FVector(0.5, 0.5, 1.0));
}

// Called when the game starts or when spawned
void ABoid::BeginPlay()
{
	Super::BeginPlay();
}

void ABoid::CollisionAvoidance(float DeltaTime)
{
	// initialize a vector
	FVector sumAvoid = FVector::ZeroVector;
	// loop through each actors that needs to avoid
	// substrate the current location with the actors location
	// and add the results up
	for (int i = 0; i < avoidActors.Num(); i++) {
		if (ABoid* boid = Cast<ABoid>(avoidActors[i])) {
			if (!(boid->isDied))
				sumAvoid += GetActorLocation() - avoidActors[i]->GetActorLocation();
		}
		else {
			sumAvoid += GetActorLocation() - avoidActors[i]->GetActorLocation();
		}
	}

	// the avoidVector in the current Tick will be the sum result multiply by deltaTime
	avoidVector = sumAvoid * DeltaTime;
}

void ABoid::VelocityMatching(float DeltaTime)
{
	// initializee a vector
	FVector sumVelocity = FVector::ZeroVector;
	// loop through the visible boids
	// add the velocity vector up to sumVelocity
	for (int i = 0; i < visibleBoids.Num(); i++) {
		if (!visibleBoids[i]->isDied)
			sumVelocity += visibleBoids[i]->velocityVector;
	}

	// loop through the visible asteroids
	// add the velocity vector up to sumVelocity
	for (int i = 0; i < visibleAsteroids.Num(); i++) {
		FVector disToTarget = visibleAsteroids[i]->targetLocation - visibleAsteroids[i]->GetActorLocation();
		disToTarget.Normalize();
		sumVelocity += disToTarget * visibleAsteroids[i]->speed * DeltaTime;
	}

	// if there is some one that this boid can see
	// the align vector will be the sum velocity divided by the total number of the visible actors
	// otherwise the align vector will be zero vector
	if (visibleAsteroids.Num() != 0 || visibleBoids.Num() != 0)
		alignVector = sumVelocity / (visibleBoids.Num() + visibleAsteroids.Num());
	else
		alignVector = sumVelocity;
}


void ABoid::FlockCenter(float DeltaTime)
{
	// start with a zero vector
	FVector sum = FVector::ZeroVector;

	// add up all boids' location which are in the visible range
	for (int i = 0; i < visibleBoids.Num(); i++) {
		if (!visibleBoids[i]->isDied)
			sum += visibleBoids[i]->GetActorLocation();
	}

	// if the boid can see other boids
	// the tendencyVector in the current Tick will be the sum vector divided by the number of the visible boids
	// minus the current location
	// multiply by the delta time
	// otherwise the tendency vector is zero vector
	if (visibleBoids.Num() != 0)
		tendencyVector = (sum / visibleBoids.Num() - GetActorLocation()) * DeltaTime;
	else
		tendencyVector = sum;
}

// Called every frame
void ABoid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IfOutOfBorder())
		Die();
	
	if (!isDied)
		timer += DeltaTime;
	else
		return;

	// if the boid has reached the target or it has been out of the border
	// reset the target location
	if (FVector::Dist(targetLocation, GetActorLocation()) < 100.f) {
		SetRandomTarget();
		
	}
	
	// calculate the velocity vector
	FVector disToTarget = targetLocation - GetActorLocation();
	disToTarget.Normalize();
	velocityVector = disToTarget * DeltaTime * speed;

	// set up three flock behaviors 
	CollisionAvoidance(DeltaTime);
	VelocityMatching(DeltaTime);
	FlockCenter(DeltaTime);

	// add them up to the current velocity with corresponding weight
	velocityVector += avoidVector * avoidWeight + alignVector * alignWeight + tendencyVector * trendWeight;
	
	// if the velocity is too slow, then it should apply the minimum speed in the current direction
	// or if the velocity is too fast, then it should appy the max speed in the current direction
	if (velocityVector.Size() < minSpeed * DeltaTime) {
		velocityVector.Normalize();
		velocityVector *= minSpeed * DeltaTime;
	} 
	else if (velocityVector.Size() > maxSpeed * DeltaTime) {
		velocityVector.Normalize();
		velocityVector *= maxSpeed * DeltaTime;
	}

	// some debug logs
	//FString name = GetName();
	//UE_LOG(LogTemp, Warning, TEXT("%s avoidVector: X: %f, Y: %f, Z: %f"), *name, avoidVector.X, avoidVector.Y, avoidVector.Z);
	//UE_LOG(LogTemp, Warning, TEXT("%s velocityVector: X: %f, Y: %f, Z: %f"), *name, velocityVector.X, velocityVector.Y, velocityVector.Z);
	//UE_LOG(LogTemp, Warning, TEXT("%s tendencyVector: X: %f, Y: %f, Z: %f"), *name, tendencyVector.X, tendencyVector.Y, tendencyVector.Z);
	//UE_LOG(LogTemp, Warning, TEXT("%s CurrentLoc: X: %f, Y: %f, Z: %f"), *name, GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z);
	//UE_LOG(LogTemp, Warning, TEXT("%s CurrentTarget: X: %f, Y: %f, Z: %f"), *name, targetLocation.X, targetLocation.Y, targetLocation.Z);
	
	// move the boid
	SetActorLocation(GetActorLocation() + velocityVector);
	
	
}

// Overlap events to handle the actors inside the spaces
void ABoid::OnOverlapBegin(UPrimitiveComponent * OverlappedComp, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{		
	// if an actor is in the death space and the current boid is not died 
	// and the other overlapped component is a static mesh rather than other boids visible space, etc.
	if (OverlappedComp == deathSpace && !isDied && Cast<UStaticMeshComponent>(OtherComp)) {
		// if the collide actor is a boid
		if (ABoid* collideBoid = Cast<ABoid>(OtherActor)) {
			// if the boid is not died
			// both of them will die
			if (!(collideBoid->isDied)) {
				Die(OtherActor);
				collideBoid->Die(this);
			}
		}
		// if the colide actor is migration urge, simply ignore it
		else if (Cast<AMigrationUrge>(OtherActor)) {

		}
		// otherwise the current boid dies due to collide with other actor
		else {
			Die(OtherActor);
		}
	}
	
	// if an actor (should be a static mesh rather than the sphere collider) goes into the personal space
	if (OverlappedComp == personalSpace && Cast<UStaticMeshComponent>(OtherComp)) {
		// if the actor is a boid or asteroid or predator, avoid it
		if (Cast<ABoid>(OtherActor) || Cast<AAsteroid>(OtherActor) || Cast<APredator>(OtherActor)) {
			avoidActors.Add(OtherActor);
		}
	}

	// if an actor goes into the visible space
	if (OverlappedComp == visibleSpace && Cast<UStaticMeshComponent>(OtherComp)) {
		// add it to the visible actor list
		if (ABoid* boid = Cast<ABoid>(OtherActor)) {
			visibleBoids.Add(boid);
		}

		if (AAsteroid* asteroid = Cast<AAsteroid>(OtherActor)) {
			visibleAsteroids.Add(asteroid);
		}
	}
}

void ABoid::OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) 
{
	// If an actor gets out of the personal space
	// remove it from the avoid actor list
	if (OverlappedComp == personalSpace && Cast<UStaticMeshComponent>(OtherComp)) {
		if (Cast<ABoid>(OtherActor) || Cast<AAsteroid>(OtherActor) || Cast<APredator>(OtherActor)) {
			avoidActors.Remove(OtherActor);
		}
	}
	
	// If an actor gets out of the visible space
	// remove it from the visible actor list
	if (OverlappedComp == visibleSpace && Cast<UStaticMeshComponent>(OtherComp)) {
		if (ABoid* boid = Cast<ABoid>(OtherActor)) {
			visibleBoids.Remove(boid);
		}

		if (AAsteroid* asteroid = Cast<AAsteroid>(OtherActor)) {
			visibleAsteroids.Remove(asteroid);
		}
	}
}

void ABoid::SetRandomTarget()
{
	// set a random target
	targetLocation.X = FMath::FRandRange(-3000, 3000);
	targetLocation.Y = FMath::FRandRange(-3000, 3000);
	targetLocation.Z = FMath::FRandRange(-3000, 3000);
}

void ABoid::SetRandomLocation()
{
	// iterate through all the actors and add all boids into an array
	TArray<ABoid*> boids;
	for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		// if a actor can successfully cast to level generator
		if (ABoid* temp = Cast<ABoid>(*ActorItr)) {
			// set up the reference
			boids.Add(temp);
		}
	}

	bool isFree = false;
	while (!isFree) {
		// create a random location
		float randX = FMath::FRandRange(-3000, 3000);
		float randY = FMath::FRandRange(-3000, 3000);
		float randZ = FMath::FRandRange(-3000, 3000);
		FVector location(randX, randY, randZ);
		bool positionOK = true;

		// loop through the boids to ensure the new location will not collide with other boids
		for (int i = 0; i < boids.Num(); i++) {
			if (FVector::Dist(location, boids[i]->GetActorLocation()) < 150.f) {
				positionOK = false;
			}
		}

		// if the position is ok, move the actor to that location
		if (positionOK) {
			SetActorLocation(location);
			isFree = true;
		}
	}
}

bool ABoid::IfOutOfBorder() {
	// if the actor is out of the border, return true, otherwise false
	FVector currentPosition = GetActorLocation();
	if (currentPosition.X > -3000.f && currentPosition.X < 3000.f && currentPosition.Y > -3000.f && currentPosition.Y < 3000.f && currentPosition.Z > -3000.f && currentPosition.Z < 3000.f) {
		return false;
	}
	return true;
}

void ABoid::GenotypeUpdate()
{
	// update the weight based on the genome
	avoidWeight = gene->avoid;
	alignWeight = gene->align;
	trendWeight = gene->trend;
}

void ABoid::Die(AActor* collideActor) {
	//set the death flag as true
	isDied = true;
	// if the boid died due to collide with asterorid or other boid
	// they are died due to hitting wall
	if (Cast<AAsteroid>(collideActor) || Cast<ABoid>(collideActor)) {
		death = DEATH_METHOD::HitWall;
	}
	// if the boid die due to collide with predator
	// they are died due to being eaten
	else if (APredator* predator = Cast<APredator>(collideActor)){
		death = DEATH_METHOD::BeEaten;
		predator->eating = true;
	} 
	// default death reason is hitting wall
	else {
		death = DEATH_METHOD::BeEaten;
	}
	// calculate the fitness
	gene->CalculateFitness(this);
	// temply hide the actor
	SetActorScale3D(FVector(0, 0, 0));
}

void ABoid::Breed(Gene* genotype) {
	// 'breed' the child by reset some values
	isDied = false;
	timer = 0.f;
	death = DEATH_METHOD::COUNTER;
	gene = genotype;
	GenotypeUpdate();
	SetActorScale3D(FVector(0.5, 0.5, 1.0));
	SetRandomLocation();
}
