// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Predator.generated.h"

class AEvolutionController;
class ABoid;

UCLASS()
class FLOCKING_API APredator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APredator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// the speed of the predator
	float speed;
	// the current target of the predator
	ABoid* target;
	float timer;
	// flag determines if the predator is eating 
	bool eating;

	// visual presentations
	UStaticMeshComponent* Visual;
	UMaterialInstanceDynamic* MaterialInst;

	// evolution controller reference
	AEvolutionController* evoController;

	// set the target 
	void SetTarget();
	// set up evolution controller
	void SetEvoControllerRef();
};
