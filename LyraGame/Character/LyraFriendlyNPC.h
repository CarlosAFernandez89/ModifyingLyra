// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interaction/IInteractableTarget.h"
#include "Inventory/IPickupable.h"
#include "LyraFriendlyNPC.generated.h"

UCLASS()
class LYRAGAME_API ALyraFriendlyNPC : public ACharacter, public IInteractableTarget, public IPickupable, public IInteractionInfoAndActions
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ALyraFriendlyNPC();

	virtual void GatherInteractionOptions(const FInteractionQuery& InteractQuery, FInteractionOptionBuilder& InteractionBuilder) override;
	virtual FInventoryPickup GetPickupInventory() const override;

	virtual FInteractionOption GetInteractionOption_Implementation() override;
	virtual void OnInteractEvent_Implementation(FGameplayEventData Payload) override;
	virtual void SetInteractionText_Implementation(const FText& InteractionText) override;

protected:
	UPROPERTY(EditAnywhere)
	FInteractionOption Option;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory", meta = (ExposeOnSpawn = "true"))
	FInventoryPickup StaticInventory;
	
};
