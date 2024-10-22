// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraPawnData.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraPawnData)

ULyraPawnData::ULyraPawnData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PawnDisplayName = FName();
	PawnClass = nullptr;
	InputConfig = nullptr;
	DefaultCameraMode = nullptr;
}

