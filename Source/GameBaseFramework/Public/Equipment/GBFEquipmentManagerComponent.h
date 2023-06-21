#pragma once

#include "Abilities/GASExtAbilitySet.h"

#include <CoreMinimal.h>
#include <ModularPawnComponent.h>
#include <Net/Serialization/FastArraySerializer.h>

#include "GBFEquipmentManagerComponent.generated.h"

class UGBFEquipmentDefinition;
class UGBFEquipmentInstance;
struct FGBFEquipmentList;

/** A single piece of applied equipment */
USTRUCT( BlueprintType )
struct FGBFAppliedEquipmentEntry : public FFastArraySerializerItem
{
    GENERATED_BODY()

    FGBFAppliedEquipmentEntry() = default;

    FString GetDebugString() const;

private:
    friend FGBFEquipmentList;
    friend UGBFEquipmentManagerComponent;

    // The equipment class that got equipped
    UPROPERTY()
    TSubclassOf< UGBFEquipmentDefinition > EquipmentDefinition;

    UPROPERTY()
    TObjectPtr< UGBFEquipmentInstance > Instance = nullptr;

    // Authority-only list of granted handles
    UPROPERTY( NotReplicated )
    FGASExtAbilitySet_GrantedHandles GrantedHandles;
};

/** List of applied equipment */
USTRUCT( BlueprintType )
struct FGBFEquipmentList : public FFastArraySerializer
{
    GENERATED_BODY()

    FGBFEquipmentList();
    explicit FGBFEquipmentList( UActorComponent * owner_component );

    void PreReplicatedRemove( const TArrayView< int32 > removed_indices, int32 final_size );
    void PostReplicatedAdd( const TArrayView< int32 > added_indices, int32 final_size );
    bool NetDeltaSerialize( FNetDeltaSerializeInfo & delta_params );
    UGBFEquipmentInstance * AddEntry( TSubclassOf< UGBFEquipmentDefinition > equipment_definition );
    void RemoveEntry( UGBFEquipmentInstance * instance );

private:
    UGASExtAbilitySystemComponent * GetAbilitySystemComponent() const;

    friend UGBFEquipmentManagerComponent;

    // Replicated list of equipment entries
    UPROPERTY()
    TArray< FGBFAppliedEquipmentEntry > Entries;

    UPROPERTY( NotReplicated )
    TObjectPtr< UActorComponent > OwnerComponent;
};

template <>
struct TStructOpsTypeTraits< FGBFEquipmentList > : public TStructOpsTypeTraitsBase2< FGBFEquipmentList >
{
    enum
    {
        WithNetDeltaSerializer = true
    };
};

UCLASS( BlueprintType, Const )
class GAMEBASEFRAMEWORK_API UGBFEquipmentManagerComponent final : public UModularPawnComponent
{
    GENERATED_BODY()

public:
    explicit UGBFEquipmentManagerComponent( const FObjectInitializer & object_initializer = FObjectInitializer::Get() );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly )
    UGBFEquipmentInstance * EquipItem( TSubclassOf< UGBFEquipmentDefinition > equipment_definition );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly )
    void UnequipItem( UGBFEquipmentInstance * item_instance );

    bool ReplicateSubobjects( class UActorChannel * channel, class FOutBunch * bunch, FReplicationFlags * rep_flags ) override;
    void UninitializeComponent() override;
    void ReadyForReplication() override;

    /** Returns the first equipped instance of a given type, or nullptr if none are found */
    UFUNCTION( BlueprintCallable, BlueprintPure )
    UGBFEquipmentInstance * GetFirstInstanceOfType( TSubclassOf< UGBFEquipmentInstance > instance_type ) const;

    /** Returns all equipped instances of a given type, or an empty array if none are found */
    UFUNCTION( BlueprintCallable, BlueprintPure )
    TArray< UGBFEquipmentInstance * > GetEquipmentInstancesOfType( TSubclassOf< UGBFEquipmentInstance > instance_type ) const;

    template < typename T >
    T * GetFirstInstanceOfType()
    {
        return static_cast< T * >( GetFirstInstanceOfType( T::StaticClass() ) );
    }

private:
    UPROPERTY( Replicated )
    FGBFEquipmentList EquipmentList;
};
