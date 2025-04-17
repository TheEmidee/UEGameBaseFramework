#include "Input/GBFInputMappingContextStackItem.h"

#include <EnhancedInputSubsystems.h>

#if WITH_EDITOR
#include "DVEDataValidator.h"
#endif

void UGBFInputMappingContextStackItem::AddToInputSystem( UEnhancedInputLocalPlayerSubsystem * input_system )
{
    if ( input_system == nullptr )
    {
        return;
    }

    for ( auto & imc : InputMappingContexts )
    {
        input_system->AddMappingContext( imc.Get(), 0 );
    }
}

void UGBFInputMappingContextStackItem::RemoveFromInputSystem( UEnhancedInputLocalPlayerSubsystem * input_system )
{
    if ( input_system == nullptr )
    {
        return;
    }

    for ( auto & imc : InputMappingContexts )
    {
        input_system->RemoveMappingContext( imc.Get() );
    }
}

#if WITH_EDITOR
EDataValidationResult UGBFInputMappingContextStackItem::IsDataValid( FDataValidationContext & context ) const
{
    return FDVEDataValidator( context )
        .NoNullItem( VALIDATOR_GET_PROPERTY( InputMappingContexts ) )
        .Result();
}
#endif