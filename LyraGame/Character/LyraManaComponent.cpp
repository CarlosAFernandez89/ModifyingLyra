// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/LyraManaComponent.h"

#include "AbilitySystem/Attributes/LyraAttributeSet.h"
#include "LyraLogChannels.h"
#include "System/LyraAssetManager.h"
#include "System/LyraGameData.h"
#include "LyraGameplayTags.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/LyraHealthSet.h"
#include "AbilitySystem/Attributes/LyraManaSet.h"
#include "Messages/LyraVerbMessage.h"
#include "Messages/LyraVerbMessageHelpers.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraManaComponent)

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lyra_OutOfMana_Message, "Lyra.OutOfMana.Message");


ULyraManaComponent::ULyraManaComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	AbilitySystemComponent = nullptr;
	ManaSet = nullptr;
	DeathState = ELyraDeathState::NotDead;
}

void ULyraManaComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULyraManaComponent, DeathState);
}

void ULyraManaComponent::OnUnregister()
{
	UninitializeFromAbilitySystem();

	Super::OnUnregister();
}

void ULyraManaComponent::InitializeWithAbilitySystem(ULyraAbilitySystemComponent* InASC)
{
	AActor* Owner = GetOwner();
	check(Owner);

	if (AbilitySystemComponent)
	{
		UE_LOG(LogLyra, Error, TEXT("LyraManaComponent: Mana component for owner [%s] has already been initialized with an ability system."), *GetNameSafe(Owner));
		return;
	}

	AbilitySystemComponent = InASC;
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogLyra, Error, TEXT("LyraManaComponent: Cannot initialize health component for owner [%s] with NULL ability system."), *GetNameSafe(Owner));
		return;
	}

	ManaSet = AbilitySystemComponent->GetSet<ULyraManaSet>();
	if (!ManaSet)
	{
		UE_LOG(LogLyra, Error, TEXT("LyraManaComponent: Cannot initialize health component for owner [%s] with NULL health set on the ability system."), *GetNameSafe(Owner));
		return;
	}

	// Register to listen for attribute changes.
	ManaSet->OnManaChanged.AddUObject(this, &ThisClass::HandleManaChanged);
	ManaSet->OnMaxManaChanged.AddUObject(this, &ThisClass::HandleMaxManaChanged);
	ManaSet->OnOutOfMana.AddUObject(this, &ThisClass::HandleOutOfMana);

	// TEMP: Reset attributes to default values.  Eventually this will be driven by a spread sheet.
	AbilitySystemComponent->SetNumericAttributeBase(ULyraManaSet::GetManaAttribute(), ManaSet->GetMaxMana());

	ClearGameplayTags();

	OnManaChanged.Broadcast(this, ManaSet->GetMana(), ManaSet->GetMana(), nullptr);
	OnMaxManaChanged.Broadcast(this, ManaSet->GetMana(), ManaSet->GetMana(), nullptr);
}

void ULyraManaComponent::UninitializeFromAbilitySystem()
{
	ClearGameplayTags();

	if (ManaSet)
	{
		ManaSet->OnManaChanged.RemoveAll(this);
		ManaSet->OnMaxManaChanged.RemoveAll(this);
		ManaSet->OnOutOfMana.RemoveAll(this);
	}

	ManaSet = nullptr;
	AbilitySystemComponent = nullptr;
}

void ULyraManaComponent::ClearGameplayTags()
{
	if (AbilitySystemComponent)
	{
		// Health Component already takes care of this and we don't want to clear when mana is out, only health.
		//AbilitySystemComponent->SetLooseGameplayTagCount(LyraGameplayTags::Status_Death_Dying, 0);
		//AbilitySystemComponent->SetLooseGameplayTagCount(LyraGameplayTags::Status_Death_Dead, 0);
	}
}

float ULyraManaComponent::GetMana() const
{
	return (ManaSet ? ManaSet->GetMana() : 0.0f);
}

float ULyraManaComponent::GetMaxMana() const
{
	return (ManaSet ? ManaSet->GetMaxMana() : 0.0f);
}

float ULyraManaComponent::GetManaNormalized() const
{
	if (ManaSet)
	{
		const float Mana = ManaSet->GetMana();
		const float MaxMana = ManaSet->GetMaxMana();

		return ((MaxMana > 0.0f) ? (Mana / MaxMana) : 0.0f);
	}

	return 0.0f;
}

void ULyraManaComponent::HandleManaChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnManaChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void ULyraManaComponent::HandleMaxManaChanged(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
	OnMaxManaChanged.Broadcast(this, OldValue, NewValue, DamageInstigator);
}

void ULyraManaComponent::HandleOutOfMana(AActor* DamageInstigator, AActor* DamageCauser, const FGameplayEffectSpec* DamageEffectSpec, float DamageMagnitude, float OldValue, float NewValue)
{
#if WITH_SERVER_CODE
	if (AbilitySystemComponent && DamageEffectSpec)
	{
		// Send a standardized verb message that other systems can observe
		{
			FLyraVerbMessage Message;
			Message.Verb = TAG_Lyra_OutOfMana_Message;
			Message.Instigator = DamageInstigator;
			Message.InstigatorTags = *DamageEffectSpec->CapturedSourceTags.GetAggregatedTags();
			Message.Target = ULyraVerbMessageHelpers::GetPlayerStateFromObject(AbilitySystemComponent->GetAvatarActor());
			Message.TargetTags = *DamageEffectSpec->CapturedTargetTags.GetAggregatedTags();
			//@TODO: Fill out context tags, and any non-ability-system source/instigator tags
			//@TODO: Determine if it's an opposing team kill, self-own, team kill, etc...

			UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(GetWorld());
			MessageSystem.BroadcastMessage(Message.Verb, Message);
		}

		//@TODO: assist messages (could compute from damage dealt elsewhere)?
	}

#endif // #if WITH_SERVER_CODE
}

void ULyraManaComponent::OnRep_DeathState(ELyraDeathState OldDeathState)
{
	const ELyraDeathState NewDeathState = DeathState;

	// Revert the death state for now since we rely on StartDeath and FinishDeath to change it.
	DeathState = OldDeathState;

	if (OldDeathState > NewDeathState)
	{
		// The server is trying to set us back but we've already predicted past the server state.
		UE_LOG(LogLyra, Warning, TEXT("LyraManaComponent: Predicted past server death state [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		return;
	}

	if (OldDeathState == ELyraDeathState::NotDead)
	{
		if (NewDeathState == ELyraDeathState::DeathStarted)
		{
			StartDeath();
		}
		else if (NewDeathState == ELyraDeathState::DeathFinished)
		{
			StartDeath();
			FinishDeath();
		}
		else
		{
			UE_LOG(LogLyra, Error, TEXT("LyraManaComponent: Invalid death transition [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		}
	}
	else if (OldDeathState == ELyraDeathState::DeathStarted)
	{
		if (NewDeathState == ELyraDeathState::DeathFinished)
		{
			FinishDeath();
		}
		else
		{
			UE_LOG(LogLyra, Error, TEXT("LyraManaComponent: Invalid death transition [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
		}
	}

	ensureMsgf((DeathState == NewDeathState), TEXT("LyraManaComponent: Death transition failed [%d] -> [%d] for owner [%s]."), (uint8)OldDeathState, (uint8)NewDeathState, *GetNameSafe(GetOwner()));
}

void ULyraManaComponent::StartDeath()
{
	if (DeathState != ELyraDeathState::NotDead)
	{
		return;
	}

	DeathState = ELyraDeathState::DeathStarted;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(LyraGameplayTags::Status_Death_Dying, 1);
	}

	AActor* Owner = GetOwner();
	check(Owner);

	OnDeathStarted.Broadcast(Owner);

	Owner->ForceNetUpdate();
}

void ULyraManaComponent::FinishDeath()
{
	if (DeathState != ELyraDeathState::DeathStarted)
	{
		return;
	}

	DeathState = ELyraDeathState::DeathFinished;

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->SetLooseGameplayTagCount(LyraGameplayTags::Status_Death_Dead, 1);
	}

	AActor* Owner = GetOwner();
	check(Owner);

	OnDeathFinished.Broadcast(Owner);

	Owner->ForceNetUpdate();
}

void ULyraManaComponent::DamageSelfDestruct(bool bFellOutOfWorld)
{
	if ((DeathState == ELyraDeathState::NotDead) && AbilitySystemComponent)
	{
		const TSubclassOf<UGameplayEffect> DamageGE = ULyraAssetManager::GetSubclass(ULyraGameData::Get().DamageGameplayEffect_SetByCaller);
		if (!DamageGE)
		{
			UE_LOG(LogLyra, Error, TEXT("LyraManaComponent: DamageSelfDestruct failed for owner [%s]. Unable to find gameplay effect [%s]."), *GetNameSafe(GetOwner()), *ULyraGameData::Get().DamageGameplayEffect_SetByCaller.GetAssetName());
			return;
		}

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DamageGE, 1.0f, AbilitySystemComponent->MakeEffectContext());
		FGameplayEffectSpec* Spec = SpecHandle.Data.Get();

		if (!Spec)
		{
			UE_LOG(LogLyra, Error, TEXT("LyraManaComponent: DamageSelfDestruct failed for owner [%s]. Unable to make outgoing spec for [%s]."), *GetNameSafe(GetOwner()), *GetNameSafe(DamageGE));
			return;
		}

		Spec->AddDynamicAssetTag(TAG_Gameplay_DamageSelfDestruct);

		if (bFellOutOfWorld)
		{
			Spec->AddDynamicAssetTag(TAG_Gameplay_FellOutOfWorld);
		}

		const float DamageAmount = GetMaxMana();

		Spec->SetSetByCallerMagnitude(LyraGameplayTags::SetByCaller_Damage, DamageAmount);
		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec);
	}
}

