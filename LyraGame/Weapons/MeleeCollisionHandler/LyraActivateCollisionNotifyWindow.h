// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/AnimGraphRuntime/Public/AnimNotifies/AnimNotify_PlayMontageNotify.h"
#include "LyraActivateCollisionNotifyWindow.generated.h"

/**
 * A notification window class for activating collision during an animation montage.
 */
UCLASS(meta = (DisplayName = "LyraActivateCollision"))
class LYRAGAME_API ULyraActivateCollisionNotifyWindow : public UAnimNotify_PlayMontageNotifyWindow
{
	GENERATED_BODY()

	public:
	ULyraActivateCollisionNotifyWindow();

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

};
