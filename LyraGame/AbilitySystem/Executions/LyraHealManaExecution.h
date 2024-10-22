// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameplayEffectExecutionCalculation.h"

#include "LyraHealManaExecution.generated.h"

/**
 * ULyraHealDamageExecution
 *
 *	Execution used by gameplay effects to apply healing to the health attributes.
 */
UCLASS()
class LYRAGAME_API ULyraHealManaExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
public:

	ULyraHealManaExecution();

protected:

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
