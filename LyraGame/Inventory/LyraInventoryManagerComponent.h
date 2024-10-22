// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "LyraInventoryManagerComponent.generated.h"

enum class ELyraItemType : uint8;
class ULyraInventoryItemDefinition;
class ULyraInventoryItemInstance;
class ULyraInventoryManagerComponent;
class UObject;
struct FFrame;
struct FLyraInventoryList;
struct FNetDeltaSerializeInfo;
struct FReplicationFlags;

/** A message when an item is added to the inventory */
USTRUCT(BlueprintType)
struct FLyraInventoryChangeMessage
{
	GENERATED_BODY()

	//@TODO: Tag based names+owning actors for inventories instead of directly exposing the component?
	UPROPERTY(BlueprintReadWrite, Category=Inventory)
	TObjectPtr<UActorComponent> InventoryOwner = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = Inventory)
	TObjectPtr<ULyraInventoryItemInstance> Instance = nullptr;

	UPROPERTY(BlueprintReadWrite, Category=Inventory)
	int32 NewCount = 0;

	UPROPERTY(BlueprintReadWrite, Category=Inventory)
	int32 Delta = 0;
};

/** A single entry in an inventory */
USTRUCT(BlueprintType)
struct FLyraInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FLyraInventoryEntry()
	{}

	FString GetDebugString() const;

	ULyraInventoryItemInstance* GetInstance() const
	{
		return Instance;
	}

	void SetInstance(ULyraInventoryItemInstance* InInstance)
	{
		Instance = InInstance;
	}

	int32 GetStackCount() const
	{
		return StackCount;
	}

	void SetStackCount(int32 InStackCount)
	{
		StackCount = InStackCount;
	}

private:
	friend FLyraInventoryList;
	friend ULyraInventoryManagerComponent;

	UPROPERTY()
	TObjectPtr<ULyraInventoryItemInstance> Instance = nullptr;

	UPROPERTY()
	int32 StackCount = 0;

	UPROPERTY(NotReplicated)
	int32 LastObservedCount = INDEX_NONE;
};

/** List of inventory items */
USTRUCT(BlueprintType)
struct FLyraInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

	FLyraInventoryList()
		: OwnerComponent(nullptr)
	{
	}

	FLyraInventoryList(UActorComponent* InOwnerComponent)
		: OwnerComponent(InOwnerComponent)
	{
	}

	TArray<ULyraInventoryItemInstance*> GetAllItems() const;
	TArray<ULyraInventoryItemInstance*> GetAllItemsOfType(ELyraItemType ItemType) const;
	bool IsEmpty() const;

	const TArray<FLyraInventoryEntry>& GetEntries() const
	{
		return Entries;
	}

	void SetEntries(const TArray<FLyraInventoryEntry>& InEntries)
	{
		Entries = InEntries;
	}

	// Getter for OwnerComponent
	UActorComponent* GetOwnerComponent() const
	{
		return OwnerComponent;
	}
	
	// Setter for OwnerComponent
	void SetOwnerComponent(UActorComponent* InOwnerComponent)
	{
		OwnerComponent = InOwnerComponent;
	}

public:
	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FLyraInventoryEntry, FLyraInventoryList>(Entries, DeltaParms, *this);
	}

	ULyraInventoryItemInstance* AddEntry(TSubclassOf<ULyraInventoryItemDefinition> ItemClass, int32 StackCount);
	void AddEntry(ULyraInventoryItemInstance* Instance);

	void RemoveEntry(ULyraInventoryItemInstance* Instance);

private:
	void BroadcastChangeMessage(FLyraInventoryEntry& Entry, int32 OldCount, int32 NewCount);

private:
	friend ULyraInventoryManagerComponent;

private:
	// Replicated list of items
	UPROPERTY()
	TArray<FLyraInventoryEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

template<>
struct TStructOpsTypeTraits<FLyraInventoryList> : public TStructOpsTypeTraitsBase2<FLyraInventoryList>
{
	enum { WithNetDeltaSerializer = true };
};


USTRUCT(BlueprintType)
struct FLyraInventoryUIMessage
{
	GENERATED_BODY()

	FLyraInventoryUIMessage()
	{}

	FLyraInventoryUIMessage(const TSubclassOf<ULyraInventoryItemDefinition>& InItemDef, const int32 InQuantity)
	{
		ItemDef = InItemDef;
		Quantity = InQuantity;
	}
	
	UPROPERTY(BlueprintReadWrite, Category=Inventory)
	TSubclassOf<ULyraInventoryItemDefinition> ItemDef;

	UPROPERTY(BlueprintReadWrite, Category=Inventory)
	int32 Quantity = 0;
	
};







/**
 * Manages an inventory
 */
UCLASS(BlueprintType)
class LYRAGAME_API ULyraInventoryManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULyraInventoryManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	bool CanAddItemDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDef, int32 StackCount = 1);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	ULyraInventoryItemInstance* AddItemDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDef, int32 StackCount = 1);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void AddItemInstance(ULyraInventoryItemInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void RemoveItemInstance(ULyraInventoryItemInstance* ItemInstance);
	
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	bool RemoveItemDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDef, int32 NumberToRemove);

	UFUNCTION()
	int32 AddItemToFirstAvailableStack(const TSubclassOf<ULyraInventoryItemDefinition>& ItemDef, int32 StackCount) const;
	
	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure=false)
	TArray<ULyraInventoryItemInstance*> GetAllItems() const;

	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure=false)
	TArray<ULyraInventoryItemInstance*> GetAllItemsOfType(ELyraItemType ItemType);

	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure=false)
	int32 GetTotalItemCount(TSubclassOf<ULyraInventoryItemDefinition> ItemDef) const;
	
	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure)
	ULyraInventoryItemInstance* FindFirstItemStackByDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDef) const;

	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure)
	ULyraInventoryItemInstance* FindFirstItemStackByDefinition_NotFull(TSubclassOf<ULyraInventoryItemDefinition> ItemDef) const;
	
	int32 GetTotalItemCountByDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDef) const;
	bool ConsumeItemsByDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDef, int32 NumToConsume);

	//~UObject interface
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;
	//~End of UObject interface
	
	UFUNCTION(BlueprintCallable, Category="SaveGame|Inventory")
	void LoadInventory();

	UFUNCTION(BlueprintCallable, Category="SaveGame|Inventory")
	void SaveInventory();

	UFUNCTION()
	void BroadcastStackChanged(ULyraInventoryItemInstance* ItemInstance, int32 ChangeDelta);


private:
	UPROPERTY(Replicated)
	FLyraInventoryList InventoryList;
};
