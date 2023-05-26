#include "GameFeatures/GBFGameFeatureAction_AddMappableInputConfig.h"

#include "Characters/Components/GBFHeroComponent.h"

#if WITH_EDITOR
#include "DVEDataValidator.h"
#endif

#include <Components/GameFrameworkComponentManager.h>
#include <Engine/LocalPlayer.h>
#include <EnhancedInputSubsystems.h>
#include <GameFramework/PlayerController.h>

#define LOCTEXT_NAMESPACE "UGBFGameFeatureAction_AddMappableInputConfig"

void UGBFGameFeatureAction_AddMappableInputConfig::OnGameFeatureRegistering()
{
    Super::OnGameFeatureRegistering();

    // Register the input configs with the local settings, this way the data inside them is available all the time
    // and not just when this game feature is active. This is necessary for displaying key binding options
    // on the main menu, or other times when the game feature may not be active.
    for ( const auto & pair : InputConfigs )
    {
        FGBFMappableConfigPair::RegisterPair( pair );
    }
}

void UGBFGameFeatureAction_AddMappableInputConfig::OnGameFeatureUnregistering()
{
    Super::OnGameFeatureUnregistering();

    for ( const auto & pair : InputConfigs )
    {
        FGBFMappableConfigPair::UnregisterPair( pair );
    }
}

void UGBFGameFeatureAction_AddMappableInputConfig::OnGameFeatureActivating( FGameFeatureActivatingContext & context )
{
    if ( auto & active_data = ContextData.FindOrAdd( context );
         !ensure( active_data.ExtensionRequestHandles.IsEmpty() ) ||
         !ensure( active_data.PawnsAddedTo.IsEmpty() ) )
    {
        Reset( active_data );
    }

    // Call super after the above logic so that we have our context before being added to the world
    Super::OnGameFeatureActivating( context );
}

void UGBFGameFeatureAction_AddMappableInputConfig::OnGameFeatureDeactivating( FGameFeatureDeactivatingContext & context )
{
    Super::OnGameFeatureDeactivating( context );

    if ( auto * active_data = ContextData.Find( context );
         ensure( active_data != nullptr ) )
    {
        Reset( *active_data );
    }
}

#if WITH_EDITOR
EDataValidationResult UGBFGameFeatureAction_AddMappableInputConfig::IsDataValid( TArray< FText > & validation_errors )
{
    return FDVEDataValidator( validation_errors )
        .CustomValidation< TArray< FGBFMappableConfigPair > >( InputConfigs, []( TArray< FText > & errors, TArray< FGBFMappableConfigPair > input_configs ) {
            auto entry_index = 0;

            for ( const auto & input_config : input_configs )
            {
                if ( input_config.Config.IsNull() )
                {
                    errors.Emplace( FText::Format( LOCTEXT( "EntryHasNullInputMapping", "Null InputMapping at index {0} in InputMappings" ), FText::AsNumber( entry_index ) ) );
                }

                ++entry_index;
            }
        } )
        .Result();
}
#endif // WITH_EDITOR

void UGBFGameFeatureAction_AddMappableInputConfig::AddToWorld( const FWorldContext & world_context, const FGameFeatureStateChangeContext & change_context )
{
    const auto * world = world_context.World();
    const auto game_instance = world_context.OwningGameInstance;
    auto & [ extension_request_handles, pawns_added_to ] = ContextData.FindOrAdd( change_context );

    if ( game_instance != nullptr && world != nullptr && world->IsGameWorld() )
    {
        if ( auto * component_manager = UGameInstance::GetSubsystem< UGameFrameworkComponentManager >( game_instance ) )
        {
            const auto add_config_delegate = UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject( this, &ThisClass::HandlePawnExtension, change_context );
            const auto extension_request_handle = component_manager->AddExtensionHandler( APawn::StaticClass(), add_config_delegate );
            extension_request_handles.Add( extension_request_handle );
        }
    }
}

void UGBFGameFeatureAction_AddMappableInputConfig::Reset( FPerContextData & active_data )
{
    active_data.ExtensionRequestHandles.Empty();

    while ( !active_data.PawnsAddedTo.IsEmpty() )
    {
        if ( auto pawn_ptr = active_data.PawnsAddedTo.Top();
             pawn_ptr.IsValid() )
        {
            RemoveInputConfig( pawn_ptr.Get(), active_data );
        }
        else
        {
            active_data.PawnsAddedTo.Pop();
        }
    }
}

void UGBFGameFeatureAction_AddMappableInputConfig::HandlePawnExtension( AActor * actor, FName event_name, const FGameFeatureStateChangeContext change_context )
{
    auto * pawn = CastChecked< APawn >( actor );
    auto & active_data = ContextData.FindOrAdd( change_context );

    if ( event_name == UGameFrameworkComponentManager::NAME_ExtensionAdded || event_name == UGBFHeroComponent::NAME_BindInputsNow )
    {
        AddInputConfig( pawn, active_data );
    }
    else if ( event_name == UGameFrameworkComponentManager::NAME_ExtensionRemoved || event_name == UGameFrameworkComponentManager::NAME_ReceiverRemoved )
    {
        RemoveInputConfig( pawn, active_data );
    }
}

void UGBFGameFeatureAction_AddMappableInputConfig::AddInputConfig( APawn * pawn, FPerContextData & active_data )
{
    const auto * pc = Cast< APlayerController >( pawn->GetController() );

    if ( const auto * lp = pc ? pc->GetLocalPlayer() : nullptr )
    {
        if ( auto * subsystem = lp->GetSubsystem< UEnhancedInputLocalPlayerSubsystem >() )
        {
            // We don't want to ignore keys that were "Down" when we add the mapping context
            // This allows you to die holding a movement key, keep holding while waiting for respawn,
            // and have it be applied after you respawn immediately. Leaving bIgnoreAllPressedKeysUntilRelease
            // to it's default "true" state would require the player to release the movement key,
            // and press it again when they respawn
            FModifyContextOptions options = {};
            options.bIgnoreAllPressedKeysUntilRelease = false;

            // Add the input mappings
            for ( const auto & pair : InputConfigs )
            {
                if ( pair.bShouldActivateAutomatically && pair.CanBeActivated() )
                {
                    subsystem->AddPlayerMappableConfig( pair.Config.LoadSynchronous(), options );
                }
                else
                {
                    subsystem->RemovePlayerMappableConfig( pair.Config.LoadSynchronous(), options );
                }
            }
            active_data.PawnsAddedTo.AddUnique( pawn );
        }
    }
}

void UGBFGameFeatureAction_AddMappableInputConfig::RemoveInputConfig( APawn * pawn, FPerContextData & active_data )
{
    const auto * pc = Cast< APlayerController >( pawn->GetController() );

    if ( const auto * lp = pc ? pc->GetLocalPlayer() : nullptr )
    {
        // If this is called during the shutdown of the game then there isn't a strict guarantee that the input subsystem is valid
        if ( auto * subsystem = lp->GetSubsystem< UEnhancedInputLocalPlayerSubsystem >() )
        {
            // Remove the input mappings
            for ( const auto & pair : InputConfigs )
            {
                subsystem->RemovePlayerMappableConfig( pair.Config.LoadSynchronous() );
            }
        }
    }
    active_data.PawnsAddedTo.Remove( pawn );
}

#undef LOCTEXT_NAMESPACE