#pragma once

#include <Abilities/Tasks/AbilityTask.h>

#include "GBFAT_WaitForTriggerManagerEvent.generated.h"

class UGBFTriggerManagerComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FGASExtOnMonitorTriggerManagerDelegate, AActor *, activator, int, actor_count );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAT_WaitForTriggerManagerEvent : public UAbilityTask
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static UGBFAT_WaitForTriggerManagerEvent * WaitForTriggerManagerEvent( UGameplayAbility * owning_ability,
        UGBFTriggerManagerComponent * trigger_manager_component,
        bool broadcast_trigger_count_on_activate,
        bool trigger_once = false );

    void Activate() override;
    void OnDestroy( bool in_owner_finished ) override;

protected:
    UPROPERTY( BlueprintAssignable )
    FGASExtOnMonitorTriggerManagerDelegate OnTriggerActivatedDelegate;

    UPROPERTY( BlueprintAssignable )
    FGASExtOnMonitorTriggerManagerDelegate OnActorInsideTriggerCountChangedDelegate;

private:
    UFUNCTION()
    void OnTriggerActivated( AActor * activator );

    UFUNCTION()
    void OnActorInsideTriggerCountChanged( int actor_count );

    void CheckShouldEndTask();

    UPROPERTY()
    UGBFTriggerManagerComponent * TriggerManagerComponent;

    UPROPERTY()
    uint8 bBroadcastTriggerCountOnActivate : 1;

    UPROPERTY()
    uint8 bTriggerOnce : 1;
};