#include "Abilities/GBFHealthAttributeSet.h"

#include "Components/GASExtAbilitySystemComponent.h"

#include <GameplayEffectExtension.h>
#include <Net/UnrealNetwork.h>

UGBFHealthAttributeSet::UGBFHealthAttributeSet()
{
    Health = 100.0f;
    MaxHealth = 100.0f;
    Shield = 0.0f;
    MaxShield = 0.0f;

    bOutOfHealth = false;
    bOutOfShield = false;
}

void UGBFHealthAttributeSet::PreAttributeChange( const FGameplayAttribute & attribute, float & new_value )
{
    Super::PreAttributeChange( attribute, new_value );
    ClampAttribute( attribute, new_value );
}

void UGBFHealthAttributeSet::PreAttributeBaseChange( const FGameplayAttribute & attribute, float & new_value ) const
{
    Super::PreAttributeBaseChange( attribute, new_value );
    ClampAttribute( attribute, new_value );
}

void UGBFHealthAttributeSet::PostAttributeChange( const FGameplayAttribute & attribute, const float old_value, const float new_value )
{
    Super::PostAttributeChange( attribute, old_value, new_value );

    if ( attribute == GetMaxHealthAttribute() )
    {
        // Make sure current health is not greater than the new max health.
        if ( GetHealth() > new_value )
        {
            auto * asc = GetAbilitySystemComponent();
            check( asc );

            asc->ApplyModToAttribute( GetHealthAttribute(), EGameplayModOp::Override, new_value );
        }
    }
    else if ( attribute == GetMaxShieldAttribute() )
    {
        // Make sure current shield is not greater than the new max shield.
        if ( GetShield() > new_value )
        {
            auto * asc = GetAbilitySystemComponent();
            check( asc );

            asc->ApplyModToAttribute( GetShieldAttribute(), EGameplayModOp::Override, new_value );
        }
    }

    if ( bOutOfHealth && GetHealth() > 0.0f )
    {
        bOutOfHealth = false;
    }

    if ( bOutOfShield && GetShield() > 0.0f )
    {
        bOutOfShield = false;
    }
}

void UGBFHealthAttributeSet::PostGameplayEffectExecute( const FGameplayEffectModCallbackData & data )
{
    Super::PostGameplayEffectExecute( data );

    const auto gameplay_effect_context_handle = data.EffectSpec.GetEffectContext();
    auto * instigator = gameplay_effect_context_handle.GetEffectCauser();
    const auto instigator_tags = *data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
    auto * target_actor = GetOwningActor();
    const auto target_tags = *data.EffectSpec.CapturedTargetTags.GetAggregatedTags();

    if ( data.EvaluatedData.Attribute == GetShieldAttribute() )
    {
        if ( data.EvaluatedData.Magnitude < 0.0f )
        {
            if ( OnShieldAbsorbedDamageEvent.IsBound() )
            {
                const auto & effect_context = gameplay_effect_context_handle;
                OnShieldAbsorbedDamageEvent.Broadcast( effect_context.GetOriginalInstigator(), effect_context.GetEffectCauser(), data.EffectSpec, data.EvaluatedData.Magnitude );
            }
        }

        if ( GetShield() <= 0.0f && !bOutOfShield )
        {
            if ( OnOutOfShieldEvent.IsBound() )
            {
                const auto & effect_context = gameplay_effect_context_handle;
                OnOutOfShieldEvent.Broadcast( effect_context.GetOriginalInstigator(), effect_context.GetEffectCauser(), data.EffectSpec, data.EvaluatedData.Magnitude );
            }
        }

        // Check health again in case an event above changed it.
        bOutOfShield = ( GetShield() <= 0.0f );
    }

    if ( data.EvaluatedData.Attribute == GetHealthAttribute() )
    {
        if ( data.EvaluatedData.Magnitude < 0.0f )
        {
            if ( OnDamageEvent.IsBound() )
            {
                const auto & effect_context = gameplay_effect_context_handle;
                OnDamageEvent.Broadcast( effect_context.GetOriginalInstigator(), effect_context.GetEffectCauser(), data.EffectSpec, data.EvaluatedData.Magnitude );
            }
        }

        if ( GetHealth() <= 0.0f && !bOutOfHealth )
        {
            if ( OnOutOfHealthEvent.IsBound() )
            {
                const auto & effect_context = gameplay_effect_context_handle;
                OnOutOfHealthEvent.Broadcast( effect_context.GetOriginalInstigator(), effect_context.GetEffectCauser(), data.EffectSpec, data.EvaluatedData.Magnitude );
            }
        }

        // Check health again in case an event above changed it.
        bOutOfHealth = ( GetHealth() <= 0.0f );
    }
}

void UGBFHealthAttributeSet::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
    Super::GetLifetimeReplicatedProps( OutLifetimeProps );

    DOREPLIFETIME_CONDITION_NOTIFY( UGBFHealthAttributeSet, Health, COND_None, REPNOTIFY_Always );
    DOREPLIFETIME_CONDITION_NOTIFY( UGBFHealthAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always );
    DOREPLIFETIME_CONDITION_NOTIFY( UGBFHealthAttributeSet, Shield, COND_None, REPNOTIFY_Always );
    DOREPLIFETIME_CONDITION_NOTIFY( UGBFHealthAttributeSet, MaxShield, COND_None, REPNOTIFY_Always );
}

void UGBFHealthAttributeSet::ClampAttribute( const FGameplayAttribute & attribute, float & new_value ) const
{
    if ( attribute == GetHealthAttribute() )
    {
        // Do not allow health to go negative or above max health.
        new_value = FMath::Clamp( new_value, 0.0f, GetMaxHealth() );
    }
    else if ( attribute == GetMaxHealthAttribute() )
    {
        // Do not allow max health to drop below 1.
        new_value = FMath::Max( new_value, 1.0f );
    }
    else if ( attribute == GetShieldAttribute() )
    {
        new_value = FMath::Clamp( new_value, 0.0f, GetMaxShield() );
    }
    else if ( attribute == GetMaxShieldAttribute() )
    {
        new_value = FMath::Max( new_value, 0.0f );
    }
}

void UGBFHealthAttributeSet::OnRep_Health( FGameplayAttributeData old_value )
{
    GAMEPLAYATTRIBUTE_REPNOTIFY( UGBFHealthAttributeSet, Health, old_value );
}

void UGBFHealthAttributeSet::OnRep_MaxHealth( FGameplayAttributeData old_value )
{
    GAMEPLAYATTRIBUTE_REPNOTIFY( UGBFHealthAttributeSet, MaxHealth, old_value );
}

void UGBFHealthAttributeSet::OnRep_Shield( FGameplayAttributeData old_value )
{
    GAMEPLAYATTRIBUTE_REPNOTIFY( UGBFHealthAttributeSet, Shield, old_value );
}

void UGBFHealthAttributeSet::OnRep_MaxShield( FGameplayAttributeData old_value )
{
    GAMEPLAYATTRIBUTE_REPNOTIFY( UGBFHealthAttributeSet, MaxShield, old_value );
}