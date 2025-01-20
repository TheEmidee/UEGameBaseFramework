#include "Animation/GBFAnimNotify_GameplayCueLooping.h"

#include <Components/SkeletalMeshComponent.h>
#include <GameplayCueFunctionLibrary.h>

void UGBFAnimNotify_GameplayCueLooping::NotifyBegin( USkeletalMeshComponent * mesh_component, UAnimSequenceBase * animation, float total_duration, const FAnimNotifyEventReference & event_reference )
{
    auto * owning_actor = mesh_component->GetAttachmentRootActor();

    if ( owning_actor != nullptr && GameplayCueTag.GetTagName() != NAME_None )
    {
        UGameplayCueFunctionLibrary::AddGameplayCueOnActor( owning_actor, GameplayCueTag, Parameters );
    }
}