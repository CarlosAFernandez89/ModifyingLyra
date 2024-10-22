// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraHealManaExecution.h"
#include "AbilitySystem/Attributes/LyraHealthSet.h"
#include "AbilitySystem/Attributes/LyraCombatSet.h"
#include "AbilitySystem/Attributes/LyraManaSet.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraHealManaExecution)


struct FHealManaStatics
{
	FGameplayEffectAttributeCaptureDefinition BaseHealManaDef;

	FHealManaStatics()
	{
		BaseHealManaDef = FGameplayEffectAttributeCaptureDefinition(ULyraCombatSet::GetBaseHealManaAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
	}
};

static FHealManaStatics& HealManaStatics()
{
	static FHealManaStatics Statics;
	return Statics;
}


ULyraHealManaExecution::ULyraHealManaExecution()
{
	RelevantAttributesToCapture.Add(FHealManaStatics().BaseHealManaDef);
}

void ULyraHealManaExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
#if WITH_SERVER_CODE
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = SourceTags;
	EvaluateParameters.TargetTags = TargetTags;

	float BaseHeal = 0.0f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(HealManaStatics().BaseHealManaDef, EvaluateParameters, BaseHeal);

	const float HealingDone = FMath::Max(0.0f, BaseHeal);

	if (HealingDone > 0.0f)
	{
		// Apply a healing modifier, this gets turned into + health on the target
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(ULyraManaSet::GetHealingAttribute(), EGameplayModOp::Additive, HealingDone));
	}
#endif // #if WITH_SERVER_CODE
}

