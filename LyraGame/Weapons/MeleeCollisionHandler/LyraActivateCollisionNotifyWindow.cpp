// Fill out your copyright notice in the Description page of Project Settings.


#include "LyraActivateCollisionNotifyWindow.h"

#include "Equipment/LyraEquipmentInstance.h"
#include "Equipment/LyraQuickBarComponent.h"
#include "Weapons/LyraMeleeWeaponInstance.h"

ULyraActivateCollisionNotifyWindow::ULyraActivateCollisionNotifyWindow()
{
	NotifyName = "LyraActivateCollisionNotifyWindow";
}

void ULyraActivateCollisionNotifyWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float TotalDuration)
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
						EquippedMeleeWeapon->ActivateCollision();
					}
				}
			}
		}
	}
}

void ULyraActivateCollisionNotifyWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
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
						EquippedMeleeWeapon->DeactivateCollision();
					}
				}
			}
		}
	}
}
