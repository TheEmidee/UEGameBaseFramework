#include "Animation/GBFAnimNotifyState_InputBuffer.h"

#include <Components/SkeletalMeshComponent.h>

void UGBFAnimNotifyState_InputBuffer::NotifyBegin( USkeletalMeshComponent * mesh_component, UAnimSequenceBase * animation, float total_duration, const FAnimNotifyEventReference & event_reference )
{
    Super::NotifyBegin( mesh_component, animation, total_duration, event_reference );

    if ( auto * aibc = GetAbilityInputBufferComponent( mesh_component ) )
    {
        aibc->StartMonitoring( InputTagsToCheck, TriggerPriority );
    }
}

void UGBFAnimNotifyState_InputBuffer::NotifyEnd( USkeletalMeshComponent * mesh_component, UAnimSequenceBase * animation, const FAnimNotifyEventReference & event_reference )
{
    Super::NotifyEnd( mesh_component, animation, event_reference );

    if ( auto * aibc = GetAbilityInputBufferComponent( mesh_component ) )
    {
        aibc->StopMonitoring();
    }
}

UGBFAbilityInputBufferComponent * UGBFAnimNotifyState_InputBuffer::GetAbilityInputBufferComponent_Implementation( const USkeletalMeshComponent * mesh_component ) const
{
    if ( mesh_component == nullptr )
    {
        return nullptr;
    }

    if ( const auto * owner = mesh_component->GetOwner() )
    {
        if ( auto * aibc = owner->FindComponentByClass< UGBFAbilityInputBufferComponent >() )
        {
            return aibc;
        }
    }

    return nullptr;
}
