// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class ABoid;

/**
 * 
 */
class FLOCKING_API Gene
{
public:
	Gene();
	~Gene();

	// the weight of the boids, also the value needs to be evoluted 
	float avoid;
	float align;
	float trend;

	// the fitness of the solution
	float fitness;

	// four pre-defined genome type
	enum INIT_GENOME {
		Normal,
		AvoidFocus,
		AlignFocus,
		TrendFocus,
		COUNTER
	};

	// Crossover and Mutation in evolutionary algorithm
	Gene* Crossover(Gene* geneB);
	void Mutation();
	// calculate the fitness of the genome
	void CalculateFitness(ABoid* boid);
};