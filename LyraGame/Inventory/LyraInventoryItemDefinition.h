// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "LyraInventoryItemDefinition.generated.h"

template <typename T> class TSubclassOf;

class ULyraInventoryItemInstance;
struct FFrame;


UENUM(BlueprintType)
enum class ELyraItemType : uint8
{
	LIT_Consumable    UMETA(DisplayName = "Consumable"),
	LIT_Material      UMETA(DisplayName = "Material"),
	LIT_Weapon        UMETA(DisplayName = "Weapon"),
	LIT_Armor         UMETA(DisplayName = "Armor"),
	LIT_Accessory     UMETA(DisplayName = "Accessory"),
	LIT_QuestItem     UMETA(DisplayName = "Quest Item"),
	LIT_Miscellaneous UMETA(DisplayName = "Miscellaneous"),
	LIT_None		  UMETA(DisplayName = "None")
};


//////////////////////////////////////////////////////////////////////

// Represents a fragment of an item definition
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class LYRAGAME_API ULyraInventoryItemFragment : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnInstanceCreated(ULyraInventoryItemInstance* Instance) const {}
};

//////////////////////////////////////////////////////////////////////

/**
 * ULyraInventoryItemDefinition
 */
UCLASS(Blueprintable, Const, Abstract)
class ULyraInventoryItemDefinition : public UObject
{
	GENERATED_BODY()

public:
	ULyraInventoryItemDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display)
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display)
	ELyraItemType ItemType = ELyraItemType::LIT_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display)
	int32 StackLimit;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display, Instanced)
	TArray<TObjectPtr<ULyraInventoryItemFragment>> Fragments;

public:
	const ULyraInventoryItemFragment* FindFragmentByClass(TSubclassOf<ULyraInventoryItemFragment> FragmentClass) const;

	int32 GetStackLimit() const { return StackLimit; }
};

//@TODO: Make into a subsystem instead?
UCLASS()
class ULyraInventoryFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, meta=(DeterminesOutputType=FragmentClass))
	static const ULyraInventoryItemFragment* FindItemDefinitionFragment(TSubclassOf<ULyraInventoryItemDefinition> ItemDef, TSubclassOf<ULyraInventoryItemFragment> FragmentClass);
};
