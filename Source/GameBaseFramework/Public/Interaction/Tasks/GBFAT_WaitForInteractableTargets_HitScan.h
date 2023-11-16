#pragma once

#include "GAS/Tasks/GASExtAT_WaitTargetDataHitScan.h"

#include "GBFAT_WaitForInteractableTargets_HitScan.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAT_WaitForInteractableTargets_HitScan : public UGASExtAT_WaitTargetDataHitScan
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
        const FGASExtWaitTargetDataReplicationOptions & replication_options,
        const FGASExtWaitTargetDataHitScanOptions & hit_scan_options,
        float interaction_scan_rate = 1.0f );

protected:
    FGameplayAbilityTargetDataHandle ProduceTargetData() override;

private:
    void SortTargetDataByDistanceToOwner( FGameplayAbilityTargetDataHandle & target_data_handle ) const;
};