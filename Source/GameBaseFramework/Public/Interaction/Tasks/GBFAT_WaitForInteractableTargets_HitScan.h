#pragma once

#include "GAS/Tasks/GBFAT_WaitTargetDataHitScan.h"

#include "GBFAT_WaitForInteractableTargets_HitScan.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAT_WaitForInteractableTargets_HitScan : public UGBFAT_WaitTargetDataHitScan
{
    GENERATED_BODY()

public:
    UGBFAT_WaitForInteractableTargets_HitScan();

    /** Wait until we trace new set of interactables.  This task automatically loops. */
    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = true ) )
    static UGBFAT_WaitForInteractableTargets_HitScan * WaitForInteractableTargets_HitScan(
        UGameplayAbility * owning_ability,
        FName task_instance_name,
        const FGameplayAbilityTargetingLocationInfo & start_trace_location_infos,
        const FGBFWaitTargetDataReplicationOptions & replication_options,
        const FGBFWaitTargetDataHitScanOptions & hit_scan_options,
        float interaction_scan_rate = 1.0f );

protected:
    FGameplayAbilityTargetDataHandle ProduceTargetData() override;

private:
    void SortTargetDataByDistanceToOwner( FGameplayAbilityTargetDataHandle & target_data_handle ) const;
};