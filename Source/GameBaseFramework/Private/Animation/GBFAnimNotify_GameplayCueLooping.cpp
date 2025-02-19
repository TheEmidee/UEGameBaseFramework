#include "Animation/GBFAnimNotify_GameplayCueLooping.h"

#include <Components/SkeletalMeshComponent.h>
#include <GameplayCueFunctionLibrary.h>

void UGBFAnimNotify_GameplayCueLooping::NotifyBegin( USkeletalMeshComponent * mesh_component, UAnimSequenceBase * animation, float total_duration, const FAnimNotifyEventReference & event_reference )
{
    auto * owning_actor = mesh_component->GetAttachmentRootActor();

    if ( owning_actor != nullptr && GameplayCueTag.GetTagName() != NAME_None )
    {
        Parameters.TargetAttachComponent = mesh_component;
        UGameplayCueFunctionLibrary::AddGameplayCueOnActor( owning_actor, GameplayCueTag, Parameters );
    }
}

void UGBFAnimNotify_GameplayCueLooping::NotifyEnd( USkeletalMeshComponent * mesh_component, UAnimSequenceBase * animation, const FAnimNotifyEventReference & event_reference )
{
    Super::NotifyEnd( mesh_component, animation, event_reference );

    auto * owning_actor = mesh_component->GetAttachmentRootActor();

    if ( owning_actor != nullptr && GameplayCueTag.GetTagName() != NAME_None )
    {
        Parameters.TargetAttachComponent = mesh_component;
        UGameplayCueFunctionLibrary::RemoveGameplayCueOnActor( owning_actor, GameplayCueTag, Parameters );
    }
}