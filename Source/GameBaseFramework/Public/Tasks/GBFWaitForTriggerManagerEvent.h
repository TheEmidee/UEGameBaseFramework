#pragma once

#include <Kismet/BlueprintAsyncActionBase.h>

#include "GBFWaitForTriggerManagerEvent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FGBFOnWaitTriggerManagerEventDelegate, AActor *, activator, int, actor_count );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFWaitForTriggerManagerEvent final : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, meta = ( BlueprintInternalUseOnly = true, WorldContext = "world_context_object" ) )
    static UGBFWaitForTriggerManagerEvent * WaitForTriggerManagerEvent( UObject * world_context_object,
        UGBFTriggerManagerComponent * trigger_manager_component,
        bool broadcast_trigger_count_on_activate,
        bool trigger_once = false );

    void Activate() override;
    void SetReadyToDestroy() override;

private:
    UFUNCTION()
    void OnTriggerActivated( AActor * activator );

    UFUNCTION()
    void OnActorInsideTriggerCountChanged( int actor_count );

    void CheckShouldEndTask();

    UPROPERTY( BlueprintAssignable )
    FGBFOnWaitTriggerManagerEventDelegate OnTriggerActivatedDelegate;

    UPROPERTY( BlueprintAssignable )
    FGBFOnWaitTriggerManagerEventDelegate OnActorInsideTriggerCountChangedDelegate;

    UPROPERTY()
    UGBFTriggerManagerComponent * TriggerManagerComponent;

    UPROPERTY()
    uint8 bBroadcastTriggerCountOnActivate : 1;

    UPROPERTY()
    uint8 bTriggerOnce : 1;
};