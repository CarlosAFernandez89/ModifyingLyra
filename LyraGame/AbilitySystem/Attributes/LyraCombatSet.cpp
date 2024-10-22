// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraCombatSet.h"

#include "AbilitySystem/Attributes/LyraAttributeSet.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraCombatSet)

class FLifetimeProperty;


ULyraCombatSet::ULyraCombatSet()
	: BaseDamage(0.0f)
	, BaseDamageMana(0.f)
	, BaseHeal(0.0f)
	, BaseHealMana(0.f)
{
}

void ULyraCombatSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(ULyraCombatSet, BaseDamage, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ULyraCombatSet, BaseDamageMana, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ULyraCombatSet, BaseHeal, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ULyraCombatSet, BaseHealMana, COND_OwnerOnly, REPNOTIFY_Always);

}

void ULyraCombatSet::OnRep_BaseDamage(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULyraCombatSet, BaseDamage, OldValue);
}

void ULyraCombatSet::OnRep_BaseDamageMana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULyraCombatSet, BaseDamageMana, OldValue);
}

void ULyraCombatSet::OnRep_BaseHeal(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULyraCombatSet, BaseHeal, OldValue);
}

void ULyraCombatSet::OnRep_BaseHealMana(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULyraCombatSet, BaseHealMana, OldValue);
}

