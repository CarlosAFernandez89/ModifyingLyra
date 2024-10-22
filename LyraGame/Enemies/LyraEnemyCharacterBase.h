// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Character/LyraCharacterWithAbilities.h"

#include "LyraEnemyCharacterBase.generated.h"

class UBehaviorTree;
class ULyraPawnData;

UCLASS(Blueprintable)
class LYRAGAME_API ALyraEnemyCharacterBase : public ALyraCharacterWithAbilities
{
	GENERATED_BODY()

public:

	virtual void BeginPlay() override;
	
	static const FName NAME_LyraAbilityReady;

	void SetPawnData(const ULyraPawnData* InPawnData);

	UFUNCTION(BlueprintCallable, Category="Lyra|PawnData")
	FName GetEnemyPawnName() const;

protected:
	virtual void OnAbilitySystemInitialized() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=PawnData)
	const ULyraPawnData* PawnData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=PawnData)
	TSubclassOf<AController> DefaultAIController;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=PawnData)
	UBehaviorTree* DefaultBehaviorTree;
	
private:


	
};
