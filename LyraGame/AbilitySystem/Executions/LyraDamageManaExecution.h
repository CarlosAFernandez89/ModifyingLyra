// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayEffectExecutionCalculation.h"

#include "LyraDamageManaExecution.generated.h"

class UObject;


/**
 * ULyraManaExecution
 *
 *	Execution used by gameplay effects to apply damage to the health attributes.
 */
UCLASS()
class ULyraDamageManaExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:

	ULyraDamageManaExecution();

protected:

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
