#include "Equipment/GBFEquipmentTagBarComponent.h"

#include "Equipment/GBFEquipmentDefinition.h"
#include "Equipment/GBFEquipmentInstance.h"
#include "Equipment/GBFEquipmentManagerComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Inventory/GBFInventoryItemDefinition.h"
#include "Inventory/GBFInventoryItemFragment_EquippableItem.h"
#include "Inventory/GBFInventoryItemInstance.h"

#include <Engine/ActorChannel.h>
#include <Engine/World.h>
#include <NativeGameplayTags.h>
#include <Net/UnrealNetwork.h>

UE_DEFINE_GAMEPLAY_TAG_STATIC( TAG_GBF_EquipmentTagBar_Message_ItemChanged, "Gameplay.EquipmentTagBar.Message.ItemChanged" );

FString FGBFEquipmentTagBarInventoryItem::GetDebugString() const
{
    return FString::Printf( TEXT( "%s of type %s" ), *GetNameSafe( InventoryItemInstance ), *TypeTag.ToString() );
}

FGBFTagBarEquipmentList::FGBFTagBarEquipmentList() :
    OwnerComponent( nullptr )
{
}

FGBFTagBarEquipmentList::FGBFTagBarEquipmentList( UActorComponent * owner_component ) :
    OwnerComponent( owner_component )
{
}

void FGBFTagBarEquipmentList::PreReplicatedRemove( const TArrayView< int32 > removed_indices, int32 final_size )
{
    for ( const auto index : removed_indices )
    {
        if ( const auto & entry = Entries[ index ];
             entry.InventoryItemInstance != nullptr )
        {
            BroadcastMessage( entry );
        }
    }
}

void FGBFTagBarEquipmentList::PostReplicatedAdd( const TArrayView< int32 > added_indices, int32 final_size )
{
    for ( const auto index : added_indices )
    {
        if ( const auto & entry = Entries[ index ];
             entry.InventoryItemInstance != nullptr )
        {
            BroadcastMessage( entry );
        }
    }
}

bool FGBFTagBarEquipmentList::NetDeltaSerialize( FNetDeltaSerializeInfo & delta_params )
{
    return FFastArraySerializer::FastArrayDeltaSerialize< FGBFEquipmentTagBarInventoryItem, FGBFTagBarEquipmentList >( Entries, delta_params, *this );
}

UGBFEquipmentInstance * FGBFTagBarEquipmentList::AddEntry( UGBFInventoryItemInstance * item )
{
    check( item != nullptr );
    check( OwnerComponent != nullptr );
    check( OwnerComponent->GetOwner()->HasAuthority() );

    const auto item_type = item->GetItemDefinition()->GetDefaultObject< UGBFInventoryItemDefinition >()->ItemTypeTag;
    if ( !item_type.IsValid() )
    {
        return nullptr;
    }

    if ( auto * equipment_manager = UGBFEquipmentManagerComponent::FindEquipmentManagerComponent( OwnerComponent->GetOwner() ) )
    {
        if ( const UGBFInventoryItemFragment_EquippableItem * equip_info = item->FindFragmentByClass< UGBFInventoryItemFragment_EquippableItem >() )
        {
            if ( const auto equipment_definition = equip_info->EquipmentDefinition;
                 equipment_definition != nullptr )
            {
                if ( auto * equipped_item_instance = equipment_manager->EquipItem( equipment_definition ) )
                {
                    equipped_item_instance->SetInstigator( item );

                    auto & new_entry = Entries.AddDefaulted_GetRef();
                    new_entry.TypeTag = item_type;
                    new_entry.InventoryItemInstance = item;
                    new_entry.EquipmentInstance = equipped_item_instance;

                    MarkItemDirty( new_entry );

                    BroadcastMessage( new_entry );

                    return equipped_item_instance;
                }
            }
        }
    }

    return nullptr;
}

UGBFEquipmentInstance * FGBFTagBarEquipmentList::RemoveEntry( FGameplayTag type )
{
    if ( auto * equipment_manager = UGBFEquipmentManagerComponent::FindEquipmentManagerComponent( OwnerComponent->GetOwner() ) )
    {
        for ( auto entry_it = Entries.CreateIterator(); entry_it; ++entry_it )
        {
            if ( auto & entry = *entry_it;
                 entry.TypeTag == type )
            {
                equipment_manager->UnequipItem( entry.EquipmentInstance );

                entry_it.RemoveCurrent();
                MarkArrayDirty();

                BroadcastMessage( entry );

                return entry.EquipmentInstance;
            }
        }
    }

    return nullptr;
}

void FGBFTagBarEquipmentList::BroadcastMessage( const FGBFEquipmentTagBarInventoryItem & item )
{
    FGBFEquipmentTagBarEquipmentChangedMessage message;
    message.Owner = OwnerComponent->GetOwner();
    message.TypeTag = item.TypeTag;
    message.InventoryItemInstance = item.InventoryItemInstance;

    UGameplayMessageSubsystem::Get( OwnerComponent->GetWorld() ).BroadcastMessage( TAG_GBF_EquipmentTagBar_Message_ItemChanged, message );
}

UGBFEquipmentTagBarComponent::UGBFEquipmentTagBarComponent( const FObjectInitializer & object_initializer ) :
    Super( object_initializer ),
    EquipmentList( this )
{
    SetIsReplicatedByDefault( true );
    bWantsInitializeComponent = true;
}

void UGBFEquipmentTagBarComponent::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
    Super::GetLifetimeReplicatedProps( OutLifetimeProps );

    DOREPLIFETIME( ThisClass, EquipmentList );
}

UGBFEquipmentInstance * UGBFEquipmentTagBarComponent::EquipItem( UGBFInventoryItemInstance * item )
{
    UnEquipItem( item );

    UGBFEquipmentInstance * result = nullptr;
    if ( item != nullptr )
    {
        result = EquipmentList.AddEntry( item );

        if ( result != nullptr )
        {
            if ( IsUsingRegisteredSubObjectList() && IsReadyForReplication() )
            {
                AddReplicatedSubObject( result );
            }
        }
    }

    return result;
}

void UGBFEquipmentTagBarComponent::UnEquipItem( UGBFInventoryItemInstance * item )
{
    const auto item_type = item->GetItemDefinition()->GetDefaultObject< UGBFInventoryItemDefinition >()->ItemTypeTag;

    UnEquipItemByType( item_type );
}

void UGBFEquipmentTagBarComponent::UnEquipItemByType( FGameplayTag type )
{
    if ( !type.IsValid() )
    {
        return;
    }

    if ( auto * unequipped_item = EquipmentList.RemoveEntry( type ) )
    {
        if ( IsUsingRegisteredSubObjectList() )
        {
            RemoveReplicatedSubObject( unequipped_item );
        }
    }
}

bool UGBFEquipmentTagBarComponent::ReplicateSubobjects( UActorChannel * channel, FOutBunch * bunch, FReplicationFlags * rep_flags )
{
    bool wrote_something = Super::ReplicateSubobjects( channel, bunch, rep_flags );

    for ( auto & entry : EquipmentList.Entries )
    {
        if ( IsValid( entry.InventoryItemInstance ) && IsValid( entry.EquipmentInstance ) )
        {
            wrote_something |= channel->ReplicateSubobject( entry.InventoryItemInstance, *bunch, *rep_flags );
            wrote_something |= channel->ReplicateSubobject( entry.EquipmentInstance, *bunch, *rep_flags );
        }
    }

    return wrote_something;
}

void UGBFEquipmentTagBarComponent::UninitializeComponent()
{
    TArray< FGameplayTag > all_equipment_types;

    // gathering all instances before removal to avoid side effects affecting the equipment list iterator
    for ( const auto & entry : EquipmentList.Entries )
    {
        all_equipment_types.Add( entry.TypeTag );
    }

    for ( auto tag : all_equipment_types )
    {
        UnEquipItemByType( tag );
    }

    Super::UninitializeComponent();
}

void UGBFEquipmentTagBarComponent::ReadyForReplication()
{
    Super::ReadyForReplication();

    if ( IsUsingRegisteredSubObjectList() )
    {
        for ( const auto & entry : EquipmentList.Entries )
        {
            if ( IsValid( entry.InventoryItemInstance ) && IsValid( entry.EquipmentInstance ) )
            {
                AddReplicatedSubObject( entry.InventoryItemInstance );
                AddReplicatedSubObject( entry.EquipmentInstance );
            }
        }
    }
}