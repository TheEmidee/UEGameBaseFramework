#include "Inventory/GBFInventoryManagerComponent.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Inventory/GBFInventoryItemDefinition.h"
#include "Inventory/GBFInventoryItemFragment.h"
#include "Inventory/GBFInventoryItemInstance.h"

#include <Engine/ActorChannel.h>
#include <GameFramework/Actor.h>
#include <NativeGameplayTags.h>
#include <Net/UnrealNetwork.h>

namespace
{
    UE_DEFINE_GAMEPLAY_TAG_STATIC( TAG_Gameplay_Inventory_Message_StackChanged, "Gameplay.Inventory.Message.StackChanged" );
}

FString FGBFInventoryEntry::GetDebugString() const
{
    TSubclassOf< UGBFInventoryItemDefinition > item_definition_class;
    if ( Instance != nullptr )
    {
        item_definition_class = Instance->GetItemDefinition();
    }

    return FString::Printf( TEXT( "%s (%d x %s)" ), *GetNameSafe( Instance ), StackCount, *GetNameSafe( item_definition_class ) );
}

FGBFInventoryList::FGBFInventoryList() :
    OwnerComponent( nullptr )
{
}

FGBFInventoryList::FGBFInventoryList( UActorComponent * owner_component ) :
    OwnerComponent( owner_component )
{
}

TArray< UGBFInventoryItemInstance * > FGBFInventoryList::GetAllItems() const
{
    TArray< UGBFInventoryItemInstance * > results;
    results.Reserve( Entries.Num() );

    for ( const auto & entry : Entries )
    {
        if ( entry.Instance != nullptr ) //@TODO: Would prefer to not deal with this here and hide it further?
        {
            results.Add( entry.Instance );
        }
    }
    return results;
}

void FGBFInventoryList::PreReplicatedRemove( const TArrayView< int32 > removed_indices, int32 /*final_size*/ )
{
    for ( const auto index : removed_indices )
    {
        auto & entry = Entries[ index ];
        BroadcastChangeMessage( entry, /*OldCount=*/entry.StackCount, /*NewCount=*/0 );
        entry.LastObservedCount = 0;
    }
}

void FGBFInventoryList::PostReplicatedAdd( const TArrayView< int32 > added_indices, int32 /*final_size*/ )
{
    for ( const auto index : added_indices )
    {
        auto & entry = Entries[ index ];
        BroadcastChangeMessage( entry, /*OldCount=*/0, /*NewCount=*/entry.StackCount );
        entry.LastObservedCount = entry.StackCount;
    }
}

void FGBFInventoryList::PostReplicatedChange( const TArrayView< int32 > changed_indices, int32 /*final_size*/ )
{
    for ( const auto index : changed_indices )
    {
        auto & entry = Entries[ index ];
        check( entry.LastObservedCount != INDEX_NONE );
        BroadcastChangeMessage( entry, /*OldCount=*/entry.LastObservedCount, /*NewCount=*/entry.StackCount );
        entry.LastObservedCount = entry.StackCount;
    }
}

bool FGBFInventoryList::NetDeltaSerialize( FNetDeltaSerializeInfo & delta_parameters )
{
    return FFastArraySerializer::FastArrayDeltaSerialize< FGBFInventoryEntry, FGBFInventoryList >( Entries, delta_parameters, *this );
}

UGBFInventoryItemInstance * FGBFInventoryList::AddEntry( TSubclassOf< UGBFInventoryItemDefinition > item_definition, const int32 stack_count )
{
    check( item_definition != nullptr );
    check( OwnerComponent != nullptr );

    const AActor * owning_actor = OwnerComponent->GetOwner();
    check( owning_actor->HasAuthority() );

    auto & new_entry = Entries.AddDefaulted_GetRef();
    new_entry.Instance = NewObject< UGBFInventoryItemInstance >( OwnerComponent->GetOwner() ); //@TODO: Using the actor instead of component as the outer due to UE-127172
    new_entry.Instance->SetItemDefinition( item_definition );

    for ( const UGBFInventoryItemFragment * fragment : GetDefault< UGBFInventoryItemDefinition >( item_definition )->Fragments )
    {
        if ( fragment != nullptr )
        {
            fragment->OnInstanceCreated( new_entry.Instance );
        }
    }
    new_entry.StackCount = stack_count;

    // const UGBFInventoryItemDefinition* ItemCDO = GetDefault<UGBFInventoryItemDefinition>(ItemDef);
    MarkItemDirty( new_entry );

    return new_entry.Instance;
}

void FGBFInventoryList::AddEntry( UGBFInventoryItemInstance * /*instance*/ )
{
    unimplemented();
}

void FGBFInventoryList::RemoveEntry( UGBFInventoryItemInstance * instance )
{
    for ( auto entry_it = Entries.CreateIterator(); entry_it; ++entry_it )
    {
        if ( auto & entry = *entry_it;
             entry.Instance == instance )
        {
            entry_it.RemoveCurrent();
            MarkArrayDirty();
        }
    }
}

void FGBFInventoryList::BroadcastChangeMessage( const FGBFInventoryEntry & entry, const int32 old_count, const int32 new_count )
{
    FGBFInventoryChangeMessage message;
    message.InventoryOwner = OwnerComponent;
    message.Instance = entry.Instance;
    message.NewCount = new_count;
    message.Delta = new_count - old_count;

    auto * message_system = UGameplayMessageSubsystem::Get( OwnerComponent->GetWorld() );
    message_system->BroadcastMessage( TAG_Gameplay_Inventory_Message_StackChanged, message );
}

UGBFInventoryManagerComponent::UGBFInventoryManagerComponent( const FObjectInitializer & object_initializer ) :
    Super( object_initializer ),
    InventoryList( this )
{
}

void UGBFInventoryManagerComponent::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
    Super::GetLifetimeReplicatedProps( OutLifetimeProps );

    DOREPLIFETIME( ThisClass, InventoryList );
}

bool UGBFInventoryManagerComponent::CanAddItemDefinition( TSubclassOf< UGBFInventoryItemDefinition > /*item_def*/, int32 /*stack_count*/ ) const
{
    //@TODO: Add support for stack limit / uniqueness checks / etc...
    return true;
}

UGBFInventoryItemInstance * UGBFInventoryManagerComponent::AddItemDefinition( const TSubclassOf< UGBFInventoryItemDefinition > item_definition, const int32 stack_count )
{
    UGBFInventoryItemInstance * result = nullptr;
    if ( item_definition != nullptr )
    {
        result = InventoryList.AddEntry( item_definition, stack_count );

        if ( IsUsingRegisteredSubObjectList() && IsReadyForReplication() && result != nullptr )
        {
            AddReplicatedSubObject( result );
        }
    }
    return result;
}

void UGBFInventoryManagerComponent::AddItemInstance( UGBFInventoryItemInstance * item_instance )
{
    InventoryList.AddEntry( item_instance );
    if ( IsUsingRegisteredSubObjectList() && IsReadyForReplication() && item_instance != nullptr )
    {
        AddReplicatedSubObject( item_instance );
    }
}

void UGBFInventoryManagerComponent::RemoveItemInstance( UGBFInventoryItemInstance * item_instance )
{
    InventoryList.RemoveEntry( item_instance );

    if ( item_instance != nullptr && IsUsingRegisteredSubObjectList() )
    {
        RemoveReplicatedSubObject( item_instance );
    }
}

TArray< UGBFInventoryItemInstance * > UGBFInventoryManagerComponent::GetAllItems() const
{
    return InventoryList.GetAllItems();
}

UGBFInventoryItemInstance * UGBFInventoryManagerComponent::FindFirstItemStackByDefinition( TSubclassOf< UGBFInventoryItemDefinition > item_definition ) const
{
    for ( const auto & entry : InventoryList.Entries )
    {
        if ( UGBFInventoryItemInstance * instance = entry.Instance;
             IsValid( instance ) )
        {
            if ( instance->GetItemDefinition() == item_definition )
            {
                return instance;
            }
        }
    }

    return nullptr;
}

int32 UGBFInventoryManagerComponent::GetTotalItemCountByDefinition( TSubclassOf< UGBFInventoryItemDefinition > item_definition ) const
{
    int32 total_count = 0;
    for ( const auto & entry : InventoryList.Entries )
    {
        if ( const UGBFInventoryItemInstance * instance = entry.Instance;
             IsValid( instance ) && instance->GetItemDefinition() == item_definition )
        {
            ++total_count;
        }
    }

    return total_count;
}

bool UGBFInventoryManagerComponent::ConsumeItemsByDefinition( TSubclassOf< UGBFInventoryItemDefinition > item_definition, int32 num_to_consume )
{
    if ( const AActor * owning_actor = GetOwner();
         owning_actor == nullptr || !owning_actor->HasAuthority() )
    {
        return false;
    }

    //@TODO: N squared right now as there's no acceleration structure
    auto total_consumed = 0;
    while ( total_consumed < num_to_consume )
    {
        if ( auto * Instance = FindFirstItemStackByDefinition( item_definition ) )
        {
            InventoryList.RemoveEntry( Instance );
            ++total_consumed;
        }
        else
        {
            return false;
        }
    }

    return total_consumed == num_to_consume;
}

bool UGBFInventoryManagerComponent::ReplicateSubobjects( UActorChannel * channel, FOutBunch * bunch, FReplicationFlags * rep_flags )
{
    bool wrote_something = Super::ReplicateSubobjects( channel, bunch, rep_flags );

    for ( auto & entry : InventoryList.Entries )
    {
        if ( UGBFInventoryItemInstance * instance = entry.Instance;
             instance != nullptr && IsValid( instance ) )
        {
            wrote_something |= channel->ReplicateSubobject( instance, *bunch, *rep_flags );
        }
    }

    return wrote_something;
}

void UGBFInventoryManagerComponent::ReadyForReplication()
{
    Super::ReadyForReplication();

    // Register existing ULyraInventoryItemInstance
    if ( IsUsingRegisteredSubObjectList() )
    {
        for ( const auto & entry : InventoryList.Entries )
        {
            if ( UGBFInventoryItemInstance * instance = entry.Instance;
                 IsValid( instance ) )
            {
                AddReplicatedSubObject( instance );
            }
        }
    }
}
