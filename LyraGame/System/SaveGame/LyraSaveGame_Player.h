// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QuestSaveState.h"
#include "GameFramework/SaveGame.h"
#include "Inventory/LyraInventoryItemInstance.h"
#include "LyraSaveGame_Player.generated.h"


USTRUCT()
struct FLyraInventoryEntrySaveData
{
	GENERATED_BODY()

	// Serialized data for the inventory item instance
	UPROPERTY()
	FLyraInventoryItemInstanceSaveData InstanceSaveData;
	
};


USTRUCT()
struct FLyraInventoryListSaveData
{
	GENERATED_BODY()

	// List of serialized inventory entries
	UPROPERTY()
	TArray<FLyraInventoryEntrySaveData> SavedEntries;

	// The name of the owner component (could be a controller or actor component)
	UPROPERTY()
	FString OwnerComponentName;
};

/**
 * 
 */
UCLASS()
class LYRAGAME_API ULyraSaveGame_Player : public USaveGame
{
	GENERATED_BODY()

public:

	//Inventory
	UPROPERTY()
	FLyraInventoryListSaveData InventoryList;
	UPROPERTY()
	FLyraInventoryEntrySaveData InventoryEntry;
	UPROPERTY()
	FQuestSaveStateData QuestData;
	UPROPERTY()
	TMap<FName, FDateTime> CompletedQuests;
};
