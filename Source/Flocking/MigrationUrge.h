// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MigrationUrge.generated.h"

class AEvolutionController;

UCLASS()
class FLOCKING_API AMigrationUrge : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMigrationUrge();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Set up evolution controller reference
	void SetEvoControllerRef();

	// evolution controller reference
	AEvolutionController* evoController;

	float timer;
	
	// visual representtions
	UStaticMeshComponent* Visual;
	UMaterialInstanceDynamic* MaterialInst;

};