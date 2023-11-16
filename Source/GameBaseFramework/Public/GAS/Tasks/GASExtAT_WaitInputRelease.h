#pragma once

#include <Abilities/Tasks/AbilityTask_WaitInputRelease.h>
#include <CoreMinimal.h>

#include "GASExtAT_WaitInputRelease.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGASExtAT_WaitInputRelease final : public UAbilityTask_WaitInputRelease
{
    GENERATED_BODY()

public:
    void Activate() override;

    /** Wait until the user releases the input button for this ability's activation. Returns time from hitting this node, till release. Will return 0 if input was already released. */
    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "true" ) )
    static UGASExtAT_WaitInputRelease * SWWaitInputRelease( UGameplayAbility * owning_ability, bool it_tests_already_released = false );
};
