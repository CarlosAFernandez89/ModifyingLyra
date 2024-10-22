// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraClearHitActorsNotify.h"

#include "Equipment/LyraQuickBarComponent.h"
#include "Weapons/LyraMeleeWeaponInstance.h"

ULyraClearHitActorsNotify::ULyraClearHitActorsNotify()
{
	NotifyName = TEXT("LyraClearHitActorsNotify");
}

void ULyraClearHitActorsNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if(MeshComp)
	{
		if(const APawn* PawnOwner = Cast<APawn>(MeshComp->GetOwner()))
		{
			if(const AController* Controller = PawnOwner->GetController())
			{
				if(ULyraQuickBarComponent* QuickBarComponent = Cast<ULyraQuickBarComponent>(Controller->GetComponentByClass(ULyraQuickBarComponent::StaticClass())))
				{
					if(ULyraMeleeWeaponInstance* EquippedMeleeWeapon = Cast<ULyraMeleeWeaponInstance>(QuickBarComponent->GetEquippedItem()))
					{
						EquippedMeleeWeapon->ClearHitActors();
					}
				}
			}
		}
	}
}

