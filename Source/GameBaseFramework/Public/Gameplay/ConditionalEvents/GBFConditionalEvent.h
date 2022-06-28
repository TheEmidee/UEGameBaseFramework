#pragma once

#include "Abilities/GASExtGameplayAbility.h"

#include <CoreMinimal.h>

#include "GBFConditionalEvent.generated.h"

class UGBFConditionalTrigger;

UCLASS( HideDropdown )
class GAMEBASEFRAMEWORK_API UGBFConditionalEvent : public UGASExtGameplayAbility
{
    GENERATED_BODY()

public:
    UGBFConditionalEvent();

    void ActivateAbility( const FGameplayAbilitySpecHandle handle,
        const FGameplayAbilityActorInfo * actor_info,
        const FGameplayAbilityActivationInfo activation_info,
        const FGameplayEventData * trigger_event_data ) override;

    void EndAbility( const FGameplayAbilitySpecHandle handle,
        const FGameplayAbilityActorInfo * actor_info,
        const FGameplayAbilityActivationInfo activation_info,
        bool replicate_end_ability,
        bool was_cancelled ) override;

protected:
    UFUNCTION( BlueprintImplementableEvent, Category = "Conditional Events" )
    void ApplyOutcomes();

private:
    UFUNCTION()
    void OnTriggerTriggered();

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Conditional Events", meta = ( AllowPrivateAccess = true ) )
    TArray< UGBFConditionalTrigger * > Triggers;

    int TriggerCount;
};
