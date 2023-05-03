#include "Interaction/Tasks/GBFAT_WaitForInteractableTargets_SingleLineTrace.h"

#include "Interaction/GBFInteractableTarget.h"
#include "Interaction/GBFInteractionStatics.h"

#include <DrawDebugHelpers.h>
#include <Engine/World.h>
#include <TimerManager.h>

UGBFAT_WaitForInteractableTargets_SingleLineTrace::UGBFAT_WaitForInteractableTargets_SingleLineTrace()
{
    bEndTaskWhenTargetDataSent = false;
    TargetDataProductionRate = 1.0f;
}

UGBFAT_WaitForInteractableTargets_SingleLineTrace * UGBFAT_WaitForInteractableTargets_SingleLineTrace::WaitForInteractableTargets_SingleLineTrace(
    UGameplayAbility * owning_ability,
    const FName task_instance_name,
    const FGameplayAbilityTargetingLocationInfo & start_trace_location_infos,
    const FGASExtWaitTargetDataReplicationOptions & replication_options,
    const FGASExtWaitTargetDataHitScanOptions & hit_scan_options,
    const float interaction_scan_rate /*= 1.0f*/ )
{
    auto * my_obj = NewAbilityTask< UGBFAT_WaitForInteractableTargets_SingleLineTrace >( owning_ability, task_instance_name );
    my_obj->StartLocationInfo = start_trace_location_infos;
    my_obj->ReplicationOptions = replication_options;
    my_obj->Options = hit_scan_options;
    my_obj->TargetDataProductionRate = interaction_scan_rate;
    return my_obj;
}