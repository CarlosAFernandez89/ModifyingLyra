// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LyraInventoryItemDefinition.h"

#include "InventoryFragment_ConsumeEffects.generated.h"


struct FGameplayTag;
class UGameplayEffect;


USTRUCT(BlueprintType)
struct FEffectWithMagnitude
{
	GENERATED_BODY()

	// The gameplay effect class
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> EffectClass;

	// The magnitude associated with the effect
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Magnitude = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag MagnitudeTag;
};

/**
 * 
 */
UCLASS()
class LYRAGAME_API UInventoryFragment_ConsumeEffects : public ULyraInventoryItemFragment
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FEffectWithMagnitude> OnUseEffectsWithMagnitude;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag GameplayCueTag;
};
