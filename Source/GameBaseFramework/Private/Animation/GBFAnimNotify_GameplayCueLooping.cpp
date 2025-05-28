#include "Animation/GBFAnimNotify_GameplayCueLooping.h"

#include <Components/SkeletalMeshComponent.h>
#include <GameplayCueFunctionLibrary.h>

void UGBFAnimNotify_GameplayCueLooping::NotifyBegin( USkeletalMeshComponent * mesh_component, UAnimSequenceBase * animation, float total_duration, const FAnimNotifyEventReference & event_reference )
{
    auto * owning_actor = mesh_component->GetAttachmentRootActor();

    if ( owning_actor != nullptr && GameplayCueTag.GetTagName() != NAME_None )
    {
        FGameplayCueParameters gameplay_cue_parameters;

        if ( bUseLineTraceToFillParameters )
        {
            FHitResult hit_result;

            const auto trace_start_location = owning_actor->GetActorLocation();
            const auto trace_end_location = owning_actor->GetActorLocation() + LineTraceVector;

            FCollisionQueryParams collision_params;
            collision_params.bReturnPhysicalMaterial = bGatherPhysicalMaterial;
            collision_params.AddIgnoredActor( owning_actor );

            owning_actor->GetWorld()->LineTraceSingleByChannel( hit_result, trace_start_location, trace_end_location, TraceCollisionChannel, collision_params );

            if ( bGatherPhysicalMaterial )
            {
                gameplay_cue_parameters.PhysicalMaterial = hit_result.PhysMaterial;
            }

            if ( bGatherTraceHitPointLocation )
            {
                gameplay_cue_parameters.Location = hit_result.Location;
                gameplay_cue_parameters.Normal = hit_result.Normal;
            }
        }

        gameplay_cue_parameters.TargetAttachComponent = mesh_component;
        UGameplayCueFunctionLibrary::AddGameplayCueOnActor( owning_actor, GameplayCueTag, gameplay_cue_parameters );
    }
}

void UGBFAnimNotify_GameplayCueLooping::NotifyEnd( USkeletalMeshComponent * mesh_component, UAnimSequenceBase * animation, const FAnimNotifyEventReference & event_reference )
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