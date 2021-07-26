// Fill out your copyright notice in the Description page of Project Settings.


#include "EvolutionController.h"
#include "Engine/Engine.h"
#include "Boid.h"
#include "Gene.h"

// Sets default values
AEvolutionController::AEvolutionController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// the population number
	popNum = 100;
	// counter for counting the generation
	generation = 0;
}

// Called when the game starts or when spawned
void AEvolutionController::BeginPlay()
{
	Super::BeginPlay();

	// initialize the population
	InitPopulation();
	
}

// Called every frame
void AEvolutionController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// iterate through the population to monitor the death number
	int deathCounter = 0;
	for (int i = 0; i < population.Num(); i++) {
		if (population[i]->isDied)
			deathCounter += 1;
	}
	GEngine->AddOnScreenDebugMessage(-2, 5.f, FColor::Yellow, FString::Printf(TEXT("Death: %d"), deathCounter));

	// if 80% population has died, start breeding
	if (deathCounter >= 80) {
		// analysis the current solution's fitness
		FitnessAnalysis();
	
		// clean children array
		children.Empty();
		// select parents, ensure parentA and B will not be the same 
		for (int i = 0; i < FMath::CeilToInt(deathCounter/2); i++) {
			parentA = ParentalSelection();
			parentB = ParentalSelection();
			while (parentB == parentA) {
				parentB = ParentalSelection();
			}

			// breed the next generation's offspring
			Breed();
		}

		// survivor selection occurs
		SurvivorSelection();

		// now we are in the next generation
		generation += 1;
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("Generation %d"), generation));
	}

	// if any of the population fits the condition, terminate the algorithm
	Termination();
}

void AEvolutionController::InitPopulation()
{
	// generate some boids and ensure they will not collide with each other
	while (population.Num() < popNum) {
		float randX = FMath::FRandRange(-3000, 3000);
		float randY = FMath::FRandRange(-3000, 3000);
		float randZ = FMath::FRandRange(-3000, 3000);
		FVector location(randX, randY, randZ);
		bool isFree = true;

		for (int i = 0; i < population.Num(); i++) {
			if (FVector::Dist(location, population[i]->GetActorLocation()) < 150.f) {
				isFree = false;
			}
		}

		if (isFree) {
			ABoid* NewBoid = GetWorld()->SpawnActor<ABoid>(BoidBlueprint, location, FRotator::ZeroRotator);
			population.Add(NewBoid);
		}
	}
}

void AEvolutionController::FitnessAnalysis() {
	// convert the fitness to possibility and store the result into the rouletee wheel
	float total = 0.f;
	for (int i = 0; i < population.Num(); i++) {
		population[i]->gene->CalculateFitness(population[i]);
		total += population[i]->gene->fitness;
	}

	for (int i = 0; i < population.Num(); i++) {
		population[i]->gene->CalculateFitness(population[i]);
		float possibility = population[i]->gene->fitness / total * 100;
		rouletteWheel.Add(population[i], possibility);
	}
}

ABoid* AEvolutionController::ParentalSelection()
{	
	// select the parent by roulette wheel selection
	ABoid* parent = nullptr;
	
	while (!parent) {
		float selectPointer = FMath::FRandRange(0.f, 100.f);

		for (TMap<ABoid*, float>::TIterator it = rouletteWheel.CreateIterator(); it; ++it) {
			selectPointer -= it->Value;
			if (selectPointer < 0) {
				parent = it->Key;
				break;
			}
		}
	}
	
	return parent;
}

void AEvolutionController::Breed()
{
	//perform crossover and mutation
	Gene* newGenotype1 = parentA->gene->Crossover(parentB->gene);
	newGenotype1->Mutation();

	Gene* newGenotype2 = parentB->gene->Crossover(parentA->gene);
	newGenotype1->Mutation();

	// add new genotypes into the children list
	children.Add(newGenotype1);
	children.Add(newGenotype2);
}

void AEvolutionController::SurvivorSelection()
{
	// loop through all died solution and reset them with the new genome
	// the alive boids continue their work
	int childrenIndex = 0;
	for (int i = 0; i < population.Num(); i++) {
		if (population[i]->isDied) {
			if (childrenIndex < children.Num() - 1) {
				population[i]->Breed(children[childrenIndex]);
				childrenIndex++;
			}
			else {
				population[i]->Breed(children[childrenIndex]);
			}
		}
	}
}

void AEvolutionController::Termination()
{
	// if any boid lives for 300 seconds (5 minutes)
	// print the weight information on the screent
	for (int i = 0; i < population.Num(); i++) {
		population[i]->gene->CalculateFitness(population[i]);
		if (population[i]->gene->fitness > 300.f) {
			GEngine->AddOnScreenDebugMessage(-3, 5.f, FColor::Yellow, 
				FString::Printf(TEXT("Optimized solution: Avoid: %f, Align: %f, Trend: %f"), 
				population[i]->gene->avoid, 
				population[i]->gene->align,
				population[i]->gene->trend));
		}
	}
}

