#include "LyraEnemySpawner.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Character/LyraPawnData.h"
#include "Character/LyraPawnExtensionComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameModes/LyraExperienceManagerComponent.h"
#include "Teams/LyraTeamSubsystem.h"

ALyraEnemySpawner::ALyraEnemySpawner(): BehaviorTree(nullptr)
{
	PrimaryActorTick.bCanEverTick = false;
}

void ALyraEnemySpawner::BeginPlay()
{
	Super::BeginPlay();
	
	const AGameStateBase* GameState = GetWorld()->GetGameState();
	check(GameState);

	ULyraExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<ULyraExperienceManagerComponent>();
	check(ExperienceComponent);
	ExperienceComponent->CallOrRegister_OnExperienceLoaded_LowPriority(FOnLyraExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}

void ALyraEnemySpawner::OnExperienceLoaded(const ULyraExperienceDefinition* Experience)
{
#if WITH_SERVER_CODE
	if (HasAuthority())
	{
		ServerCreateEnemies();
	}
#endif
}

void ALyraEnemySpawner::ServerCreateEnemies()
{
	if (ControllerClass == nullptr)
	{
		return;
	}

	// Create them
	for (int32 Count = 0; Count < NumberOfEnemiesToCreate; ++Count)
	{
		SpawnOneEnemy();
	}
}

// similar to UAIBlueprintHelperLibrary::SpawnAIFromClass but we use the controller class defined here instead of the one set on the pawn
// #todo could make a new static function in  UAIBlueprintHelperLibrary, like SpawnAIFromClassSpecifyController
APawn* ALyraEnemySpawner::SpawnEnemyFromClass(UObject* WorldContextObject, ULyraPawnData* LoadedPawnData, UBehaviorTree* BehaviorTreeToRun, FVector Location, FRotator Rotation, bool bNoCollisionFail, AActor *PawnOwner, TSubclassOf
                                     <AAIController> ControllerClassToSpawn)
{
	// GetWorld(), LoadedPawnData->PawnClass, BehaviorTree, GetActorLocation(), GetActorRotation(), true, this, ControllerClass
	
	APawn* NewPawn = nullptr;

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World && *LoadedPawnData->PawnClass)
	{
		FActorSpawnParameters ActorSpawnParams;
		ActorSpawnParams.Owner = PawnOwner;
		ActorSpawnParams.ObjectFlags |= RF_Transient;	// We never want to save spawned AI pawns into a map
		ActorSpawnParams.SpawnCollisionHandlingOverride = bNoCollisionFail ? ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn : ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
		// defer spawning the pawn to setup the AIController, else it spawns the default controller on spawn if set to spawn AI on spawn
		ActorSpawnParams.bDeferConstruction = ControllerClassToSpawn != nullptr;
		
		NewPawn = World->SpawnActor<APawn>(*LoadedPawnData->PawnClass, Location, Rotation, ActorSpawnParams);
		if (ControllerClassToSpawn)
		{
			NewPawn->AIControllerClass = ControllerClassToSpawn;
			if (ULyraPawnExtensionComponent* PawnExtComp = ULyraPawnExtensionComponent::FindPawnExtensionComponent(NewPawn))
			{
				PawnExtComp->SetPawnData(LoadedPawnData);
			}
			NewPawn->FinishSpawning(FTransform(Rotation, Location, GetActorScale3D()));
		}
		
		if (NewPawn != nullptr)
		{
			if (NewPawn->Controller == NULL)
			{
				NewPawn->SpawnDefaultController();
			}

			if (BehaviorTreeToRun != nullptr)
			{
				if (AAIController* AIController = Cast<AAIController>(NewPawn->Controller))
				{
					AIController->RunBehaviorTree(BehaviorTreeToRun);
				}
			}
		}
	}

	return NewPawn;
}

void ALyraEnemySpawner::OnSpawnedPawnDestroyed(AActor* DestroyedActor)
{
	if (!HasAuthority())
	{
		return;
	}

	//TODO: remove from the SpawnedEnemyList list correctly
	if(APawn* Pawn = Cast<APawn>(DestroyedActor))
	{
		SpawnedEnemyList.RemoveSingle(Cast<AAIController>(Pawn->Controller));
	}
	
	if (ShouldRespawn)
	{
		FTimerHandle RespawnHandle;
		GetWorldTimerManager().SetTimer(RespawnHandle, this, &ThisClass::SpawnOneEnemy, RespawnTime, false);
	}
}

void ALyraEnemySpawner::SpawnOneEnemy()
{
	ULyraPawnData* LoadedPawnData = PawnData.Get();
	if (!PawnData.IsValid())
	{
		LoadedPawnData = PawnData.LoadSynchronous();
	}

	if (LoadedPawnData)
	{
		if (APawn* SpawnedNPC = SpawnEnemyFromClass(GetWorld(), LoadedPawnData, BehaviorTree, GetActorLocation(), GetActorRotation(), true, this, ControllerClass))
		{
			bool bWantsPlayerState = true;
			if (const AAIController* AIController = Cast<AAIController>(SpawnedNPC->Controller))
			{
				bWantsPlayerState = AIController->bWantsPlayerState;
			}
			
			if (ULyraPawnExtensionComponent* PawnExtComp = ULyraPawnExtensionComponent::FindPawnExtensionComponent(SpawnedNPC))
			{
				AActor* AbilityOwner = bWantsPlayerState ? SpawnedNPC->GetPlayerState() : Cast<AActor>(SpawnedNPC);
				
				if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(AbilityOwner))
				{
					PawnExtComp->InitializeAbilitySystem(Cast<ULyraAbilitySystemComponent>(AbilitySystemComponent), AbilityOwner);
				}
			}

			if (ULyraTeamSubsystem* TeamSubsystem = UWorld::GetSubsystem<ULyraTeamSubsystem>(GetWorld()))
			{
				TeamSubsystem->ChangeTeamForActor(SpawnedNPC->Controller, TeamID);
			}
			
			SpawnedEnemyList.Add(Cast<AAIController>(SpawnedNPC->Controller));

			SpawnedNPC->OnDestroyed.AddDynamic(this, &ThisClass::OnSpawnedPawnDestroyed);
			OnEnemyPawnSpawned(SpawnedNPC);
		}
	}
}