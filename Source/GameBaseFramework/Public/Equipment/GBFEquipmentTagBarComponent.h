#pragma once

#include "Net/Serialization/FastArraySerializer.h"

#include <Components/ControllerComponent.h>
#include <CoreMinimal.h>

#include "GBFEquipmentTagBarComponent.generated.h"

class UGBFInventoryItemInstance;
class UGBFEquipmentInstance;

/** A single piece of applied equipment */
USTRUCT( BlueprintType )
struct FGBFEquipmentTagBarInventoryItem : public FFastArraySerializerItem
{
    GENERATED_BODY()

    FGBFEquipmentTagBarInventoryItem() = default;

    FString GetDebugString() const;

private:
    friend FGBFTagBarEquipmentList;
    friend UGBFEquipmentTagBarComponent;

    UPROPERTY()
    FGameplayTag TypeTag;

    UPROPERTY()
    TObjectPtr< UGBFInventoryItemInstance > InventoryItemInstance;

    UPROPERTY()
    TObjectPtr< UGBFEquipmentInstance > EquipmentInstance;
};

/** List of applied equipment */
USTRUCT( BlueprintType )
struct FGBFTagBarEquipmentList : public FFastArraySerializer
{
    GENERATED_BODY()

    FGBFTagBarEquipmentList();
    explicit FGBFTagBarEquipmentList( UActorComponent * owner_component );

    void PreReplicatedRemove( const TArrayView< int32 > removed_indices, int32 final_size );
    void PostReplicatedAdd( const TArrayView< int32 > added_indices, int32 final_size );
    bool NetDeltaSerialize( FNetDeltaSerializeInfo & delta_params );
    UGBFEquipmentInstance * AddEntry( UGBFInventoryItemInstance * item );
    UGBFEquipmentInstance * RemoveEntry( FGameplayTag type );

private:
    friend UGBFEquipmentTagBarComponent;

    void BroadcastMessage( const FGBFEquipmentTagBarInventoryItem & item );

    // Replicated list of equipment entries
    UPROPERTY()
    TArray< FGBFEquipmentTagBarInventoryItem > Entries;

    UPROPERTY( NotReplicated )
    TObjectPtr< UActorComponent > OwnerComponent;
};

template <>
struct TStructOpsTypeTraits< FGBFTagBarEquipmentList > : public TStructOpsTypeTraitsBase2< FGBFTagBarEquipmentList >
{
    enum
    {
        WithNetDeltaSerializer = true
    };
};

USTRUCT( BlueprintType )
struct FGBFEquipmentTagBarEquipmentChangedMessage
{
    GENERATED_BODY()

    UPROPERTY( BlueprintReadOnly, Category = Inventory )
    TObjectPtr< AActor > Owner = nullptr;

    UPROPERTY( BlueprintReadOnly, Category = Inventory )
    FGameplayTag TypeTag;

    UPROPERTY( BlueprintReadOnly, Category = Inventory )
    TObjectPtr< UGBFInventoryItemInstance > InventoryItemInstance;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFEquipmentTagBarComponent final : public UControllerComponent
{
    GENERATED_BODY()

public:
    explicit UGBFEquipmentTagBarComponent( const FObjectInitializer & object_initializer = FObjectInitializer::Get() );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly )
    UGBFEquipmentInstance * EquipItem( UGBFInventoryItemInstance * item );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly )
    void UnEquipItem( UGBFInventoryItemInstance * item );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly )
    void UnEquipItemByType( FGameplayTag type );

    bool ReplicateSubobjects( class UActorChannel * channel, class FOutBunch * bunch, FReplicationFlags * rep_flags ) override;
    void UninitializeComponent() override;
    void ReadyForReplication() override;

private:
    UPROPERTY( Replicated )
    FGBFTagBarEquipmentList EquipmentList;
};
