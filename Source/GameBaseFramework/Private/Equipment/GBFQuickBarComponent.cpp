#include "Equipment/GBFQuickBarComponent.h"

#include "Equipment/GBFEquipmentDefinition.h"
#include "Equipment/GBFEquipmentInstance.h"
#include "Equipment/GBFEquipmentManagerComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Inventory/GBFInventoryItemFragment_EquippableItem.h"
#include "Inventory/GBFInventoryItemInstance.h"

#include <NativeGameplayTags.h>
#include <Net/UnrealNetwork.h>

UE_DEFINE_GAMEPLAY_TAG_STATIC( TAG_GBF_QuickBar_Message_SlotsChanged, "Gameplay.QuickBar.Message.SlotsChanged" );
UE_DEFINE_GAMEPLAY_TAG_STATIC( TAG_GBF_QuickBar_Message_ActiveIndexChanged, "Gameplay.QuickBar.Message.ActiveIndexChanged" );

UGBFQuickBarComponent::UGBFQuickBarComponent( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    SetIsReplicatedByDefault( true );
}

void UGBFQuickBarComponent::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
    Super::GetLifetimeReplicatedProps( OutLifetimeProps );

    DOREPLIFETIME( ThisClass, Slots );
    DOREPLIFETIME( ThisClass, ActiveSlotIndex );
}

void UGBFQuickBarComponent::CycleActiveSlotForward()
{
    if ( Slots.Num() < 2 )
    {
        return;
    }

    const auto old_index = ActiveSlotIndex < 0 ? Slots.Num() - 1 : ActiveSlotIndex;
    auto new_index = ActiveSlotIndex;
    do
    {
        new_index = ( new_index + 1 ) % Slots.Num();
        if ( Slots[ new_index ] != nullptr )
        {
            SetActiveSlotIndex( new_index );
            return;
        }
    } while ( new_index != old_index );
}

void UGBFQuickBarComponent::CycleActiveSlotBackward()
{
    if ( Slots.Num() < 2 )
    {
        return;
    }

    const auto old_index = ActiveSlotIndex < 0 ? Slots.Num() - 1 : ActiveSlotIndex;
    auto new_index = ActiveSlotIndex;
    do
    {
        new_index = ( new_index - 1 + Slots.Num() ) % Slots.Num();
        if ( Slots[ new_index ] != nullptr )
        {
            SetActiveSlotIndex( new_index );
            return;
        }
    } while ( new_index != old_index );
}

UGBFInventoryItemInstance * UGBFQuickBarComponent::GetActiveSlotItem() const
{
    return Slots.IsValidIndex( ActiveSlotIndex ) ? Slots[ ActiveSlotIndex ] : nullptr;
}

int32 UGBFQuickBarComponent::GetNextFreeItemSlot() const
{
    auto slot_index = 0;
    for ( auto item_ptr : Slots )
    {
        if ( item_ptr == nullptr )
        {
            return slot_index;
        }
        ++slot_index;
    }

    return INDEX_NONE;
}

void UGBFQuickBarComponent::AddItemToSlot( const int32 slot_index, UGBFInventoryItemInstance * item )
{
    if ( Slots.IsValidIndex( slot_index ) && item != nullptr )
    {
        if ( Slots[ slot_index ] == nullptr )
        {
            Slots[ slot_index ] = item;
            OnRep_Slots();
        }
    }
}

UGBFInventoryItemInstance * UGBFQuickBarComponent::RemoveItemFromSlot( const int32 slot_index )
{
    UGBFInventoryItemInstance * result = nullptr;

    if ( ActiveSlotIndex == slot_index )
    {
        UnequipItemInSlot();
        ActiveSlotIndex = -1;
    }

    if ( Slots.IsValidIndex( slot_index ) )
    {
        result = Slots[ slot_index ];

        if ( result != nullptr )
        {
            Slots[ slot_index ] = nullptr;
            OnRep_Slots();
        }
    }

    return result;
}

void UGBFQuickBarComponent::BeginPlay()
{
    if ( Slots.Num() < NumSlots )
    {
        Slots.AddDefaulted( NumSlots - Slots.Num() );
    }

    Super::BeginPlay();
}

void UGBFQuickBarComponent::UnequipItemInSlot()
{
    if ( auto * equipment_manager = UGBFEquipmentManagerComponent::FindEquipmentManagerComponent( GetOwner() ) )
    {
        if ( EquippedItem != nullptr )
        {
            equipment_manager->UnequipItem( EquippedItem );
            EquippedItem = nullptr;
        }
    }
}

void UGBFQuickBarComponent::EquipItemInSlot()
{
    check( Slots.IsValidIndex( ActiveSlotIndex ) );
    check( EquippedItem == nullptr );

    if ( UGBFInventoryItemInstance * slot_item = Slots[ ActiveSlotIndex ] )
    {
        if ( const UGBFInventoryItemFragment_EquippableItem * equip_info = slot_item->FindFragmentByClass< UGBFInventoryItemFragment_EquippableItem >() )
        {
            if ( const auto equipment_definition = equip_info->EquipmentDefinition;
                 equipment_definition != nullptr )
            {
                if ( auto * equipment_manager = UGBFEquipmentManagerComponent::FindEquipmentManagerComponent( GetOwner() ) )
                {
                    EquippedItem = equipment_manager->EquipItem( equipment_definition );
                    if ( EquippedItem != nullptr )
                    {
                        EquippedItem->SetInstigator( slot_item );
                    }
                }
            }
        }
    }
}

void UGBFQuickBarComponent::OnRep_Slots()
{
    FGBFQuickBarSlotsChangedMessage Message;
    Message.Owner = GetOwner();
    Message.Slots = Slots;

    UGameplayMessageSubsystem::Get( this ).BroadcastMessage( TAG_GBF_QuickBar_Message_SlotsChanged, Message );
}

void UGBFQuickBarComponent::OnRep_ActiveSlotIndex()
{
    FGBFQuickBarActiveIndexChangedMessage Message;
    Message.Owner = GetOwner();
    Message.ActiveIndex = ActiveSlotIndex;

    UGameplayMessageSubsystem::Get( this ).BroadcastMessage( TAG_GBF_QuickBar_Message_ActiveIndexChanged, Message );
}

void UGBFQuickBarComponent::SetActiveSlotIndex_Implementation( const int32 new_index )
{
    if ( Slots.IsValidIndex( new_index ) && ActiveSlotIndex != new_index )
    {
        UnequipItemInSlot();

        ActiveSlotIndex = new_index;

        EquipItemInSlot();

        OnRep_ActiveSlotIndex();
    }
}
