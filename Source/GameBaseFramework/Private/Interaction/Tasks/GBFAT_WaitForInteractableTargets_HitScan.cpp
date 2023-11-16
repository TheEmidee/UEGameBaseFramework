#include "Interaction/Tasks/GBFAT_WaitForInteractableTargets_HitScan.h"

UGBFAT_WaitForInteractableTargets_HitScan::UGBFAT_WaitForInteractableTargets_HitScan()
{
    bEndTaskWhenTargetDataSent = false;
    TargetDataProductionRate = 1.0f;
}

UGBFAT_WaitForInteractableTargets_HitScan * UGBFAT_WaitForInteractableTargets_HitScan::WaitForInteractableTargets_HitScan(
    UGameplayAbility * owning_ability,
    const FName task_instance_name,
    const FGameplayAbilityTargetingLocationInfo & start_trace_location_infos,
    const FGBFWaitTargetDataReplicationOptions & replication_options,
    const FGBFWaitTargetDataHitScanOptions & hit_scan_options,
    const float interaction_scan_rate /*= 1.0f*/ )
{
    auto * my_obj = NewAbilityTask< UGBFAT_WaitForInteractableTargets_HitScan >( owning_ability, task_instance_name );
    my_obj->StartLocationInfo = start_trace_location_infos;
    my_obj->ReplicationOptions = replication_options;
    my_obj->Options = hit_scan_options;
    my_obj->TargetDataProductionRate = interaction_scan_rate;
    return my_obj;
}

FGameplayAbilityTargetDataHandle UGBFAT_WaitForInteractableTargets_HitScan::ProduceTargetData()
{
    auto target_data_handle = Super::ProduceTargetData();
    SortTargetDataByDistanceToOwner( target_data_handle );

    return target_data_handle;
}

void UGBFAT_WaitForInteractableTargets_HitScan::SortTargetDataByDistanceToOwner( FGameplayAbilityTargetDataHandle & target_data_handle ) const
{
    const auto character_location = GetAvatarActor()->GetActorLocation();

    target_data_handle.Data.Sort( [ character_location ]( auto left, auto right ) {
        auto * left_actor = left->GetHitResult()->GetActor();
        auto * right_actor = right->GetHitResult()->GetActor();

        if ( left_actor == nullptr || right_actor == nullptr )
        {
            return false;
        }

        return FVector::DistSquared( character_location, left_actor->GetActorLocation() ) <
               FVector::DistSquared( character_location, right_actor->GetActorLocation() );
    } );
}