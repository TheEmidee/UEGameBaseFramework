#include "Animation/GBFAnimNotifyState_InputBuffer.h"

#include "GBFLog.h"

#include <Components/SkeletalMeshComponent.h>

void UGBFAnimNotifyState_InputBuffer::NotifyBegin( USkeletalMeshComponent * mesh_component, UAnimSequenceBase * animation, float total_duration, const FAnimNotifyEventReference & event_reference )
{
    Super::NotifyBegin( mesh_component, animation, total_duration, event_reference );

    if ( auto * ability_input_buffer_component = GetAbilityInputBufferComponent( mesh_component ) )
    {
        ability_input_buffer_component->StartMonitoring( InputTagsToCheck, InputTagsToCancel, TriggerPriority );
    }
}

void UGBFAnimNotifyState_InputBuffer::NotifyEnd( USkeletalMeshComponent * mesh_component, UAnimSequenceBase * animation, const FAnimNotifyEventReference & event_reference )
{
    Super::NotifyEnd( mesh_component, animation, event_reference );

    if ( auto * ability_input_buffer_component = GetAbilityInputBufferComponent( mesh_component ) )
    {
        ability_input_buffer_component->StopMonitoring();
    }
}

UGBFAbilityInputBufferComponent * UGBFAnimNotifyState_InputBuffer::GetAbilityInputBufferComponent_Implementation( const USkeletalMeshComponent * mesh_component ) const
{
    if ( mesh_component == nullptr )
    {
        return nullptr;
    }

    const auto * owner = mesh_component->GetOwner();
    if ( owner == nullptr )
    {
        UE_LOG( LogGBF, Error, TEXT( "Ability Input Buffer Notify : No Owner found" ) );
        return nullptr;
    }

    auto * ability_input_buffer_component = owner->FindComponentByClass< UGBFAbilityInputBufferComponent >();

    if ( ability_input_buffer_component != nullptr )
    {
        return ability_input_buffer_component;
    }

    // Check all parent
    auto * parent = owner->GetParentActor();
    while ( parent )
    {
        ability_input_buffer_component = parent->FindComponentByClass< UGBFAbilityInputBufferComponent >();
        if ( ability_input_buffer_component != nullptr )
        {
            return ability_input_buffer_component;
        }
        parent = parent->GetParentActor();
    }

    // Check all attached actors
    TArray< AActor * > actors;
    owner->GetAttachedActors( actors, true, true );
    actors.RemoveAll( []( AActor * actor ) {
        return !Cast< APawn >( actor );
    } );
    for ( auto & child : actors )
    {
        ability_input_buffer_component = child->FindComponentByClass< UGBFAbilityInputBufferComponent >();
        if ( ability_input_buffer_component != nullptr )
        {
            return ability_input_buffer_component;
        }
    }

    UE_LOG( LogGBF, Error, TEXT( "No Ability Input Buffer Component found" ) );
    return nullptr;
}
