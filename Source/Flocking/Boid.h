// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Boid.generated.h"

class AAsteroid;
class Gene;

UCLASS()
class FLOCKING_API ABoid : public AActor
{
	GENERATED_BODY()

	UFUNCTION()
		void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
		void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
public:	
	// Sets default values for this actor's properties
	ABoid();

	// The method of death
	enum DEATH_METHOD 
	{
		HitWall,
		BeEaten,
		COUNTER
	};

	// update the weight based on the genome
	void GenotypeUpdate();
	// breed child
	void Breed(Gene* genotype);
	// kill the current boid
	void Die(AActor* collideActor = nullptr);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Flock rules
	void FlockCenter(float DeltaTime);
	void CollisionAvoidance(float DeltaTime);
	void VelocityMatching(float DeltaTime);
	// Set a random target
	void SetRandomTarget();
	// Check if the boid is out of the borders
	bool IfOutOfBorder();
	// set a random position for the boid
	void SetRandomLocation();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// the timer to count the survive time
	float timer;
	// flag indicating if the boid is death
	bool isDied;
	// the reason of the boid's death
	DEATH_METHOD death;

	// the speed of the boid
	float speed;
	// the maximum speed of the boid
	float maxSpeed;
	// the minimum speed of the boid
	float minSpeed;
	
	// the radius of the personal space
	float personalSpaceRadius;
	// the radius of the visible space
	float visibleSpaceRadius;
	// the radius of the visible space
	float deathSpaceRadius;
	
	// current target location
	FVector targetLocation;
	// current velocity
	FVector velocityVector;
	
	// flock vectors
	FVector alignVector;
	FVector avoidVector;
	FVector tendencyVector;

	// flock weights
	float alignWeight;
	float avoidWeight;
	float trendWeight;

	// genome
	Gene* gene;

	// arrays to store the actors in the personal space/visible space
	TArray<AActor*> avoidActors;
	TArray<ABoid*> visibleBoids;
	TArray<AAsteroid*> visibleAsteroids;
	
	// visual components;
	UStaticMeshComponent* VisualComponent;
	USphereComponent* personalSpace;
	USphereComponent* visibleSpace;
	USphereComponent* deathSpace;
	UMaterialInstanceDynamic* DynamicMaterialInst;
};