#include "Input/GBFInputComponent.h"

#include "Engine/GBFLocalPlayer.h"
#include "GameFramework/Pawn.h"

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

void UGBFInputComponent::AddIMCStackItem( UGBFIMCStackItem * imc_stack_item )
{
    UGBFIMCStackItem * current_imc_stack_item = nullptr;

    if ( !IMCStackItems.IsEmpty() )
    {
        current_imc_stack_item = IMCStackItems[ 0 ].Get();
    }

    ManageStackItems( current_imc_stack_item, imc_stack_item );
    IMCStackItems.Insert( imc_stack_item, 0 );
}

void UGBFInputComponent::RemoveIMCStackItem( UGBFIMCStackItem * imc_stack_item )
{
    if ( IMCStackItems.IsEmpty() )
    {
        return;
    }

    const auto found_index = IMCStackItems.Find( imc_stack_item );
    IMCStackItems.Remove( imc_stack_item );

    if ( found_index != 0 )
    {
        return;
    }

    UGBFIMCStackItem * next_imc_stack_item = nullptr;

    if ( !IMCStackItems.IsEmpty() )
    {
        next_imc_stack_item = IMCStackItems[ 0 ].Get();
    }

    ManageStackItems( imc_stack_item, next_imc_stack_item );
}

void UGBFInputComponent::ManageStackItems( UGBFIMCStackItem * stack_item_to_remove, UGBFIMCStackItem * stack_item_to_add ) const
{
    const auto * pawn = Cast< APawn >( GetOwner() );
    if ( pawn == nullptr )
    {
        return;
    }

    const auto * pc = pawn->GetController< APlayerController >();
    if ( pc == nullptr )
    {
        return;
    }

    const auto * local_player = pc->GetLocalPlayer();
    if ( local_player == nullptr )
    {
        return;
    }

    auto * input_system = local_player->GetSubsystem< UEnhancedInputLocalPlayerSubsystem >();
    if ( input_system == nullptr )
    {
        return;
    }

    if ( stack_item_to_remove != nullptr )
    {
        stack_item_to_remove->RemoveInputMappings( input_system );
    }

    if ( stack_item_to_add != nullptr )
    {
        stack_item_to_add->AddInputMappings( input_system );
    }
}
