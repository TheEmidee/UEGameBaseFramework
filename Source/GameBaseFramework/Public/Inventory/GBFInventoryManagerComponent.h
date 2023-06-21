#pragma once

#include <CoreMinimal.h>
#include <ModularPawnComponent.h>
#include <Net/Serialization/FastArraySerializer.h>

#include "GBFInventoryManagerComponent.generated.h"

class UGBFInventoryItemInstance;
struct FGBFInventoryList;

/** A message when an item is added to the inventory */
USTRUCT( BlueprintType )
struct FGBFInventoryChangeMessage
{
    GENERATED_BODY()

    UPROPERTY( BlueprintReadOnly, Category = Inventory )
    TObjectPtr< UActorComponent > InventoryOwner = nullptr;

    UPROPERTY( BlueprintReadOnly, Category = Inventory )
    TObjectPtr< UGBFInventoryItemInstance > Instance = nullptr;

    UPROPERTY( BlueprintReadOnly, Category = Inventory )
    int32 NewCount = 0;

    UPROPERTY( BlueprintReadOnly, Category = Inventory )
    int32 Delta = 0;
};

/** A single entry in an inventory */
USTRUCT( BlueprintType )
struct FGBFInventoryEntry : public FFastArraySerializerItem
{
    GENERATED_BODY()

    FGBFInventoryEntry() = default;
    FString GetDebugString() const;

private:
    friend FGBFInventoryList;
    friend UGBFInventoryManagerComponent;

    UPROPERTY()
    TObjectPtr< UGBFInventoryItemInstance > Instance = nullptr;

    UPROPERTY()
    int32 StackCount = 0;

    UPROPERTY( NotReplicated )
    int32 LastObservedCount = INDEX_NONE;
};

/** List of inventory items */
USTRUCT( BlueprintType )
struct FGBFInventoryList : public FFastArraySerializer
{
    GENERATED_BODY()

    FGBFInventoryList();
    explicit FGBFInventoryList( UActorComponent * owner_component );

    TArray< UGBFInventoryItemInstance * > GetAllItems() const;

    void PreReplicatedRemove( const TArrayView< int32 > removed_indices, int32 final_size );
    void PostReplicatedAdd( const TArrayView< int32 > added_indices, int32 final_size );
    void PostReplicatedChange( const TArrayView< int32 > changed_indices, int32 final_size );
    bool NetDeltaSerialize( FNetDeltaSerializeInfo & delta_parameters );
    UGBFInventoryItemInstance * AddEntry( TSubclassOf< UGBFInventoryItemDefinition > item_class, int32 stack_count );
    void AddEntry( UGBFInventoryItemInstance * instance );
    void RemoveEntry( UGBFInventoryItemInstance * instance );

private:
    void BroadcastChangeMessage( const FGBFInventoryEntry & entry, int32 old_count, int32 new_count );
    friend UGBFInventoryManagerComponent;

    // Replicated list of items
    UPROPERTY()
    TArray< FGBFInventoryEntry > Entries;

    UPROPERTY( NotReplicated )
    TObjectPtr< UActorComponent > OwnerComponent;
};

template <>
struct TStructOpsTypeTraits< FGBFInventoryList > : TStructOpsTypeTraitsBase2< FGBFInventoryList >
{
    enum
    {
        WithNetDeltaSerializer = true
    };
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFInventoryManagerComponent final : public UModularPawnComponent
{
    GENERATED_BODY()

public:
    explicit UGBFInventoryManagerComponent( const FObjectInitializer & object_initializer = FObjectInitializer::Get() );

    UFUNCTION( BlueprintPure, BlueprintAuthorityOnly, Category = Inventory )
    bool CanAddItemDefinition( TSubclassOf< UGBFInventoryItemDefinition > item_def, int32 stack_count = 1 ) const;

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory )
    UGBFInventoryItemInstance * AddItemDefinition( TSubclassOf< UGBFInventoryItemDefinition > item_definition, int32 stack_count = 1 );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory )
    void AddItemInstance( UGBFInventoryItemInstance * item_instance );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = Inventory )
    void RemoveItemInstance( UGBFInventoryItemInstance * item_instance );

    UFUNCTION( BlueprintCallable, Category = Inventory, BlueprintPure = false )
    TArray< UGBFInventoryItemInstance * > GetAllItems() const;

    UFUNCTION( BlueprintPure, Category = Inventory )
    UGBFInventoryItemInstance * FindFirstItemStackByDefinition( TSubclassOf< UGBFInventoryItemDefinition > item_definition ) const;

    int32 GetTotalItemCountByDefinition( TSubclassOf< UGBFInventoryItemDefinition > item_definition ) const;
    bool ConsumeItemsByDefinition( TSubclassOf< UGBFInventoryItemDefinition > item_definition, int32 num_to_consume );

    bool ReplicateSubobjects( class UActorChannel * channel, class FOutBunch * bunch, FReplicationFlags * rep_flags ) override;
    void ReadyForReplication() override;

private:
    UPROPERTY( Replicated )
    FGBFInventoryList InventoryList;
};
