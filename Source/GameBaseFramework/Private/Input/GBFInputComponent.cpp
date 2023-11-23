#include "Input/GBFInputComponent.h"

#include "Engine/GBFLocalPlayer.h"

#include <EnhancedInputSubsystems.h>

UGBFInputComponent::UGBFInputComponent( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
}

void UGBFInputComponent::AddInputMappings( const UGBFInputConfig * input_config, UEnhancedInputLocalPlayerSubsystem * input_system ) const
{
    check( input_config != nullptr );
    check( input_system != nullptr );

    // Here you can handle any custom logic to add something from your input config if required
}

void UGBFInputComponent::RemoveInputMappings( const UGBFInputConfig * input_config, UEnhancedInputLocalPlayerSubsystem * input_system ) const
{
    check( input_config != nullptr );
    check( input_system != nullptr );

    // Here you can handle any custom logic to remove input mappings that you may have added above
}

void UGBFInputComponent::RemoveBinds( TArray< uint32 > & bind_handles )
{
    for ( const uint32 handle : bind_handles )
    {
        RemoveBindingByHandle( handle );
    }
    bind_handles.Reset();
}
