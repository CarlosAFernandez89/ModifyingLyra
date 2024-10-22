// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraFriendlyNPC.h"


// Sets default values
ALyraFriendlyNPC::ALyraFriendlyNPC()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void ALyraFriendlyNPC::GatherInteractionOptions(const FInteractionQuery& InteractQuery,
	FInteractionOptionBuilder& InteractionBuilder)
{
	InteractionBuilder.AddInteractionOption(Option);
}

FInventoryPickup ALyraFriendlyNPC::GetPickupInventory() const
{
	return StaticInventory;
}

FInteractionOption ALyraFriendlyNPC::GetInteractionOption_Implementation()
{
	return Option;
}

void ALyraFriendlyNPC::OnInteractEvent_Implementation(FGameplayEventData Payload)
{
	IInteractionInfoAndActions::OnInteractEvent_Implementation(Payload);
}

void ALyraFriendlyNPC::SetInteractionText_Implementation(const FText& InteractionText)
{
	Option.Text = InteractionText;
}

