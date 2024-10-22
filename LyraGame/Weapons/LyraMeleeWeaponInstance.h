// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "LyraWeaponInstance.h"
#include "AbilitySystem/LyraAbilitySourceInterface.h"

#include "LyraMeleeWeaponInstance.generated.h"

class ALyraCollisionHandlerComponent;
class UPhysicalMaterial;

USTRUCT(BlueprintType)
struct FLyraDamageValues
{
	GENERATED_BODY()

	// Base damage value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float BaseDamage;

	// Max damage value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float MaxDamage;

	// Constructor to initialize default values
	FLyraDamageValues()
		: BaseDamage(0.0f), MaxDamage(0.0f)
	{
	}

	FLyraDamageValues(float InBaseDamage, float InMaxDamage)
		: BaseDamage(InBaseDamage), MaxDamage(InMaxDamage)
	{
	}

	float GetRandomDamageValue() const { return FMath::RandRange(BaseDamage, MaxDamage); }
};

USTRUCT(BlueprintType)
struct FLyraCollidingComponent
{
	GENERATED_BODY()

	FLyraCollidingComponent() {}

	FLyraCollidingComponent(class UPrimitiveComponent* InCollisionComponent, TArray<FName> InSockets)
		: CollidingComponent(InCollisionComponent),
		  Sockets(InSockets)
	{
		// if doesn't have any sockets
		// add default one which will represent component world location
		if( Sockets.Num() == 0 )
		{
			Sockets.Add( NAME_None );
		}
	}


	/* Component which will be used to find socket locations from e.g Sword Static Mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Lyra|CollidingComponent")
	UPrimitiveComponent* CollidingComponent = nullptr;

	/* Socket names that should exist in given Component in order to retrieve their locations */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "CollidingComponent" )
	TArray<FName> Sockets;

	/* Hidden property that stores hit actors by this Component */
	UPROPERTY( BlueprintReadOnly, Category = "CollidingComponent" )
	TArray<AActor*> HitActors;

	/** Returns location on component by given socket name */
	FVector GetSocketLocation( const FName& SocketName ) const;

	/* Override == operator to compare these structs on its Component pointer */
	FORCEINLINE bool operator == (const FLyraCollidingComponent& Other) const
	{
		return this->CollidingComponent == Other.CollidingComponent;
	}
	
};

/**
 * ULyraMeleeWeaponInstance
 *
 * A piece of equipment representing a melee weapon spawned and applied to a pawn
 */
UCLASS()
class ULyraMeleeWeaponInstance : public ULyraWeaponInstance, public ILyraAbilitySourceInterface
{
	GENERATED_BODY()

public:
	ULyraMeleeWeaponInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PostLoad() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:

	//The base weapon damage of the weapon.
	UPROPERTY(EditAnywhere, Category = "Weapon Config")
	FLyraDamageValues WeaponDamage;
	
	// List of special tags that affect how damage is dealt
	// These tags will be compared to tags in the physical material of the thing being hit
	// If more than one tag is present, the multipliers will be combined multiplicatively
	UPROPERTY(EditAnywhere, Category = "Weapon Config")
	TMap<FGameplayTag, float> MaterialDamageMultiplier;

public:

	//~ULyraEquipmentInstance interface
	virtual void OnEquipped() override;
	virtual void OnUnequipped() override;
	//~End ULyraEquipmentInstance interface

	//~ILyraAbilitySourceInterface interface
	virtual float GetDistanceAttenuation(float Distance, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr) const override;
	virtual float GetPhysicalMaterialAttenuation(const UPhysicalMaterial* PhysicalMaterial, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr) const override;
	virtual float GetWeaponBaseDamage() const override;
	//~End of ILyraAbilitySourceInterface interface

public:
	/* Whether to use trace complex option during trace check or not */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponCollision")
	uint32 bTraceComplex : 1;

	/* Radius of sphere trace checking collision */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponCollision")
	float TraceRadius;

	/* How often there is trace check while collision is activated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponCollision")
	float TraceCheckInterval;

	/* Classes that will be ignored while checking collision, may be friendly AI etc. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponCollision")
	TArray<TSubclassOf<AActor>> IgnoredClasses;

	/** 
	* Profile names that components will be ignored with.
	* May be useful to ignore collision capsule which profile name is 'Pawn' 
	* and collide only with its Character Mesh which profile name is 'CharacterMesh'
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyra|CollisionHandlerComponent")
	TArray<FName> IgnoredCollisionProfileNames;

	/* Types of objects to collide with - Pawn, WordStatic etc. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyra|CollisionHandlerComponent")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesToCollideWith;

	/**
	 * Actors which should always be ignored performing trace checks
	 * Means that they will never be hit
	 */
	UPROPERTY( BlueprintReadWrite, Category = "Lyra|CollisionHandlerComponent")
	TArray<AActor*> IgnoredActors;

	/* Checks whether given class should be ignored or not */
	bool IsIgnoredClass(const TSubclassOf<AActor>& ActorClass );

	/* Checks whether given profile name is ignored or not*/
	bool IsIgnoredProfileName(FName ProfileName) const;


	/* Array storing colliding components */
	UPROPERTY( BlueprintReadOnly, Category = "Lyra|CollisionHandlerComponent" )
	TArray<FLyraCollidingComponent> ActiveCollidingComponents;

	/* Activates collision */
	UFUNCTION(BlueprintCallable, Category = "Lyra|CollisionHandlerComponent")
	void ActivateCollision();

	/* Deactivates collision */
	UFUNCTION(BlueprintCallable, Category = "Lyra|CollisionHandlerComponent")
	void DeactivateCollision();

	/* Returns true if collision is activated, false otherwise */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Lyra|CollisionHandlerComponent")
	bool IsCollisionActivated() const { return bIsCollisionActivated; }
	
	/* Clears HitActors array for each colliding component */
	UFUNCTION(BlueprintCallable, Category = "Lyra|CollisionHandlerComponent")
	void ClearHitActors();


protected:
	/* Determines whether collision is activated */
	UPROPERTY()
	uint32 bIsCollisionActivated : 1;

	/* Handle for trace check loop timer */
	UPROPERTY()
	FTimerHandle TimerHandle_TraceCheck;

	/**
	 * Map storing location of sockets in last frame.
	 * For example:	SteelSwordSocket01, FVector(12,32,0)
	 *				SteelSwordSocket02, FVector(12,32,5) etc.
	 */
	UPROPERTY()
	TMap<FName, FVector> LastFrameSocketLocations;

	/* Function called on timer to perform trace check */
	UFUNCTION()
	void TraceCheckLoop();

	/**
	 * Determines whether trace check can be performed.
	 * Used to make sure it won't happen on first timer tick to firstly store socket locations.
	 */
	uint32 bCanPerformTrace : 1;

	/* Updates location values in sockets */
	void UpdateSocketLocations();

	/**
	 * Does a sphere trace between socket locations in last and current frame,
	 * and check whether there is any colliding object between these locations
	 */
	void PerformTraceCheck();
	void HandleComponent(FLyraCollidingComponent& Component);
	void ProcessHitResult(const FHitResult& HitResult, FLyraCollidingComponent& Component);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Lyra|CollisionHandlerComponent")
	void OnWeaponHit(const FHitResult& HitResult, UPrimitiveComponent* Component);

	/**
	 * Generates unique socket name based on given component and socket name e.g 'SteelSwordCollidingSocket01'.
	 * Used to differentiate components if they are using same socket names.
	 */
	static FName GenerateUniqueSocketName(const UPrimitiveComponent* CollidingComponent, FName Socket );

	/************************************************************************/
	/*								DEBUG								*/
	/************************************************************************/
public:
	/* Debug property exists only with editor */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lyra|CollisionHandlerComponent")
	uint32 bDebug : 1;

private:
	UFUNCTION()
	void DrawHitSphere(const FVector& Location) const;

	UFUNCTION()
	void DrawDebugTrace(const FVector& Start, const FVector& End) const;
	/************************************************************************/

};
