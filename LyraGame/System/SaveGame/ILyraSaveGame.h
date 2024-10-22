// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QuestSaveState.h"
#include "Inventory/LyraInventoryManagerComponent.h"
#include "UObject/Interface.h"
#include "ILyraSaveGame.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UILyraSaveGame : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class LYRAGAME_API IILyraSaveGame
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="SaveGame")
	void SaveInventory(const FLyraInventoryList& InventoryList);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="SaveGame")
	void LoadInventory(FLyraInventoryList& InventoryList);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="SaveGame")
	void SaveQuestState(const FQuestSaveStateData& QuestSaveStateData);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="SaveGame")
	void LoadQuestState(FQuestSaveStateData& QuestSaveStateData);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="SaveGame")
	void SaveCompletedQuestsName(const TMap<FName, FDateTime>& QuestGUID);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="SaveGame")
	void LoadCompletedQuestsName(TMap<FName, FDateTime>& QuestGUID);
};
