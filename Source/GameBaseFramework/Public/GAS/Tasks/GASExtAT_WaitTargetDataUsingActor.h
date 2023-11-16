#pragma once

#include <Abilities/Tasks/AbilityTask.h>
#include <CoreMinimal.h>

#include "GASExtAT_WaitTargetDataUsingActor.generated.h"

class AGameplayAbilityTargetActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FSWOnWaitTargetDataUsingActorDelegate, const FGameplayAbilityTargetDataHandle &, data );

/**
* Waits for TargetData from an already spawned TargetActor and does *NOT* destroy it when it receives data.
*
* The original WaitTargetData's comments expects us to subclass it heavily, but the majority of its functions
* are not virtual. Therefore this is a total rewrite of it to add bCreateKeyIfNotValidForMorePredicting functionality.
*/

UCLASS()
class GAMEBASEFRAMEWORK_API UGASExtAT_WaitTargetDataUsingActor final : public UAbilityTask
{
    GENERATED_BODY()

public:
    UPROPERTY( BlueprintAssignable )
    FSWOnWaitTargetDataUsingActorDelegate ValidData;

    UPROPERTY( BlueprintAssignable )
    FSWOnWaitTargetDataUsingActorDelegate Cancelled;

    /**
	* Uses specified spawned TargetActor and waits for it to return valid data or to be canceled. The TargetActor is *NOT* destroyed.
	*
	* @param create_key_if_not_valid_for_more_predicting Will create a new scoped prediction key if the current scoped prediction key is not valid for more predicting.
	* If false, it will always create a new scoped prediction key. We would want to set this to true if we want to use a potentially existing valid scoped prediction
	* key like the ability's activation key in a batched ability.
	*/
    UFUNCTION( BlueprintCallable, meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator" ), Category = "Ability|Tasks" )
    static UGASExtAT_WaitTargetDataUsingActor * WaitTargetDataWithReusableActor(
        UGameplayAbility * owning_ability,
        FName task_instance_name,
        TEnumAsByte< EGameplayTargetingConfirmation::Type > confirmation_type,
        AGameplayAbilityTargetActor * target_actor,
        bool create_key_if_not_valid_for_more_predicting = false );

    virtual void Activate() override;

    UFUNCTION()
    void OnTargetDataReplicatedCallback( const FGameplayAbilityTargetDataHandle & data, FGameplayTag activation_tag );

    UFUNCTION()
    void OnTargetDataReplicatedCancelledCallback();

    UFUNCTION()
    void OnTargetDataReadyCallback( const FGameplayAbilityTargetDataHandle & data );

    UFUNCTION()
    void OnTargetDataCancelledCallback( const FGameplayAbilityTargetDataHandle & data );

    // Called when the ability is asked to confirm from an outside node. What this means depends on the individual task. By default, this does nothing other than ending if bEndTask is true.
    virtual void ExternalConfirm( bool end_task ) override;

    // Called when the ability is asked to cancel from an outside node. What this means depends on the individual task. By default, this does nothing other than ending the task.
    virtual void ExternalCancel() override;

protected:
    void InitializeTargetActor() const;
    void FinalizeTargetActor() const;
    void RegisterTargetDataCallbacks();
    void OnDestroy( bool ability_ended ) override;
    bool ShouldReplicateDataToServer() const;

    UPROPERTY()
    AGameplayAbilityTargetActor * TargetActor;

    bool CreateKeyIfNotValidForMorePredicting;
    TEnumAsByte< EGameplayTargetingConfirmation::Type > ConfirmationType;
    FDelegateHandle OnTargetDataReplicatedCallbackDelegateHandle;
};
