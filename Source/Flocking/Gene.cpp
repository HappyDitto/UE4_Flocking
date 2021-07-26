// Fill out your copyright notice in the Description page of Project Settings.


#include "Gene.h"
#include "Boid.h"

Gene::Gene()
{
	// randomly choose a genome type when constructs 
	int genome = FMath::RandRange(0, INIT_GENOME::COUNTER - 1);

	switch (genome) {
	case INIT_GENOME::Normal:
		avoid = 1;
		align = 1;
		trend = 1;
		break;
	case INIT_GENOME::AvoidFocus:
		avoid = 2;
		align = 0.5;
		trend = 0.5;
		break;
	case INIT_GENOME::AlignFocus:
		avoid = 0.5;
		align = 2;
		trend = 0.5;
		break;
	case INIT_GENOME::TrendFocus:
		avoid = 0.5;
		align = 0.5;
		trend = 2;
		break;
	}
}

void Gene::CalculateFitness(ABoid* boid)
{
	// based on the death method, calculate the fitness
	switch (boid->death) {
	case ABoid::DEATH_METHOD::HitWall:
		fitness = boid->timer * (1 - 0.25);
		break;
	case ABoid::DEATH_METHOD::BeEaten:
		fitness = boid->timer * (1 - 0.5);
		break;
	default:
		fitness = boid->timer;
	}
}

Gene* Gene::Crossover(Gene* geneB)
{
	// crossover the portions of the gene
	Gene* newGene = new Gene();
	newGene->avoid = geneB->avoid;
	newGene->align = this->align;
	newGene->trend = geneB->trend;

	return newGene;
}

void Gene::Mutation()
{
	// as mutation randomly happened, determine it by randbool
	if (FMath::RandBool()) {
		// randomly choose a value to mutate
		int selector = FMath::RandRange(0, 2);
		switch (selector) {
		case 0:
			this->avoid = FMath::FRandRange(0.5, 2);
			break;
		case 1:
			this->align = FMath::FRandRange(0.5, 2);
			break;
		case 2:
			this->trend = FMath::FRandRange(0.5, 2);
			break;
		}
	}
}


Gene::~Gene()
{
}
