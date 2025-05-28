#include "Animation/GBFAnimNotify_GameplayCueLooping_WithLineTraceParameters.h"

#include <Components/SkeletalMeshComponent.h>
#include <GameplayCueFunctionLibrary.h>

void UGBFAnimNotify_GameplayCueLooping_WithLineTraceParameters::NotifyBegin( USkeletalMeshComponent * mesh_component, UAnimSequenceBase * animation, float total_duration, const FAnimNotifyEventReference & event_reference )
{
    auto * owning_actor = mesh_component->GetAttachmentRootActor();

    if ( owning_actor != nullptr && GameplayCueTag.GetTagName() != NAME_None )
    {
        FCollisionQueryParams collision_params;
        collision_params.bReturnPhysicalMaterial = bReturnPhysicalMaterial;

        if ( bIgnoreSelfActor )
        {
            collision_params.AddIgnoredActor( owning_actor );
        }

        auto start_location = owning_actor->GetActorLocation();
        auto end_location = owning_actor->GetActorLocation() + LineTraceVector;

        FHitResult hit_result;
        owning_actor->GetWorld()->LineTraceSingleByChannel( hit_result, start_location, end_location, LineTraceChannel, collision_params );

        auto gameplay_cue_parameters = UGameplayCueFunctionLibrary::MakeGameplayCueParametersFromHitResult( hit_result );
        gameplay_cue_parameters.TargetAttachComponent = mesh_component;

        if ( !bUseHitPointLocationForCue )
        {
            gameplay_cue_parameters.Location = FVector3d::Zero();
            gameplay_cue_parameters.Normal = FVector3d::Zero();
        }

        UGameplayCueFunctionLibrary::AddGameplayCueOnActor( owning_actor, GameplayCueTag, gameplay_cue_parameters );
    }
}

void UGBFAnimNotify_GameplayCueLooping_WithLineTraceParameters::NotifyEnd( USkeletalMeshComponent * mesh_component, UAnimSequenceBase * animation, const FAnimNotifyEventReference & event_reference )
{
    Super::NotifyEnd( mesh_component, animation, event_reference );

    auto * owning_actor = mesh_component->GetAttachmentRootActor();
    if ( owning_actor != nullptr && GameplayCueTag.GetTagName() != NAME_None )
    {
        FGameplayCueParameters gameplay_cue_parameters;
        gameplay_cue_parameters.TargetAttachComponent = mesh_component;
        UGameplayCueFunctionLibrary::RemoveGameplayCueOnActor( owning_actor, GameplayCueTag, gameplay_cue_parameters );
    }
}