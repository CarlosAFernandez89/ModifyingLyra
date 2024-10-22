// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LyraGameplayAbility.h"
#include "LyraGameplayAbility_RangeTarget.generated.h"

enum ECollisionChannel : int;
enum class ELyraAbilityTargetingSource : uint8;

class APawn;
class UObject;
struct FCollisionQueryParams;
struct FFrame;
struct FGameplayAbilityActorInfo;
struct FGameplayEventData;
struct FGameplayTag;
struct FGameplayTagContainer;

/**
 * ULyraGameplayAbility_RangeTarget
 *
 * An ability with ranged targeting capabilities
 */
UCLASS()
class LYRAGAME_API ULyraGameplayAbility_RangeTarget : public ULyraGameplayAbility
{
	GENERATED_BODY()

public:

	ULyraGameplayAbility_RangeTarget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~UGameplayAbility interface
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	//~End of UGameplayAbility interface

	UPROPERTY(EditDefaultsOnly, Category=Targeting)
	ELyraAbilityTargetingSource TargetingSource;

	UPROPERTY(EditDefaultsOnly, Category=Targeting)
	float MaxTargetRange;

	UPROPERTY(EditDefaultsOnly, Category=Targeting)
	int32 NumberOfProjectiles;

	UPROPERTY(EditDefaultsOnly, Category=Targeting)
	float SpreadAngle;
	
	UPROPERTY(EditDefaultsOnly, Category=Targeting)
	float SweepRadius;

protected:

protected:
	struct FRangedFiringInput
	{
		// Start of the trace
		FVector StartTrace;

		// End of the trace if aim were perfect
		FVector EndAim;

		// The direction of the trace if aim were perfect
		FVector AimDir;

		// The weapon instance / source of weapon data
		ALyraCharacter* CharacterData = nullptr;

		// Can we play bullet FX for hits during this trace
		bool bCanPlayBulletFX = false;

		FRangedFiringInput()
			: StartTrace(ForceInitToZero)
			, EndAim(ForceInitToZero)
			, AimDir(ForceInitToZero)
		{
		}
	};
	
	static int32 FindFirstPawnHitResult(const TArray<FHitResult>& HitResults);
	
	// Does a single trace, either sweeping or ray depending on if SweepRadius is above zero
	FHitResult Trace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHitResults) const;

	// Wrapper around Trace to handle trying to do a ray trace before falling back to a sweep trace if there were no hits and SweepRadius is above zero 
	FHitResult DoSingleTrace(const FVector& StartTrace, const FVector& EndTrace, float SweepRadius, bool bIsSimulated, OUT TArray<FHitResult>& OutHits) const;

	// Traces attack
	void TraceAttack(const FRangedFiringInput& InputData, OUT TArray<FHitResult>& OutHits);

	virtual void AddAdditionalTraceIgnoreActors(FCollisionQueryParams& TraceParams) const;

	// Determine the trace channel to use for the weapon trace(s)
	virtual ECollisionChannel DetermineTraceChannel(FCollisionQueryParams& TraceParams, bool bIsSimulated) const;

	void PerformLocalTargeting(OUT TArray<FHitResult>& OutHits);
	
	FVector GetTargetingSourceLocation() const;
	FTransform GetTargetingTransform(APawn* SourcePawn, ELyraAbilityTargetingSource Source) const;


	void OnTargetDataReadyCallback(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag ApplicationTag);

	UFUNCTION(BlueprintCallable)
	void StartRangedTargeting();
	
	// Called when target data is ready
	UFUNCTION(BlueprintImplementableEvent)
	void OnRangedTargetDataReady(const FGameplayAbilityTargetDataHandle& TargetData);

private:
	FDelegateHandle OnTargetDataReadyCallbackDelegateHandle;
};
