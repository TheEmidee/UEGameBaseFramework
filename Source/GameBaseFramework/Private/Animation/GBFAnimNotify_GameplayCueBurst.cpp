#include "Animation/GBFAnimNotify_GameplayCueBurst.h"

#include "Components/SkeletalMeshComponent.h"
#include "GameplayCueFunctionLibrary.h"

void UGBFAnimNotify_GameplayCueBurst::Notify( USkeletalMeshComponent * mesh_component, UAnimSequenceBase * animation, const FAnimNotifyEventReference & event_reference )
{
    auto * owning_actor = mesh_component->GetAttachmentRootActor();

    if ( owning_actor != nullptr && GameplayCueTag.GetTagName() != NAME_None )
    {
        UGameplayCueFunctionLibrary::ExecuteGameplayCueOnActor( owning_actor, GameplayCueTag, Parameters );
    }
}