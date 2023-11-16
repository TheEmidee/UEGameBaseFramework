#pragma once

#include <Abilities/Tasks/AbilityTask.h>
#include <CoreMinimal.h>

#include "GBFAT_WaitBehaviorTreeNotRunning.generated.h"

class UBehaviorTreeComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FSWOnWaitBehaviorNotRunningDelegate );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAT_WaitBehaviorTreeNotRunning final : public UAbilityTask
{
    GENERATED_BODY()

public:
    UGBFAT_WaitBehaviorTreeNotRunning();

    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static UGBFAT_WaitBehaviorTreeNotRunning * WaitBehaviorTreeNotRunning( UGameplayAbility * owning_ability, UBehaviorTreeComponent * behavior_tree_component );

    void TickTask( float delta_time ) override;

private:
    void BroadcastDelegate();

    UPROPERTY( BlueprintAssignable )
    FSWOnWaitBehaviorNotRunningDelegate OnBehaviorTreeNotRunning;

    UPROPERTY()
    UBehaviorTreeComponent * BehaviorTreeComponent;
};
