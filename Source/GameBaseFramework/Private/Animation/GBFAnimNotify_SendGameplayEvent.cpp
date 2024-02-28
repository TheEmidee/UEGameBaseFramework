#include "Animation/GBFAnimNotify_SendGameplayEvent.h"

#include <AbilitySystemComponent.h>
#include <AbilitySystemGlobals.h>
#include <Components/SkeletalMeshComponent.h>

void UGBFAnimNotify_SendGameplayEvent::Notify( USkeletalMeshComponent * mesh_comp, UAnimSequenceBase * animation, const FAnimNotifyEventReference & event_reference )
{
    Super::Notify( mesh_comp, animation, event_reference );

    check( mesh_comp != nullptr );

    if ( !EventTag.IsValid() )
    {
        return;
    }

    auto * owner = mesh_comp->GetOwner();
    if ( owner == nullptr )
    {
        return;
    }

    auto * target = GetTargetActorFromOwner( owner );
    if ( target == nullptr )
    {
        return;
    }

    auto * asc = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor( target );
    if ( asc == nullptr )
    {
        return;
    }

    FGameplayEventData event_data;
    event_data.EventTag = EventTag;
    event_data.Instigator = owner;
    event_data.Target = target;

    asc->HandleGameplayEvent( EventTag, &event_data );
}

AActor * UGBFAnimNotify_SendGameplayEvent::GetTargetActorFromOwner_Implementation( AActor * owner ) const
{
    return owner;
}
