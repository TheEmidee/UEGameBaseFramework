#include "GameFeatures/GBFGameFeatureAction_AddIMCStackItem.h"

#include "Characters/Components/GBFHeroComponent.h"

#include <Components/GameFrameworkComponentManager.h>
#include <Engine/GameInstance.h>

#if WITH_EDITOR
#include "DVEDataValidator.h"
#endif

void UGBFGameFeatureAction_AddIMCStackItem::OnGameFeatureActivating( FGameFeatureActivatingContext & context )
{
    if ( auto & active_data = ContextData.FindOrAdd( context );
        !ensure( active_data.ExtensionRequestHandles.IsEmpty() ) ||
        !ensure( active_data.PawnsAddedTo.IsEmpty() ) )
    {
        Reset( active_data );
    }

    Super::OnGameFeatureActivating( context );
}

void UGBFGameFeatureAction_AddIMCStackItem::OnGameFeatureDeactivating( FGameFeatureDeactivatingContext & context )
{
    Super::OnGameFeatureDeactivating( context );

    if ( auto * active_data = ContextData.Find( context );
        ensure( active_data ) )
    {
        Reset( *active_data );
    }
}

#if WITH_EDITOR
EDataValidationResult UGBFGameFeatureAction_AddIMCStackItem::IsDataValid( FDataValidationContext & context ) const
{
    return FDVEDataValidator( context )
        .NotNull( VALIDATOR_GET_PROPERTY( IMCStackItem ) )
        .Result();
}
#endif

void UGBFGameFeatureAction_AddIMCStackItem::AddToWorld( const FWorldContext & world_context, const FGameFeatureStateChangeContext & change_context )
{
    const auto * world = world_context.World();
    const auto game_instance = world_context.OwningGameInstance;
    auto & [ extension_request_handles, pawns_added_to ] = ContextData.FindOrAdd( change_context );

    if ( game_instance != nullptr && world != nullptr && world->IsGameWorld() )
    {
        if ( auto * component_manager = UGameInstance::GetSubsystem< UGameFrameworkComponentManager >( game_instance ) )
        {
            const auto add_imc_stack_item_delegate = UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject( this, &ThisClass::HandlePawnExtension, change_context );
            const auto extension_request_handle = component_manager->AddExtensionHandler( APawn::StaticClass(), add_imc_stack_item_delegate );

            extension_request_handles.Add( extension_request_handle );
        }
    }
}

void UGBFGameFeatureAction_AddIMCStackItem::Reset( FPerContextData & active_data )
{
    active_data.ExtensionRequestHandles.Empty();

    while ( !active_data.PawnsAddedTo.IsEmpty() )
    {
        if ( auto pawn_ptr = active_data.PawnsAddedTo.Top();
            pawn_ptr.IsValid() )
        {
            RemoveIMCStackItemForPlayer( pawn_ptr.Get(), active_data );
        }
        else
        {
            active_data.PawnsAddedTo.Pop();
        }
    }
}

void UGBFGameFeatureAction_AddIMCStackItem::HandlePawnExtension( AActor * actor, const FName event_name, const FGameFeatureStateChangeContext change_context )
{
    auto * pawn = CastChecked< APawn >( actor );
    auto & active_data = ContextData.FindOrAdd( change_context );

    if ( event_name == UGameFrameworkComponentManager::NAME_ExtensionRemoved || event_name == UGameFrameworkComponentManager::NAME_ReceiverRemoved )
    {
        RemoveIMCStackItemForPlayer( pawn, active_data );
    }
    else if ( ( event_name == UGameFrameworkComponentManager::NAME_ExtensionAdded ) || ( event_name == UGBFHeroComponent::NAME_BindInputsNow ) )
    {
        AddIMCStackItemForPlayer( pawn, active_data );
    }
}

void UGBFGameFeatureAction_AddIMCStackItem::AddIMCStackItemForPlayer( APawn * pawn, FPerContextData & active_data )
{
    auto * hero_component = UGBFHeroComponent::FindHeroComponent( pawn );
    if ( hero_component == nullptr )
    {
        return;
    }

    hero_component->AddIMCStackItem( IMCStackItem );
    active_data.PawnsAddedTo.AddUnique( pawn );
}

void UGBFGameFeatureAction_AddIMCStackItem::RemoveIMCStackItemForPlayer( APawn * pawn, FPerContextData & active_data )
{
    auto * hero_component = UGBFHeroComponent::FindHeroComponent( pawn );
    if ( hero_component == nullptr )
    {
        return;
    }

    hero_component->RemoveIMCStackItem( IMCStackItem );
    active_data.PawnsAddedTo.Remove( pawn );
}
