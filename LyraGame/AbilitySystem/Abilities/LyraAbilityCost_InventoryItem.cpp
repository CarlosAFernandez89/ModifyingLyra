// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraAbilityCost_InventoryItem.h"
#include "GameplayAbilitySpec.h"
#include "GameplayAbilitySpecHandle.h"
#include "LyraGameplayAbility.h"
#include "NativeGameplayTags.h"
#include "Inventory/LyraInventoryManagerComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraAbilityCost_InventoryItem)

UE_DEFINE_GAMEPLAY_TAG(TAG_ABILITY_FAIL_ITEMCOST, "Ability.ActivateFail.Cost");

ULyraAbilityCost_InventoryItem::ULyraAbilityCost_InventoryItem()
{
	Quantity = 1;
	FailureTag = TAG_ABILITY_FAIL_ITEMCOST;
}

bool ULyraAbilityCost_InventoryItem::CheckCost(const ULyraGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (AController* PC = Ability->GetControllerFromActorInfo())
	{
		if (ULyraInventoryManagerComponent* InventoryComponent = PC->GetComponentByClass<ULyraInventoryManagerComponent>())
		{
			const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

			const float NumItemsToConsumeReal = GetQuantityAtLevel(AbilityLevel);
			const int32 NumItemsToConsume = FMath::TruncToInt(NumItemsToConsumeReal);
			const bool bCanApplyCost = InventoryComponent->GetTotalItemCountByDefinition(ItemDefinition) >= Quantity;

			// Inform other abilities why this cost cannot be applied
			if (!bCanApplyCost && OptionalRelevantTags && FailureTag.IsValid())
			{
				OptionalRelevantTags->AddTag(FailureTag);				
			}

			return InventoryComponent->GetTotalItemCountByDefinition(ItemDefinition) >= NumItemsToConsume;
		}
	}
	return false;
}

void ULyraAbilityCost_InventoryItem::ApplyCost(const ULyraGameplayAbility* Ability, const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	if (ActorInfo->IsNetAuthority())
	{
		if (AController* PC = Ability->GetControllerFromActorInfo())
		{
			if (ULyraInventoryManagerComponent* InventoryComponent = PC->GetComponentByClass<ULyraInventoryManagerComponent>())
			{
				const int32 AbilityLevel = Ability->GetAbilityLevel(Handle, ActorInfo);

				const float NumItemsToConsumeReal = GetQuantityAtLevel(AbilityLevel);
				const int32 NumItemsToConsume = FMath::TruncToInt(NumItemsToConsumeReal);

				InventoryComponent->RemoveItemDefinition(ItemDefinition, NumItemsToConsume);
			}
		}
	}
}

int32 ULyraAbilityCost_InventoryItem::GetQuantityAtLevel(int32 AbilityLevel) const
{
	return Quantity * AbilityLevel;
}

