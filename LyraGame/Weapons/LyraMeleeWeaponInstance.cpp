// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraMeleeWeaponInstance.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "NativeGameplayTags.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility_Death.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Physics/PhysicalMaterialWithTags.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_MeleeWeaponHit, "GameplayEvent.Montage.Hit")

FVector FLyraCollidingComponent::GetSocketLocation(const FName& SocketName) const
{
	if( SocketName.IsNone() )
	{
		return CollidingComponent->GetComponentLocation();
	}
	else
	{
		return CollidingComponent->GetSocketLocation(SocketName);
	}
}

ULyraMeleeWeaponInstance::ULyraMeleeWeaponInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer), bTraceComplex(0), TraceRadius(0.1f), TraceCheckInterval(0.025f), bIsCollisionActivated(0),
	  bCanPerformTrace(0),
	  bDebug(0)
{
	ObjectTypesToCollideWith.Add(EObjectTypeQuery::ObjectTypeQuery3);
}

void ULyraMeleeWeaponInstance::PostLoad()
{
	Super::PostLoad();
}	

#if WITH_EDITOR
void ULyraMeleeWeaponInstance::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void ULyraMeleeWeaponInstance::OnEquipped()
{
	Super::OnEquipped();

	for (const AActor* SpawnedActor : GetSpawnedActors())
	{
		TArray<FName> SocketNames;

		// Look for components that may have sockets
		UPrimitiveComponent* PrimitiveComponent = SpawnedActor->FindComponentByClass<UPrimitiveComponent>();

		if (PrimitiveComponent)
		{
			// Get all socket names from PrimitiveComponent
			SocketNames = PrimitiveComponent->GetAllSocketNames();
		}

		if (SocketNames.Num() > 0)
		{
			// Add the colliding component with its sockets to ActiveCollidingComponents
			ActiveCollidingComponents.Add(FLyraCollidingComponent(PrimitiveComponent, SocketNames));
		}
	}

}

void ULyraMeleeWeaponInstance::OnUnequipped()
{
	OnEquipmentUnequipped.Broadcast();

	ActiveCollidingComponents.Empty();
	
	Super::OnUnequipped();
}

float ULyraMeleeWeaponInstance::GetDistanceAttenuation(float Distance, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags) const
{
	return 1.f;
}

float ULyraMeleeWeaponInstance::GetPhysicalMaterialAttenuation(const UPhysicalMaterial* PhysicalMaterial,
                                                               const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags) const
{
	float CombinedMultiplier = 1.0f;
	if (const UPhysicalMaterialWithTags* PhysMatWithTags = Cast<const UPhysicalMaterialWithTags>(PhysicalMaterial))
	{
		for (const FGameplayTag MaterialTag : PhysMatWithTags->Tags)
		{
			if (const float* pTagMultiplier = MaterialDamageMultiplier.Find(MaterialTag))
			{
				CombinedMultiplier *= *pTagMultiplier;
			}
		}
	}

	return CombinedMultiplier;
}

float ULyraMeleeWeaponInstance::GetWeaponBaseDamage() const
{
	return WeaponDamage.GetRandomDamageValue();
}

bool ULyraMeleeWeaponInstance::IsIgnoredClass(const TSubclassOf<AActor>& ActorClass)
{
	for( const auto& IgnoredClass : IgnoredClasses )
	{
		if(ActorClass->IsChildOf(IgnoredClass))
			return true;
	}
	return false;
}

bool ULyraMeleeWeaponInstance::IsIgnoredProfileName(const FName ProfileName) const
{
	return IgnoredCollisionProfileNames.Contains(ProfileName);
}

void ULyraMeleeWeaponInstance::ActivateCollision()
{
	if(bIsCollisionActivated == false)
	{
		bIsCollisionActivated = true;
		// clear hit actors
		ClearHitActors();

		// set timer which will check for collisions
		if(UWorld* World = GetWorld())
		{
			TraceCheckLoop(); //do this once right away while the timer starts.
			World->GetTimerManager().SetTimer( TimerHandle_TraceCheck, this, &ULyraMeleeWeaponInstance::TraceCheckLoop, TraceCheckInterval, true );
		}
	}
}

void ULyraMeleeWeaponInstance::DeactivateCollision()
{
	if(bIsCollisionActivated)
	{
		bIsCollisionActivated = false;
		bCanPerformTrace = false;

		// clear timer checking for collision
		if(UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer( TimerHandle_TraceCheck);
		}
	}
}

void ULyraMeleeWeaponInstance::ClearHitActors()
{
	for( auto& Component : ActiveCollidingComponents )
	{
		Component.HitActors.Empty();
	}
}

void ULyraMeleeWeaponInstance::TraceCheckLoop()
{
	if(bCanPerformTrace)
	{
		PerformTraceCheck();
	}

	UpdateSocketLocations();
	bCanPerformTrace = true;
}

void ULyraMeleeWeaponInstance::UpdateSocketLocations()
{
	for( auto& Component : ActiveCollidingComponents )
	{
		if( IsValid( Component.CollidingComponent ) )
		{
			// for each colliding component get its location and store in LastFrameSocketLocations map
			for( const FName& SocketName : Component.Sockets )
			{
				// get socket name
				FVector SocketLocation = Component.GetSocketLocation(SocketName);

				// generate unique socket name
				FName UniqueSocketName = GenerateUniqueSocketName(Component.CollidingComponent, SocketName);

				// store values in map
				LastFrameSocketLocations.Add(UniqueSocketName, SocketLocation);
			}
		}
	}
}

void ULyraMeleeWeaponInstance::PerformTraceCheck()
{
	for (auto& Component : ActiveCollidingComponents)
	{
		if (IsValid(Component.CollidingComponent))
		{
			HandleComponent(Component);
		}
	}
}

void ULyraMeleeWeaponInstance::HandleComponent(FLyraCollidingComponent& Component)
{
	for (const FName& SocketName : Component.Sockets)
	{
		FName UniqueSocketName = GenerateUniqueSocketName(Component.CollidingComponent, SocketName);
		FVector StartTrace = *LastFrameSocketLocations.Find(UniqueSocketName);
		FVector EndTrace = Component.GetSocketLocation(SocketName);

		TArray<FHitResult> HitResults;
		TArray<AActor*> WantedIgnoredActors{ Component.HitActors };
		WantedIgnoredActors.Add(GetPawn()); //Gets the owner of the weapon, usually the player.
		WantedIgnoredActors.Append(IgnoredActors);

		const bool WasHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
			this, StartTrace, EndTrace, TraceRadius, ObjectTypesToCollideWith,
			bTraceComplex, WantedIgnoredActors, EDrawDebugTrace::Type::None, HitResults, true);

		if (WasHit)
		{
			for (const FHitResult& HitResult : HitResults)
			{
				ProcessHitResult(HitResult, Component);
			}
		}
#if WITH_EDITOR
		if (bDebug)
		{
			DrawDebugTrace(StartTrace, EndTrace);
		}
#endif
	}
}

void ULyraMeleeWeaponInstance::ProcessHitResult(const FHitResult& HitResult, FLyraCollidingComponent& Component)
{
	if (AActor* HitActor = HitResult.GetActor())
	{
		if (!Component.HitActors.Contains(HitActor) &&
			!IsIgnoredClass(HitActor->GetClass()) &&
			!IsIgnoredProfileName(HitResult.Component->GetCollisionProfileName()))
		{
			Component.HitActors.Add(HitActor);
			OnWeaponHit(HitResult, Component.CollidingComponent);
#if WITH_EDITOR
			if (bDebug)
			{
				DrawHitSphere(HitResult.Location);
			}
#endif
		}
	}
}

void ULyraMeleeWeaponInstance::OnWeaponHit_Implementation(const FHitResult& HitResult,
	UPrimitiveComponent* Component)
{
	//TODO: Apply the damage and effects here. Can also be done in blueprint.
	FGameplayEventData EventData;
	EventData.Instigator = GetPawn()->GetController();
	EventData.Target = HitResult.GetActor();
	EventData.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromHitResult(HitResult);
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetPawn(), TAG_MeleeWeaponHit, EventData);
}

FName ULyraMeleeWeaponInstance::GenerateUniqueSocketName(const UPrimitiveComponent* CollidingComponent, FName Socket)
{
	// concatenate colliding component name + socket to get unique name e.g 'SwordMeshSocket01'
	return FName( *CollidingComponent->GetName().Append(Socket.ToString()));
}

void ULyraMeleeWeaponInstance::DrawHitSphere(const FVector& Location) const
{
	if(const UWorld* World = GetWorld())
	{
		const float Radius = TraceRadius >= 8.f ? TraceRadius : 8.f;
		DrawDebugSphere(World, Location, Radius, 12, FColor::Green, false, 5.f);
	}
}

void ULyraMeleeWeaponInstance::DrawDebugTrace(const FVector& Start, const FVector& End) const
{
	if(const UWorld* World = GetWorld())
	{
		DrawDebugCylinder(World, Start, End, TraceRadius, 12, FColor::Red, false, 5.f);
	}
}
