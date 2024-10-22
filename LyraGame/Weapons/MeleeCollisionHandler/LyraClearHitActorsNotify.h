// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AnimGraphRuntime/Public/AnimNotifies/AnimNotify_PlayMontageNotify.h"
#include "LyraClearHitActorsNotify.generated.h"

/**
 * 
 */
UCLASS(meta = (DisplayName = "LyraClearHitActors"))
class LYRAGAME_API ULyraClearHitActorsNotify : public UAnimNotify_PlayMontageNotify
{
	GENERATED_BODY()

	ULyraClearHitActorsNotify();

	/* overridden method */
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
