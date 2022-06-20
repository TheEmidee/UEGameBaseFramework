#pragma once

#include <Abilities/Tasks/AbilityTask.h>
#include <CoreMinimal.h>

#include "GBFAT_WaitReceivedDamage.generated.h"

class UGBFHealthAttributeSet;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams( FGBFOnReceivedDamageDelegate, AActor *, damage_instigator, AActor *, damage_causer, const FGameplayEffectSpec &, damage_effect_spec, float, damage_magnitude, bool, is_shield_damage );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAT_WaitReceivedDamage final : public UAbilityTask
{
    GENERATED_BODY()

public:
    // If DamagedCharacter is null, this will try to use the avatar running the ability
    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = true ) )
    static UGBFAT_WaitReceivedDamage * WaitReceivedDamage( UGameplayAbility * owning_ability, AActor * damaged_character, bool observe_shield_absorption, bool observe_damage_to_health, bool triggers_once );

    void Activate() override;

private:
    void OnShieldAbsorbedDamage( AActor * damage_instigator, AActor * damage_causer, const FGameplayEffectSpec & damage_effect_spec, float damage_magnitude );
    void OnDamaged( AActor * damage_instigator, AActor * damage_causer, const FGameplayEffectSpec & damage_effect_spec, float damage_magnitude );

    void OnDestroy( bool it_has_finished_owner ) override;

    UPROPERTY( BlueprintAssignable, meta = ( AllowPrivateAccess = true ) )
    FGBFOnReceivedDamageDelegate OnReceivedDamageDelegate;

    UPROPERTY()
    AActor * DamagedCharacter;

    UPROPERTY()
    const UGBFHealthAttributeSet * HealthAttributeSet;

    bool bObserveShieldAbsorption;
    bool bObserveDamageToHealth;
    bool bItTriggersOnce;
};
