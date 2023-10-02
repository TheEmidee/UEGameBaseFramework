#include "Abilities/Tasks/GBFAT_WaitReceivedDamage.h"

#include "Abilities/GBFHealthAttributeSet.h"
#include "AbilitySystemBlueprintLibrary.h"

UGBFAT_WaitReceivedDamage * UGBFAT_WaitReceivedDamage::WaitReceivedDamage( UGameplayAbility * owning_ability, AActor * damaged_character, const bool observe_shield_absorption, const bool observe_damage_to_health, const bool triggers_once )
{
    auto * my_obj = NewAbilityTask< UGBFAT_WaitReceivedDamage >( owning_ability );
    my_obj->DamagedCharacter = damaged_character;
    my_obj->bObserveShieldAbsorption = observe_shield_absorption;
    my_obj->bObserveDamageToHealth = observe_damage_to_health;
    my_obj->bItTriggersOnce = triggers_once;
    return my_obj;
}

void UGBFAT_WaitReceivedDamage::Activate()
{
    Super::Activate();

    SetWaitingOnAvatar();

    if ( DamagedCharacter == nullptr )
    {
        DamagedCharacter = GetAvatarActor();
    }

    if ( DamagedCharacter != nullptr )
    {
        if ( auto * asc = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent( DamagedCharacter ) )
        {
            HealthAttributeSet = asc->GetSetChecked< UGBFHealthAttributeSet >();

            if ( HealthAttributeSet )
            {
                if ( bObserveDamageToHealth )
                {
                    HealthAttributeSet->OnDamaged().AddUObject( this, &ThisClass::OnDamaged );
                }
                if ( bObserveShieldAbsorption )
                {
                    HealthAttributeSet->OnShieldAbsorbedDamage().AddUObject( this, &ThisClass::OnShieldAbsorbedDamage );
                }
            }
        }
    }
}

void UGBFAT_WaitReceivedDamage::OnShieldAbsorbedDamage( AActor * damage_instigator, AActor * damage_causer, const FGameplayEffectSpec * damage_effect_spec, const float damage_magnitude, float /*old_value*/, float /*new_value*/ )
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        OnReceivedDamageDelegate.Broadcast( damage_instigator, damage_causer, *damage_effect_spec, -damage_magnitude, true );
    }

    if ( bItTriggersOnce )
    {
        EndTask();
    }
}

void UGBFAT_WaitReceivedDamage::OnDamaged( AActor * damage_instigator, AActor * damage_causer, const FGameplayEffectSpec * damage_effect_spec, const float damage_magnitude, float /*old_value*/, float /*new_value*/ )
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        OnReceivedDamageDelegate.Broadcast( damage_instigator, damage_causer, *damage_effect_spec, -damage_magnitude, false );
    }

    if ( bItTriggersOnce )
    {
        EndTask();
    }
}

void UGBFAT_WaitReceivedDamage::OnDestroy( const bool it_has_finished_owner )
{
    Super::OnDestroy( it_has_finished_owner );

    if ( HealthAttributeSet != nullptr )
    {
        HealthAttributeSet->OnDamaged().RemoveAll( this );
        HealthAttributeSet->OnShieldAbsorbedDamage().RemoveAll( this );
    }
}
