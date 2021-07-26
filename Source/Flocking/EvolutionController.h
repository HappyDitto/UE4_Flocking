// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EvolutionController.generated.h"

class ABoid;
class Gene;

UCLASS()
class FLOCKING_API AEvolutionController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEvolutionController();

	UPROPERTY(EditAnywhere, Category = "Boid")
		TSubclassOf<AActor> BoidBlueprint;

	// population list
	TArray<ABoid*> population;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// counter for counting the generations
	int generation;
	// the population number 
	int popNum;

	// the roulette wheel for choosing parents
	TMap<ABoid*, float> rouletteWheel;
	// parents reference
	ABoid* parentA;
	ABoid* parentB;

	// offspring list
	TArray<Gene*> children;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	// Evolution Algorithms
	void InitPopulation();
	void FitnessAnalysis();
	ABoid* ParentalSelection();
	void Breed();
	void SurvivorSelection();
	void Termination();
};
