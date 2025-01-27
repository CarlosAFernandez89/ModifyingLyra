﻿#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GameFramework/Actor.h"
#include "GameModes/LyraExperienceDefinition.h"
#include "LyraEnemySpawner.generated.h"

class AAIController;
class ULyraPawnData;

UCLASS()
class LYRAGAME_API ALyraEnemySpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALyraEnemySpawner();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Pawn)
	TSoftObjectPtr<ULyraPawnData> PawnData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	FGenericTeamId TeamID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UBehaviorTree* BehaviorTree;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void OnExperienceLoaded(const ULyraExperienceDefinition* Experience);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Spawn, meta=(UIMin=1))
	int32 NumberOfEnemiesToCreate = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Spawn)
	bool ShouldRespawn = false;
	
	/**
	 * Time it takes after pawn death to spawn a new pawn
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Spawn, meta=(EditCondition=ShouldRespawn, EditConditionHides))
	float RespawnTime = 1.f;

	UPROPERTY(EditAnywhere, Category=Gameplay)
	TSubclassOf<AAIController> ControllerClass;

	/** #todo find out how to pool the AIControllers. At the moment the controllers are destroyed with APawn::DetachFromControllerPendingDestroy() **/
	UPROPERTY(Transient)
	TArray<TObjectPtr<AAIController>> SpawnedEnemyList;

	virtual void ServerCreateEnemies();

	APawn* SpawnEnemyFromClass(UObject* WorldContextObject, ULyraPawnData* LoadedPawnData, UBehaviorTree* BehaviorTreeToRun,
	                        FVector Location,
	                        FRotator Rotation, bool bNoCollisionFail, AActor* PawnOwner, TSubclassOf<AAIController> ControllerClassToSpawn);

	UFUNCTION(BlueprintImplementableEvent)
	void OnEnemyPawnSpawned(APawn* SpawnedPawn);

	UFUNCTION()
	void OnSpawnedPawnDestroyed(AActor* DestroyedActor);

	virtual void SpawnOneEnemy();
};