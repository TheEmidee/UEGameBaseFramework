#include "GameFeatures/GBFGameFeatureAction_AddInputBinding.h"

#include "Characters/Components/GBFHeroComponent.h"
#include "DVEDataValidator.h"
#include "Engine/LocalPlayer.h"

#include <Components/GameFrameworkComponentManager.h>
#include <EnhancedInputSubsystems.h>
#include <GameFramework/PlayerController.h>

void UGBFGameFeatureAction_AddInputBinding::OnGameFeatureActivating( FGameFeatureActivatingContext & context )
{
    if ( auto & active_data = ContextData.FindOrAdd( context );
         !ensure( active_data.ExtensionRequestHandles.IsEmpty() ) ||
         !ensure( active_data.PawnsAddedTo.IsEmpty() ) )
    {
        Reset( active_data );
    }
    Super::OnGameFeatureActivating( context );
}

void UGBFGameFeatureAction_AddInputBinding::OnGameFeatureDeactivating( FGameFeatureDeactivatingContext & context )
{
    Super::OnGameFeatureDeactivating( context );

    if ( auto * active_data = ContextData.Find( context );
         ensure( active_data ) )
    {
        Reset( *active_data );
    }
}

#if WITH_EDITOR
EDataValidationResult UGBFGameFeatureAction_AddInputBinding::IsDataValid( TArray< FText > & ValidationErrors )
{
    return FDVEDataValidator( ValidationErrors )
        .NoNullItem( VALIDATOR_GET_PROPERTY( InputConfigs ) )
        .Result();
}
#endif

void UGBFGameFeatureAction_AddInputBinding::AddToWorld( const FWorldContext & world_context, const FGameFeatureStateChangeContext & change_context )
{
    const UWorld * world = world_context.World();
    const UGameInstance * game_instance = world_context.OwningGameInstance;
    auto & [ extension_request_handles, pawns_added_to ] = ContextData.FindOrAdd( change_context );

    if ( game_instance != nullptr && world != nullptr && world->IsGameWorld() )
    {
        if ( auto * component_manager = UGameInstance::GetSubsystem< UGameFrameworkComponentManager >( game_instance ) )
        {
            const auto add_abilities_delegate = UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject( this, &ThisClass::HandlePawnExtension, change_context );
            const auto extension_request_handle = component_manager->AddExtensionHandler( APawn::StaticClass(), add_abilities_delegate );

            extension_request_handles.Add( extension_request_handle );
        }
    }
}

void UGBFGameFeatureAction_AddInputBinding::Reset( FPerContextData & active_data )
{
    active_data.ExtensionRequestHandles.Empty();

    while ( !active_data.PawnsAddedTo.IsEmpty() )
    {
        if ( auto pawn_ptr = active_data.PawnsAddedTo.Top();
             pawn_ptr.IsValid() )
        {
            RemoveInputMapping( pawn_ptr.Get(), active_data );
        }
        else
        {
            active_data.PawnsAddedTo.Pop();
        }
    }
}

void UGBFGameFeatureAction_AddInputBinding::HandlePawnExtension( AActor * actor, const FName event_name, const FGameFeatureStateChangeContext change_context )
{
    auto * pawn = CastChecked< APawn >( actor );
    auto & active_data = ContextData.FindOrAdd( change_context );

    if ( event_name == UGameFrameworkComponentManager::NAME_ExtensionRemoved || ( event_name == UGameFrameworkComponentManager::NAME_ReceiverRemoved ) )
    {
        RemoveInputMapping( pawn, active_data );
    }
    else if ( event_name == UGameFrameworkComponentManager::NAME_ExtensionAdded || event_name == UGBFHeroComponent::NAME_BindInputsNow )
    {
        AddInputMappingForPlayer( pawn, active_data );
    }
}

void UGBFGameFeatureAction_AddInputBinding::AddInputMappingForPlayer( APawn * pawn, FPerContextData & active_data )
{
    const auto * pc = Cast< APlayerController >( pawn->GetController() );

    if ( const auto * local_player = pc ? pc->GetLocalPlayer() : nullptr )
    {
        if ( local_player->GetSubsystem< UEnhancedInputLocalPlayerSubsystem >() != nullptr )
        {
            if ( auto * hero_component = pawn->FindComponentByClass< UGBFHeroComponent >();
                 hero_component != nullptr && hero_component->IsReadyToBindInputs() )
            {
                for ( const auto & entry : InputConfigs )
                {
                    if ( const auto * bind_set = entry.Get() )
                    {
                        hero_component->AddAdditionalInputConfig( bind_set );
                    }
                }
            }
            active_data.PawnsAddedTo.AddUnique( pawn );
        }
        else
        {
            UE_LOG( LogGameFeatures, Error, TEXT( "Failed to find `UEnhancedInputLocalPlayerSubsystem` for local player. Input mappings will not be added. Make sure you're set to use the EnhancedInput system via config file." ) );
        }
    }
}

void UGBFGameFeatureAction_AddInputBinding::RemoveInputMapping( APawn * pawn, FPerContextData & active_data )
{
    const auto * pc = Cast< APlayerController >( pawn->GetController() );

    if ( const auto * lp = pc ? pc->GetLocalPlayer() : nullptr )
    {
        if ( lp->GetSubsystem< UEnhancedInputLocalPlayerSubsystem >() != nullptr )
        {
            if ( auto * hero_component = pawn->FindComponentByClass< UGBFHeroComponent >() )
            {
                for ( const auto & entry : InputConfigs )
                {
                    if ( const auto * input_config = entry.Get() )
                    {
                        hero_component->RemoveAdditionalInputConfig( input_config );
                    }
                }
            }
        }
    }

    active_data.PawnsAddedTo.Remove( pawn );
}