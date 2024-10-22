// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "LyraInventoryManagerComponent.h"
#include "System/GameplayTagStack.h"
#include "Templates/SubclassOf.h"

#include "LyraInventoryItemInstance.generated.h"

class FLifetimeProperty;

class ULyraInventoryItemDefinition;
class ULyraInventoryItemFragment;
struct FFrame;
struct FGameplayTag;

USTRUCT()
struct FLyraInventoryItemInstanceSaveData
{
	GENERATED_BODY()

	// Class name of the item definition
	UPROPERTY()
	FString ItemDefClassName;

	// The gameplay tags associated with this item instance
	UPROPERTY()
	FGameplayTagStackContainer StatTags;

	// The name of the inventory manager component associated with this item instance
	UPROPERTY()
	FString InventoryManagerComponentName;
};

/**
 * ULyraInventoryItemInstance
 */
UCLASS(BlueprintType)
class ULyraInventoryItemInstance : public UObject
{
	GENERATED_BODY()

public:
	ULyraInventoryItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	//~UObject interface
	virtual bool IsSupportedForNetworking() const override { return true; }
	//~End of UObject interface

	// Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category= Inventory)
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Returns the stack count of the specified tag (or 0 if the tag is not present)
	UFUNCTION(BlueprintCallable, Category=Inventory)
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	// Returns true if there is at least one stack of the specified tag
	UFUNCTION(BlueprintCallable, Category=Inventory)
	bool HasStatTag(FGameplayTag Tag) const;

	UFUNCTION(BlueprintCallable,Category=Inventory)
	TSubclassOf<ULyraInventoryItemDefinition> GetItemDef() const
	{
		return ItemDef;
	}

	void SetItemDef(TSubclassOf<ULyraInventoryItemDefinition> InDef)
	{
		ItemDef = InDef;
	}

	UFUNCTION(BlueprintCallable,Category=Inventory)
	const FGameplayTagStackContainer& GetStatTags() const
	{
		return StatTags;
	}

	void SetStatTags(const FGameplayTagStackContainer& InStatTags)
	{
		StatTags = InStatTags;
	}

	void SetInventoryManagerComponent(ULyraInventoryManagerComponent* InInventoryManagerComponent)
	{
		InventoryManagerComponent = InInventoryManagerComponent;
	}
	
	UFUNCTION(BlueprintCallable, BlueprintPure=false, meta=(DeterminesOutputType=FragmentClass))
	const ULyraInventoryItemFragment* FindFragmentByClass(TSubclassOf<ULyraInventoryItemFragment> FragmentClass) const;

	template <typename ResultClass>
	const ResultClass* FindFragmentByClass() const
	{
		return (ResultClass*)FindFragmentByClass(ResultClass::StaticClass());
	}

	
	void ExtractSaveData(FLyraInventoryItemInstanceSaveData& OutSaveData) const;

private:
#if UE_WITH_IRIS
	/** Register all replication fragments */
	virtual void RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags) override;
#endif // UE_WITH_IRIS

	friend struct FLyraInventoryList;

private:
	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;

	// The item definition
	UPROPERTY(Replicated)
	TSubclassOf<ULyraInventoryItemDefinition> ItemDef;

	UPROPERTY()
	TObjectPtr<ULyraInventoryManagerComponent> InventoryManagerComponent;
};
