// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraEnemyCharacterBase.h"

#include "AIController.h"
#include "AbilitySystem/LyraAbilitySet.h" 
#include "Character/LyraPawnData.h"
#include "Character/LyraPawnExtensionComponent.h"
#include "Components/GameFrameworkComponentManager.h"
#include "GameFramework/PlayerState.h"


const FName ALyraEnemyCharacterBase::NAME_LyraAbilityReady("LyraAbilitiesReady");

void ALyraEnemyCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void ALyraEnemyCharacterBase::SetPawnData(const ULyraPawnData* InPawnData)
{
	check(InPawnData);

	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	PawnData = InPawnData;

	for (const ULyraAbilitySet* AbilitySet : InPawnData->AbilitySets)
	{
		if (AbilitySet)
		{
			AbilitySet->GiveToAbilitySystem(AbilitySystemComponent, nullptr);
		}
	}

	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, NAME_LyraAbilityReady);
}

FName ALyraEnemyCharacterBase::GetEnemyPawnName() const
{
	if(PawnData)
	{
		return PawnData->PawnDisplayName;
	}

	return FName();
}

void ALyraEnemyCharacterBase::OnAbilitySystemInitialized()
{
	Super::OnAbilitySystemInitialized();

	if (const ULyraPawnExtensionComponent* LyraPawnExtensionComponent = ULyraPawnExtensionComponent::FindPawnExtensionComponent(this))
	{
		SetPawnData(LyraPawnExtensionComponent->GetPawnData<ULyraPawnData>());
	}
}
