#include "Equipment/GBFEquipmentManagerComponent.h"

#include "Equipment/GBFEquipmentDefinition.h"
#include "Equipment/GBFEquipmentInstance.h"
#include "GAS/Components/GBFAbilitySystemComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"

#include <AbilitySystemGlobals.h>
#include <Components/SkeletalMeshComponent.h>
#include <Engine/ActorChannel.h>
#include <Engine/World.h>
#include <GameFramework/Controller.h>
#include <NativeGameplayTags.h>
#include <Net/UnrealNetwork.h>

namespace
{
    UE_DEFINE_GAMEPLAY_TAG_STATIC( TAG_Gameplay_Equipment_Message_Equipped, "Gameplay.Equipment.Message.Equipped" );
    UE_DEFINE_GAMEPLAY_TAG_STATIC( TAG_Gameplay_Equipment_Message_UnEquipped, "Gameplay.Equipment.Message.UnEquipped" );
}

FString FGBFAppliedEquipmentEntry::GetDebugString() const
{
    return FString::Printf( TEXT( "%s of %s" ), *GetNameSafe( Instance ), *GetNameSafe( EquipmentDefinition.Get() ) );
}

FGBFEquipmentList::FGBFEquipmentList() :
    OwnerComponent( nullptr )
{
}

FGBFEquipmentList::FGBFEquipmentList( UActorComponent * owner_component ) :
    OwnerComponent( owner_component )
{
}

void FGBFEquipmentList::PreReplicatedRemove( const TArrayView< int32 > removed_indices, int32 /*final_size*/ )
{
    for ( const auto index : removed_indices )
    {
        if ( const auto & entry = Entries[ index ];
             entry.Instance != nullptr )
        {
            entry.Instance->OnUnequipped();
        }
    }
}

void FGBFEquipmentList::PostReplicatedAdd( const TArrayView< int32 > added_indices, int32 /*final_size*/ )
{
    for ( const auto index : added_indices )
    {
        if ( const auto & entry = Entries[ index ];
             entry.Instance != nullptr )
        {
            entry.Instance->OnEquipped();
        }
    }
}

bool FGBFEquipmentList::NetDeltaSerialize( FNetDeltaSerializeInfo & delta_params )
{
    return FFastArraySerializer::FastArrayDeltaSerialize< FGBFAppliedEquipmentEntry, FGBFEquipmentList >( Entries, delta_params, *this );
}

UGBFEquipmentInstance * FGBFEquipmentList::AddEntry( TSubclassOf< UGBFEquipmentDefinition > equipment_definition )
{
    const auto * equipment_cdo = GetDefault< UGBFEquipmentDefinition >( equipment_definition );

    TSubclassOf< UGBFEquipmentInstance > instance_type = equipment_cdo->InstanceType;
    if ( instance_type == nullptr )
    {
        instance_type = UGBFEquipmentInstance::StaticClass();
    }

    auto equipment_instance = NewObject< UGBFEquipmentInstance >( OwnerComponent->GetOwner(), instance_type );
    equipment_instance->Initialize();

    return AddEntryInternal( equipment_instance, equipment_definition, true );
}

UGBFEquipmentInstance * FGBFEquipmentList::AddEntry( UGBFEquipmentInstance * equipment_instance, TSubclassOf< UGBFEquipmentDefinition > equipment_definition )
{
    return AddEntryInternal( equipment_instance, equipment_definition, false );
}

UGBFEquipmentInstance * FGBFEquipmentList::AddEntryInternal( UGBFEquipmentInstance * equipment_instance, TSubclassOf< UGBFEquipmentDefinition > equipment_definition, bool spawn_equipment_actors )
{
    check( equipment_definition != nullptr );
    check( equipment_instance != nullptr );
    check( OwnerComponent != nullptr );
    check( OwnerComponent->GetOwner()->HasAuthority() );

    const auto * equipment_cdo = GetDefault< UGBFEquipmentDefinition >( equipment_definition );

    auto & new_entry = Entries.AddDefaulted_GetRef();
    new_entry.EquipmentDefinition = equipment_definition;
    new_entry.Instance = equipment_instance; // :TODO: Using the actor instead of component as the outer due to UE-127172

    if ( auto * asc = GetAbilitySystemComponent() )
    {
        for ( const auto & ability_set : equipment_cdo->AbilitySetsToGrant )
        {
            ability_set->GiveToAbilitySystem( asc, /*inout*/ &new_entry.GrantedHandles, new_entry.Instance );
        }
    }
    else
    {
        // :TODO: Warning logging?
    }

    auto & actors_to_spawn = new_entry.EquipmentDefinition->GetDefaultObject< UGBFEquipmentDefinition >()->ActorsToSpawn;

    if ( spawn_equipment_actors )
    {
        new_entry.Instance->SpawnEquipmentActors( actors_to_spawn );
    }
    else
    {
        if ( auto * owning_pawn = new_entry.Instance->GetPawn() )
        {
            if ( auto * attach_target = owning_pawn->FindComponentByClass< USkeletalMeshComponent >() )
            {
                for ( const auto & actor_to_spawn : actors_to_spawn )
                {
                    new_entry.Instance->SetEquipmentActorTransform( Cast< AActor >( new_entry.Instance->GetInstigator() ), actor_to_spawn.ItemSocket, actor_to_spawn.AttachSocket, actor_to_spawn.AttachTransform, attach_target );
                }
            }
        }
    }

    MarkItemDirty( new_entry );

    FGBFEquipmentStateChangedMessage message;
    message.EquipmentOwner = OwnerComponent;
    message.Instance = new_entry.Instance;

    UGameplayMessageSubsystem::Get( OwnerComponent->GetWorld() ).BroadcastMessage( TAG_Gameplay_Equipment_Message_Equipped, message );

    return new_entry.Instance;
}

void FGBFEquipmentList::RemoveEntry( UGBFEquipmentInstance * instance )
{
    for ( auto entry_it = Entries.CreateIterator(); entry_it; ++entry_it )
    {
        if ( auto & entry = *entry_it;
             entry.Instance == instance )
        {
            if ( auto * asc = GetAbilitySystemComponent() )
            {
                entry.GrantedHandles.TakeFromAbilitySystem( asc );
            }

            if ( instance->bDestroyWhenUnEquipped )
            {
                instance->DestroyEquipmentActors();
            }

            entry_it.RemoveCurrent();
            MarkArrayDirty();

            FGBFEquipmentStateChangedMessage message;
            message.EquipmentOwner = OwnerComponent;
            message.Instance = entry.Instance;

            UGameplayMessageSubsystem::Get( OwnerComponent->GetWorld() ).BroadcastMessage( TAG_Gameplay_Equipment_Message_UnEquipped, message );
        }
    }
}

UGBFAbilitySystemComponent * FGBFEquipmentList::GetAbilitySystemComponent() const
{
    check( OwnerComponent != nullptr );
    const auto * owning_actor = OwnerComponent->GetOwner();
    return Cast< UGBFAbilitySystemComponent >( UAbilitySystemGlobals::GetAbilitySystemComponentFromActor( owning_actor ) );
}

UGBFEquipmentManagerComponent::UGBFEquipmentManagerComponent( const FObjectInitializer & object_initializer ) :
    Super( object_initializer ),
    EquipmentList( this )
{
    SetIsReplicatedByDefault( true );
    bWantsInitializeComponent = true;
}

void UGBFEquipmentManagerComponent::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
    Super::GetLifetimeReplicatedProps( OutLifetimeProps );

    DOREPLIFETIME( ThisClass, EquipmentList );
}

UGBFEquipmentInstance * UGBFEquipmentManagerComponent::EquipItem( const TSubclassOf< UGBFEquipmentDefinition > equipment_definition )
{
    UGBFEquipmentInstance * result = nullptr;
    if ( equipment_definition != nullptr )
    {
        result = EquipmentList.AddEntry( equipment_definition );
        if ( result != nullptr )
        {
            result->OnEquipped();

            if ( IsUsingRegisteredSubObjectList() && IsReadyForReplication() )
            {
                AddReplicatedSubObject( result );
            }
        }
    }
    return result;
}

void UGBFEquipmentManagerComponent::EquipItemWithInstance( UGBFEquipmentInstance * equipment_instance, TSubclassOf< UGBFEquipmentDefinition > equipment_definition )
{
    if ( equipment_instance != nullptr && equipment_definition != nullptr )
    {
        // :NOTE: Set the character who pick the item up as owner what is originally made at the actor spawning
        equipment_instance->Rename( nullptr, GetOwner() );
        EquipmentList.AddEntry( equipment_instance, equipment_definition );
        equipment_instance->OnEquipped();

        if ( IsUsingRegisteredSubObjectList() && IsReadyForReplication() )
        {
            AddReplicatedSubObject( equipment_instance );
        }
    }
}

void UGBFEquipmentManagerComponent::UnequipItem( UGBFEquipmentInstance * item_instance )
{
    if ( item_instance != nullptr )
    {
        if ( IsUsingRegisteredSubObjectList() )
        {
            RemoveReplicatedSubObject( item_instance );
        }

        item_instance->OnUnequipped();
        EquipmentList.RemoveEntry( item_instance );
    }
}

bool UGBFEquipmentManagerComponent::ReplicateSubobjects( UActorChannel * channel, FOutBunch * bunch, FReplicationFlags * rep_flags )
{
    bool wrote_something = Super::ReplicateSubobjects( channel, bunch, rep_flags );

    for ( auto & entry : EquipmentList.Entries )
    {
        if ( UGBFEquipmentInstance * instance = entry.Instance;
             IsValid( instance ) )
        {
            wrote_something |= channel->ReplicateSubobject( instance, *bunch, *rep_flags );
        }
    }

    return wrote_something;
}

void UGBFEquipmentManagerComponent::UninitializeComponent()
{
    TArray< UGBFEquipmentInstance * > all_equipment_instances;

    // gathering all instances before removal to avoid side effects affecting the equipment list iterator
    for ( const auto & entry : EquipmentList.Entries )
    {
        all_equipment_instances.Add( entry.Instance );
    }

    for ( auto * equip_instance : all_equipment_instances )
    {
        UnequipItem( equip_instance );
    }

    Super::UninitializeComponent();
}

void UGBFEquipmentManagerComponent::ReadyForReplication()
{
    Super::ReadyForReplication();

    // Register existing LyraEquipmentInstances
    if ( IsUsingRegisteredSubObjectList() )
    {
        for ( const auto & entry : EquipmentList.Entries )
        {
            if ( UGBFEquipmentInstance * instance = entry.Instance;
                 IsValid( instance ) )
            {
                AddReplicatedSubObject( instance );
            }
        }
    }
}

UGBFEquipmentInstance * UGBFEquipmentManagerComponent::GetFirstInstanceOfType( const TSubclassOf< UGBFEquipmentInstance > instance_type ) const
{
    for ( auto & entry : EquipmentList.Entries )
    {
        if ( UGBFEquipmentInstance * instance = entry.Instance )
        {
            if ( instance->IsA( instance_type ) )
            {
                return instance;
            }
        }
    }

    return nullptr;
}

TArray< UGBFEquipmentInstance * > UGBFEquipmentManagerComponent::GetEquipmentInstancesOfType( const TSubclassOf< UGBFEquipmentInstance > instance_type ) const
{
    TArray< UGBFEquipmentInstance * > results;
    for ( const auto & entry : EquipmentList.Entries )
    {
        if ( UGBFEquipmentInstance * instance = entry.Instance )
        {
            if ( instance->IsA( instance_type ) )
            {
                results.Add( instance );
            }
        }
    }
    return results;
}

UGBFEquipmentManagerComponent * UGBFEquipmentManagerComponent::FindEquipmentManagerComponent( AActor * actor )
{
    if ( const auto * owner_controller = Cast< AController >( actor ) )
    {
        if ( const auto * pawn = owner_controller->GetPawn() )
        {
            return pawn->FindComponentByClass< UGBFEquipmentManagerComponent >();
        }
    }
    return nullptr;
}
