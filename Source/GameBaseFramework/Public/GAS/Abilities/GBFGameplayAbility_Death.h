#pragma once

#include "GAS/Abilities/GASExtGameplayAbility.h"

#include <CoreMinimal.h>

#include "GBFGameplayAbility_Death.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameplayAbility_Death : public UGASExtGameplayAbility
{
    GENERATED_BODY()

public:
    UGBFGameplayAbility_Death();

protected:
    void ActivateAbility( const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilityActivationInfo activation_info, const FGameplayEventData * trigger_event_data ) override;
    void EndAbility( const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilityActivationInfo activation_info, bool replicate_end_ability, bool was_cancelled ) override;

    // Starts the death sequence.
    UFUNCTION( BlueprintCallable, Category = "Ability|Death" )
    void StartDeath();

    // Finishes the death sequence.
    UFUNCTION( BlueprintCallable, Category = "Ability|Death" )
    void FinishDeath();

    // If enabled, the ability will automatically call StartDeath.  FinishDeath is always called when the ability ends if the death was started.
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Death" )
    bool bAutoStartDeath;
};
