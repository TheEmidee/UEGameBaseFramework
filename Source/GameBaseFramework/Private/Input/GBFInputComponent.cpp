#include "Input/GBFInputComponent.h"

#include "Engine/GBFLocalPlayer.h"

#include <EnhancedInputSubsystems.h>
#include <GameFramework/Pawn.h>

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

void UGBFInputComponent::AddInputMappingContextStackItem( UGBFInputMappingContextStackItem * input_mapping_context_stack_item )
{
    InternalAddInputMappingContextStackItem( input_mapping_context_stack_item );

    if ( !InputMappingContextStackItems.IsEmpty() )
    {
        InternalRemoveInputMappingContextStackItem( InputMappingContextStackItems.Last().Get() );
    }

    InputMappingContextStackItems.Add( input_mapping_context_stack_item );
}

void UGBFInputComponent::RemoveInputMappingContextStackItem( UGBFInputMappingContextStackItem * input_mapping_context_stack_item )
{
    if ( InputMappingContextStackItems.IsEmpty() )
    {
        return;
    }

    const auto was_last = InputMappingContextStackItems.Last() == input_mapping_context_stack_item;
    InputMappingContextStackItems.Remove( input_mapping_context_stack_item );

    if ( !was_last )
    {
        return;
    }

    InternalRemoveInputMappingContextStackItem( input_mapping_context_stack_item );

    if ( !InputMappingContextStackItems.IsEmpty() )
    {
        InternalAddInputMappingContextStackItem( InputMappingContextStackItems.Last().Get() );
    }
}

void UGBFInputComponent::InternalAddInputMappingContextStackItem( UGBFInputMappingContextStackItem * input_mapping_context_stack_item ) const
{
    if ( input_mapping_context_stack_item == nullptr )
    {
        return;
    }

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

    input_mapping_context_stack_item->AddInputMappings( input_system );
}

void UGBFInputComponent::InternalRemoveInputMappingContextStackItem( UGBFInputMappingContextStackItem * input_mapping_context_stack_item ) const
{
    if ( input_mapping_context_stack_item == nullptr )
    {
        return;
    }

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

    input_mapping_context_stack_item->RemoveInputMappings( input_system );
}
