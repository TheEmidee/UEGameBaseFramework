#pragma once

#include "Abilities/GASExtGameplayAbility.h"

#include <CoreMinimal.h>

#include "GBFConditionalEventAbility.generated.h"

class UGBFConditionalTrigger;

UCLASS( HideDropdown )
class GAMEBASEFRAMEWORK_API UGBFConditionalEventAbility : public UGASExtGameplayAbility
{
    GENERATED_BODY()

public:
    UGBFConditionalEventAbility();

    void ActivateAbility( const FGameplayAbilitySpecHandle handle,
        const FGameplayAbilityActorInfo * actor_info,
        const FGameplayAbilityActivationInfo activation_info,
        const FGameplayEventData * trigger_event_data ) override;

    void EndAbility( const FGameplayAbilitySpecHandle handle,
        const FGameplayAbilityActorInfo * actor_info,
        const FGameplayAbilityActivationInfo activation_info,
        bool replicate_end_ability,
        bool was_cancelled ) override;

#if WITH_EDITOR
    EDataValidationResult IsDataValid( TArray< FText > & validation_errors ) override;
#endif

protected:
    UFUNCTION( BlueprintImplementableEvent, Category = "Conditional Events" )
    void ExecuteOutcomes();

private:
    UFUNCTION()
    void OnTriggersTriggered( UGBFConditionalTrigger * trigger, bool triggered );

    void DeactivateTriggers();

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Instanced, Category = "Conditional Events", meta = ( AllowPrivateAccess = true ) )
    TArray< UGBFConditionalTrigger * > Triggers;

    UPROPERTY( EditDefaultsOnly, Category = "Conditional Events" )
    uint8 bEndAbilityAfterOutcomes : 1;

    UPROPERTY( EditDefaultsOnly, Category = "Conditional Events" )
    uint8 bExecuteOutcomesOnlyOnce : 1;

    UPROPERTY()
    TArray< UGBFConditionalTrigger * > TriggeredTriggers;
};