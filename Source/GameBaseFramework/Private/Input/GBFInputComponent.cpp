#include "Input/GBFInputComponent.h"

#include "Engine/GBFLocalPlayer.h"
#include "Input/GBFMappableConfigPair.h"
#include "Settings/GBFGameSettingRegistry.h"

#include <EnhancedInputSubsystems.h>

UGBFInputComponent::UGBFInputComponent( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
}

void UGBFInputComponent::AddInputMappings( const UGBFInputConfig * input_config, UEnhancedInputLocalPlayerSubsystem * input_system ) const
{
    check( input_config != nullptr );
    check( input_system != nullptr );

    const auto * local_player = input_system->GetLocalPlayer< UGBFLocalPlayer >();
    check( local_player != nullptr );

    // Add any registered input mappings from the settings!
    if ( const auto * local_settings = UGBFGameUserSettings::Get() )
    {
        // Tell enhanced input about any custom keymappings that the player may have customized
        for ( const TPair< FName, FKey > & pair : local_settings->GetCustomPlayerInputConfig() )
        {
            if ( pair.Key != NAME_None && pair.Value.IsValid() )
            {
                input_system->AddPlayerMappedKeyInSlot( pair.Key, pair.Value );
            }
        }
    }
}

void UGBFInputComponent::RemoveInputMappings( const UGBFInputConfig * input_config, UEnhancedInputLocalPlayerSubsystem * input_system ) const
{
    check( input_config != nullptr );
    check( input_system != nullptr );

    const auto * local_player = input_system->GetLocalPlayer< UGBFLocalPlayer >();
    check( local_player != nullptr );

    // Add any registered input mappings from the settings!
    if ( const auto * local_settings = UGBFGameUserSettings::Get() )
    {
        // Remove any registered input contexts
        const auto & configs = local_settings->GetAllRegisteredInputConfigs();
        for ( const auto & pair : configs )
        {
            input_system->RemovePlayerMappableConfig( pair.Config );
        }

        // Clear any player mapped keys from enhanced input
        for ( const TPair< FName, FKey > & pair : local_settings->GetCustomPlayerInputConfig() )
        {
            input_system->RemovePlayerMappedKeyInSlot( pair.Key );
        }
    }
}

void UGBFInputComponent::RemoveBinds( TArray< uint32 > & bind_handles )
{
    for ( const uint32 handle : bind_handles )
    {
        RemoveBindingByHandle( handle );
    }
    bind_handles.Reset();
}
