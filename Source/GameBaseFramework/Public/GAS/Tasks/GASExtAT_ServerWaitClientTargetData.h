#pragma once

#include <Abilities/Tasks/AbilityTask.h>
#include <Abilities/Tasks/AbilityTask_WaitTargetData.h>
#include <CoreMinimal.h>

#include "GASExtAT_ServerWaitClientTargetData.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGASExtAT_ServerWaitClientTargetData final : public UAbilityTask
{
    GENERATED_BODY()

public:
    UPROPERTY( BlueprintAssignable )
    FWaitTargetDataDelegate ValidData;

    UFUNCTION( BlueprintCallable, meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator" ), Category = "Ability|Tasks" )
    static UGASExtAT_ServerWaitClientTargetData * ServerWaitForClientTargetData( UGameplayAbility * owning_ability, FName task_instance_name, bool trigger_once );

    virtual void Activate() override;

    UFUNCTION()
    void OnTargetDataReplicatedCallback( const FGameplayAbilityTargetDataHandle & data, FGameplayTag activation_tag );

protected:
    virtual void OnDestroy( bool ability_ended ) override;

    bool TriggerOnce;
};
