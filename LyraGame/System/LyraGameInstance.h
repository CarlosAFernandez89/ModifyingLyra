// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CommonGameInstance.h"
#include "SaveGame/ILyraSaveGame.h"

#include "LyraGameInstance.generated.h"

class ALyraPlayerController;
class UObject;

UCLASS(Config = Game)
class LYRAGAME_API ULyraGameInstance : public UCommonGameInstance, public IILyraSaveGame
{
	GENERATED_BODY()

public:

	ULyraGameInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	ALyraPlayerController* GetPrimaryPlayerController() const;
	
	virtual bool CanJoinRequestedSession() const override;
	virtual void HandlerUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext) override;

	virtual void ReceivedNetworkEncryptionToken(const FString& EncryptionToken, const FOnEncryptionKeyResponse& Delegate) override;
	virtual void ReceivedNetworkEncryptionAck(const FOnEncryptionKeyResponse& Delegate) override;

	//ILyraSaveGame
	virtual void SaveInventory_Implementation(const FLyraInventoryList& InventoryList) override;
	virtual void LoadInventory_Implementation(FLyraInventoryList& InventoryList) override;

	virtual void SaveQuestState_Implementation(const FQuestSaveStateData& QuestSaveStateData) override;
	virtual void LoadQuestState_Implementation(FQuestSaveStateData& QuestSaveStateData) override;
	virtual void SaveCompletedQuestsName_Implementation(const TMap<FName, FDateTime>& QuestGUID) override;
	virtual void LoadCompletedQuestsName_Implementation(TMap<FName, FDateTime>& QuestGUID) override;
	//~ILyraSaveGame

	FString SaveGame_Player_SlotName;

protected:

	virtual void Init() override;
	virtual void Shutdown() override;

	void OnPreClientTravelToSession(FString& URL);

	/** A hard-coded encryption key used to try out the encryption code. This is NOT SECURE, do not use this technique in production! */
	TArray<uint8> DebugTestEncryptionKey;
};
