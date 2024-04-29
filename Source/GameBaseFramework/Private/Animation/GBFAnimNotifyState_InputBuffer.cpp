#include "Animation/GBFAnimNotifyState_InputBuffer.h"

#include "GBFLog.h"

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

    UGBFAbilityInputBufferComponent * aibc = nullptr;

    if ( const auto * owner = mesh_component->GetOwner() )
    {
        aibc = owner->FindComponentByClass< UGBFAbilityInputBufferComponent >();

        if ( aibc != nullptr )
        {
            return aibc;
        }
        //check all parent
        auto * parent = owner->GetParentActor();
        while ( parent )
        {
            aibc = parent->FindComponentByClass< UGBFAbilityInputBufferComponent >();
            if ( aibc != nullptr )
            {
                return aibc;
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
            aibc = child->FindComponentByClass< UGBFAbilityInputBufferComponent >();
            if ( aibc != nullptr )
            {
                return aibc;
            }
        }
    }

    UE_LOG( LogGBF, Error, TEXT( "No Ability Input Buffer Component found" ) );
    return nullptr;
}
