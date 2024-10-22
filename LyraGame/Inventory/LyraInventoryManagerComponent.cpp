// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraInventoryManagerComponent.h"

#include "Engine/ActorChannel.h"
#include "Engine/World.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "LyraInventoryItemDefinition.h"
#include "LyraInventoryItemInstance.h"
#include "NativeGameplayTags.h"
#include "Equipment/LyraQuickBarComponent.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraInventoryManagerComponent)

class FLifetimeProperty;
struct FReplicationFlags;

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lyra_Inventory_Message_StackChanged, "Lyra.Inventory.Message.StackChanged");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lyra_Inventory_Item_Count, "Lyra.Inventory.Item.Count"); 
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lyra_Inventory_Item_StackCount, "Lyra.Inventory.Item.StackLimit");

//////////////////////////////////////////////////////////////////////
// FLyraInventoryEntry

FString FLyraInventoryEntry::GetDebugString() const
{
	TSubclassOf<ULyraInventoryItemDefinition> ItemDef;
	if (Instance != nullptr)
	{
		ItemDef = Instance->GetItemDef();
	}

	return FString::Printf(TEXT("%s (%d x %s)"), *GetNameSafe(Instance), StackCount, *GetNameSafe(ItemDef));
}

//////////////////////////////////////////////////////////////////////
// FLyraInventoryList

void FLyraInventoryList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		FLyraInventoryEntry& Stack = Entries[Index];
		BroadcastChangeMessage(Stack, /*OldCount=*/ Stack.StackCount, /*NewCount=*/ 0);
		Stack.LastObservedCount = 0;
	}
}

void FLyraInventoryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FLyraInventoryEntry& Stack = Entries[Index];
		BroadcastChangeMessage(Stack, /*OldCount=*/ 0, /*NewCount=*/ Stack.StackCount);
		Stack.LastObservedCount = Stack.StackCount;
	}
}

void FLyraInventoryList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FLyraInventoryEntry& Stack = Entries[Index];
		check(Stack.LastObservedCount != INDEX_NONE);
		BroadcastChangeMessage(Stack, /*OldCount=*/ Stack.LastObservedCount, /*NewCount=*/ Stack.StackCount);
		Stack.LastObservedCount = Stack.StackCount;
	}
}

void FLyraInventoryList::BroadcastChangeMessage(FLyraInventoryEntry& Entry, int32 OldCount, int32 NewCount)
{
	FLyraInventoryChangeMessage Message;
	Message.InventoryOwner = OwnerComponent;
	Message.Instance = Entry.Instance;
	Message.NewCount = NewCount;
	Message.Delta = NewCount - OldCount;

	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(OwnerComponent->GetWorld());
	MessageSystem.BroadcastMessage(TAG_Lyra_Inventory_Message_StackChanged, Message);
}

ULyraInventoryItemInstance* FLyraInventoryList::AddEntry(TSubclassOf<ULyraInventoryItemDefinition> ItemDef, int32 StackCount)
{
	ULyraInventoryItemInstance* Result = nullptr;

	check(ItemDef != nullptr);
 	check(OwnerComponent);

	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor->HasAuthority());


	FLyraInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Instance = NewObject<ULyraInventoryItemInstance>(OwnerComponent->GetOwner());  //@TODO: Using the actor instead of component as the outer due to UE-127172
	NewEntry.Instance->SetItemDef(ItemDef);
	for (ULyraInventoryItemFragment* Fragment : GetDefault<ULyraInventoryItemDefinition>(ItemDef)->Fragments)
	{
		if (Fragment != nullptr)
		{
			Fragment->OnInstanceCreated(NewEntry.Instance);
		}
	}
	NewEntry.StackCount = StackCount;

	// Add item count as a GameplayTag so it can be retrieved from the ULyraInventoryItemInstance
	NewEntry.Instance->AddStatTagStack(TAG_Lyra_Inventory_Item_Count, StackCount);
	NewEntry.Instance->AddStatTagStack(TAG_Lyra_Inventory_Item_StackCount, GetDefault<ULyraInventoryItemDefinition>(ItemDef)->GetStackLimit());
	
	Result = NewEntry.Instance;

	//const ULyraInventoryItemDefinition* ItemCDO = GetDefault<ULyraInventoryItemDefinition>(ItemDef);
	MarkItemDirty(NewEntry);

	return Result;
}

void FLyraInventoryList::AddEntry(ULyraInventoryItemInstance* Instance)
{
	unimplemented();
}

void FLyraInventoryList::RemoveEntry(ULyraInventoryItemInstance* Instance)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FLyraInventoryEntry& Entry = *EntryIt;
		if (Entry.Instance == Instance)
		{
			EntryIt.RemoveCurrent();
			MarkArrayDirty();
		}
	}
}

TArray<ULyraInventoryItemInstance*> FLyraInventoryList::GetAllItems() const
{
	TArray<ULyraInventoryItemInstance*> Results;
	Results.Reserve(Entries.Num());
	for (const FLyraInventoryEntry& Entry : Entries)
	{
		if (Entry.Instance != nullptr) //@TODO: Would prefer to not deal with this here and hide it further?
		{
			Results.Add(Entry.Instance);
		}
	}
	return Results;
}

TArray<ULyraInventoryItemInstance*> FLyraInventoryList::GetAllItemsOfType(ELyraItemType ItemType) const
{
	TArray<ULyraInventoryItemInstance*> Results;
	for (const FLyraInventoryEntry& Entry : Entries)
	{
		if(Entry.Instance != nullptr && Entry.Instance->ItemDef.GetDefaultObject()->ItemType == ItemType)
		{
			if(Entry.Instance->GetStatTags().GetStackCount(TAG_Lyra_Inventory_Item_Count) > 0)
			{
				Results.Add(Entry.Instance);
			}
		}
	}
	return Results;
}

bool FLyraInventoryList::IsEmpty() const
{
	return Entries.Num() <= 0;
}

//////////////////////////////////////////////////////////////////////
// ULyraInventoryManagerComponent

ULyraInventoryManagerComponent::ULyraInventoryManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InventoryList(this)
{
	SetIsReplicatedByDefault(true);
}

void ULyraInventoryManagerComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, InventoryList);
}

bool ULyraInventoryManagerComponent::CanAddItemDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDef, int32 StackCount)
{
	//@TODO: Add support for stack limit / uniqueness checks / etc...

	for (FLyraInventoryEntry Entry : InventoryList.GetEntries())
	{
		if (Entry.GetInstance()->GetItemDef() == ItemDef)
		{
			return true;
		}
	}
	
	return false;
}

ULyraInventoryItemInstance* ULyraInventoryManagerComponent::AddItemDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDef, int32 StackCount)
{
	ULyraInventoryItemInstance* Result = nullptr;
	if (ItemDef != nullptr)
	{		
		if(CanAddItemDefinition(ItemDef, StackCount)) // Can we just add it onto a stack?
		{
			StackCount = AddItemToFirstAvailableStack(ItemDef, StackCount);
		}
		
		while (StackCount > 0) // Make new stacks until we are done adding all the items.
		{
			int32 ItemsToAdd = FMath::Min(StackCount, ItemDef.GetDefaultObject()->GetStackLimit());
			Result = InventoryList.AddEntry(ItemDef, ItemsToAdd);
			Result->SetInventoryManagerComponent(this);
			BroadcastStackChanged(Result, ItemsToAdd);
			StackCount -= ItemsToAdd;

			// If we pick up a weapon, lets check if we can add it to our quick-slots.
			if(Result->GetItemDef().GetDefaultObject()->ItemType == ELyraItemType::LIT_Weapon)
			{
				if(ULyraQuickBarComponent* QuickBarComponent = Cast<ULyraQuickBarComponent>(this->GetOwner()->GetComponentByClass(ULyraQuickBarComponent::StaticClass())))
				{
					int32 FreeSlotIndex = QuickBarComponent->GetNextFreeItemSlot();
					if(FreeSlotIndex != INDEX_NONE)
					{
						QuickBarComponent->AddItemToSlot(FreeSlotIndex,Result);
						QuickBarComponent->SetActiveSlotIndex(FreeSlotIndex);
					}
				}
			}
		}
		
		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && Result)
		{
			AddReplicatedSubObject(Result);
		}
	}

	SaveInventory();
	
	return Result;
}

void ULyraInventoryManagerComponent::AddItemInstance(ULyraInventoryItemInstance* ItemInstance)
{
	ItemInstance->SetInventoryManagerComponent(this);
	InventoryList.AddEntry(ItemInstance);

	// If we pick up a weapon, lets check if we can add it to our quick-slots.
	if(ItemInstance->GetItemDef().GetDefaultObject()->ItemType == ELyraItemType::LIT_Weapon)
	{
		if(ULyraQuickBarComponent* QuickBarComponent = Cast<ULyraQuickBarComponent>(this->GetOwner()->GetComponentByClass(ULyraQuickBarComponent::StaticClass())))
		{
			if(QuickBarComponent->GetNextFreeItemSlot() != INDEX_NONE)
			{
				QuickBarComponent->AddItemToSlot(QuickBarComponent->GetNextFreeItemSlot(),ItemInstance);
			}
		}
	}
	
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && ItemInstance)
	{
		AddReplicatedSubObject(ItemInstance);
	}

	SaveInventory();
}

void ULyraInventoryManagerComponent::RemoveItemInstance(ULyraInventoryItemInstance* ItemInstance)
{
	FLyraInventoryChangeMessage Message;
	Message.InventoryOwner = this;
	Message.Instance = ItemInstance;
	Message.NewCount = 0;
	Message.Delta = -1;

	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this->GetWorld());
	MessageSystem.BroadcastMessage(TAG_Lyra_Inventory_Message_StackChanged, Message);
	
	InventoryList.RemoveEntry(ItemInstance);
	
	if (ItemInstance && IsUsingRegisteredSubObjectList())
	{
		RemoveReplicatedSubObject(ItemInstance);
	}
}

bool ULyraInventoryManagerComponent::RemoveItemDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDef,
                                                          int32 NumberToRemove)
{
	// Check if we have enough items to remove.
	if(GetTotalItemCount(ItemDef) >= NumberToRemove) 
	{
		while (NumberToRemove > 0)
		{
			ULyraInventoryItemInstance* ItemInstance = FindFirstItemStackByDefinition(ItemDef);
			if(!IsValid(ItemInstance))
			{
				return false; // Safety check 
			}

			const int32 ItemCount = ItemInstance->GetStatTagStackCount(TAG_Lyra_Inventory_Item_Count);
			
			// Determine the number of items to remove form this stack
			int32 ItemsToRemove = FMath::Min(ItemCount, NumberToRemove);

			// Check if the stack is empty
			if(ItemInstance->GetStatTagStackCount(TAG_Lyra_Inventory_Item_Count) <= 1)
			{
				RemoveItemInstance(ItemInstance); // Remove stack from inventory.
			}
			else
			{
				// Remove the items from the stack
				ItemInstance->RemoveStatTagStack(TAG_Lyra_Inventory_Item_Count, ItemsToRemove);
				BroadcastStackChanged(ItemInstance, -NumberToRemove);
			}

			// Update the counter to keep removing if needed.
			NumberToRemove -= ItemsToRemove;
		}

		SaveInventory();
		return true;
	}
	return false;
}

int32 ULyraInventoryManagerComponent::AddItemToFirstAvailableStack(const TSubclassOf<ULyraInventoryItemDefinition>& ItemDef,
                                                                   int32 StackCount) const
{
	while (StackCount > 0)
	{
		if (ULyraInventoryItemInstance* ItemInstance = FindFirstItemStackByDefinition_NotFull(ItemDef))
		{
			const int32 ItemCount = ItemInstance->GetStatTagStackCount(TAG_Lyra_Inventory_Item_Count);
			const int32 MaxStackCount = ItemInstance->GetStatTagStackCount(TAG_Lyra_Inventory_Item_StackCount);
			int32 AvailableSpace = MaxStackCount - ItemCount;
        
			// Determine the number of items to add to this stack
			int32 ItemsToAdd = FMath::Min(StackCount, AvailableSpace);
        
			// Add the items to the stack
			ItemInstance->AddStatTagStack(TAG_Lyra_Inventory_Item_Count, ItemsToAdd);
        
			// Decrease the StackCount by the number of items added
			StackCount -= ItemsToAdd;
		}
		else
		{
			// No more available item stacks to add to, break the loop
			break;
		}
	}

	return StackCount;
}

TArray<ULyraInventoryItemInstance*> ULyraInventoryManagerComponent::GetAllItems() const
{
	return InventoryList.GetAllItems();
}

TArray<ULyraInventoryItemInstance*> ULyraInventoryManagerComponent::GetAllItemsOfType(ELyraItemType ItemType)
{
	return InventoryList.GetAllItemsOfType(ItemType);
}

int32 ULyraInventoryManagerComponent::GetTotalItemCount(TSubclassOf<ULyraInventoryItemDefinition> ItemDef) const
{
	int32 ItemCount = 0;
	for (const FLyraInventoryEntry& Entry : InventoryList.Entries)
	{
		ULyraInventoryItemInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			if (Instance->GetItemDef() == ItemDef)
			{
				ItemCount += Instance->GetStatTagStackCount(TAG_Lyra_Inventory_Item_Count);
			}
		}
	}

	return ItemCount;
}

ULyraInventoryItemInstance* ULyraInventoryManagerComponent::FindFirstItemStackByDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDef) const
{
	for (const FLyraInventoryEntry& Entry : InventoryList.Entries)
	{
		ULyraInventoryItemInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			if (Instance->GetItemDef() == ItemDef)
			{
				return Instance;
			}
		}
	}

	return nullptr;
}

ULyraInventoryItemInstance* ULyraInventoryManagerComponent::FindFirstItemStackByDefinition_NotFull(
	TSubclassOf<ULyraInventoryItemDefinition> ItemDef) const
{
	for (const FLyraInventoryEntry& Entry : InventoryList.Entries)
	{
		ULyraInventoryItemInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			if (Instance->GetItemDef() == ItemDef)
			{
				const int32 ItemCount = Instance->GetStatTagStackCount(TAG_Lyra_Inventory_Item_Count);
				const int32 StackCount = Instance->GetStatTagStackCount(TAG_Lyra_Inventory_Item_StackCount);
				if(ItemCount < StackCount)
				{
					return Instance;
				}
			}
		}
	}

	return nullptr;
}

int32 ULyraInventoryManagerComponent::GetTotalItemCountByDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDef) const
{
	int32 TotalCount = 0;
	for (const FLyraInventoryEntry& Entry : InventoryList.Entries)
	{
		ULyraInventoryItemInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			if (Instance->GetItemDef() == ItemDef)
			{
				++TotalCount;
			}
		}
	}

	return TotalCount;
}

bool ULyraInventoryManagerComponent::ConsumeItemsByDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDef, int32 NumToConsume)
{
	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority())
	{
		return false;
	}

	//@TODO: N squared right now as there's no acceleration structure
	int32 TotalConsumed = 0;
	while (TotalConsumed < NumToConsume)
	{
		if (ULyraInventoryItemInstance* Instance = ULyraInventoryManagerComponent::FindFirstItemStackByDefinition(ItemDef))
		{
			BroadcastStackChanged(Instance, -1);
			InventoryList.RemoveEntry(Instance);
			++TotalConsumed;
		}
		else
		{
			return false;
		}
	}

	SaveInventory();

	return TotalConsumed == NumToConsume;
}

void ULyraInventoryManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// Register existing ULyraInventoryItemInstance
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FLyraInventoryEntry& Entry : InventoryList.Entries)
		{
			ULyraInventoryItemInstance* Instance = Entry.Instance;

			if (IsValid(Instance))
			{
				AddReplicatedSubObject(Instance);
			}
		}
	}
}

void ULyraInventoryManagerComponent::LoadInventory()
{
}

void ULyraInventoryManagerComponent::SaveInventory()
{
}

void ULyraInventoryManagerComponent::BroadcastStackChanged(ULyraInventoryItemInstance* ItemInstance, int32 ChangeDelta)
{
	FLyraInventoryChangeMessage Message;
	Message.InventoryOwner = this;
	Message.Instance = ItemInstance;
	Message.NewCount = ItemInstance->GetStatTagStackCount(TAG_Lyra_Inventory_Item_Count);
	Message.Delta = ChangeDelta;

	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this->GetWorld());
	MessageSystem.BroadcastMessage(TAG_Lyra_Inventory_Message_StackChanged, Message);
}


bool ULyraInventoryManagerComponent::ReplicateSubobjects(UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (FLyraInventoryEntry& Entry : InventoryList.Entries)
	{
		ULyraInventoryItemInstance* Instance = Entry.Instance;

		if (Instance && IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}

//////////////////////////////////////////////////////////////////////
//

// UCLASS(Abstract)
// class ULyraInventoryFilter : public UObject
// {
// public:
// 	virtual bool PassesFilter(ULyraInventoryItemInstance* Instance) const { return true; }
// };

// UCLASS()
// class ULyraInventoryFilter_HasTag : public ULyraInventoryFilter
// {
// public:
// 	virtual bool PassesFilter(ULyraInventoryItemInstance* Instance) const { return true; }
// };


