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

    MaxHealthBeforeAttributeChange = 0.0f;
    HealthBeforeAttributeChange = 0.0f;
    MaxShieldBeforeAttributeChange = 0.0f;
    ShieldBeforeAttributeChange = 0.0f;
}

bool UGBFHealthAttributeSet::PreGameplayEffectExecute( FGameplayEffectModCallbackData & data )
{
    // Save the current health
    HealthBeforeAttributeChange = GetHealth();
    MaxHealthBeforeAttributeChange = GetMaxHealth();
    ShieldBeforeAttributeChange = GetShield();
    MaxShieldBeforeAttributeChange = GetMaxShield();

    return true;
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

    const auto effect_context = data.EffectSpec.GetEffectContext();
    auto * instigator = effect_context.GetOriginalInstigator();
    auto * causer = effect_context.GetEffectCauser();

    const auto instigator_tags = *data.EffectSpec.CapturedSourceTags.GetAggregatedTags();
    auto * target_actor = GetOwningActor();
    const auto target_tags = *data.EffectSpec.CapturedTargetTags.GetAggregatedTags();

    if ( data.EvaluatedData.Attribute == GetShieldAttribute() )
    {
        // Clamp and fall into out of shield handling below
        SetShield( FMath::Clamp( GetShield(), 0.0f, GetMaxShield() ) );
    }
    else if ( data.EvaluatedData.Attribute == GetMaxShieldAttribute() )
    {
        // TODO clamp current shield?

        // Notify on any requested max shield changes
        OnMaxShieldChangedEvent.Broadcast( instigator, causer, &data.EffectSpec, data.EvaluatedData.Magnitude, MaxShieldBeforeAttributeChange, GetMaxShield() );
    }
    else if ( data.EvaluatedData.Attribute == GetHealthAttribute() )
    {
        // Clamp and fall into out of health handling below
        SetHealth( FMath::Clamp( GetHealth(), 0.0f, GetMaxHealth() ) );
    }
    else if ( data.EvaluatedData.Attribute == GetMaxHealthAttribute() )
    {
        // TODO clamp current health?

        // Notify on any requested max health changes
        OnMaxHealthChangedEvent.Broadcast( instigator, causer, &data.EffectSpec, data.EvaluatedData.Magnitude, MaxHealthBeforeAttributeChange, GetMaxHealth() );
    }

    // If shield has actually changed activate callbacks
    if ( GetShield() != ShieldBeforeAttributeChange )
    {
        OnShieldChangedEvent.Broadcast( instigator, causer, &data.EffectSpec, data.EvaluatedData.Magnitude, ShieldBeforeAttributeChange, GetShield() );
    }

    if ( ( GetShield() <= 0.0f ) && !bOutOfShield )
    {
        OnOutOfShieldEvent.Broadcast( instigator, causer, &data.EffectSpec, data.EvaluatedData.Magnitude, ShieldBeforeAttributeChange, GetShield() );
    }

    // If health has actually changed activate callbacks
    if ( GetHealth() != HealthBeforeAttributeChange )
    {
        OnHealthChangedEvent.Broadcast( instigator, causer, &data.EffectSpec, data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHealth() );
    }

    if ( ( GetHealth() <= 0.0f ) && !bOutOfHealth )
    {
        OnOutOfHealthEvent.Broadcast( instigator, causer, &data.EffectSpec, data.EvaluatedData.Magnitude, HealthBeforeAttributeChange, GetHealth() );
    }

    // Check health/shield again in case an event above changed it.
    bOutOfShield = ( GetShield() <= 0.0f );
    bOutOfHealth = ( GetHealth() <= 0.0f );
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

    // Call the change callback, but without an instigator
    // This could be changed to an explicit RPC in the future
    // These events on the client should not be changing attributes
    const float current_health = GetHealth();
    const float estimated_magnitude = current_health - old_value.GetCurrentValue();
    OnHealthChangedEvent.Broadcast( nullptr, nullptr, nullptr, estimated_magnitude, old_value.GetCurrentValue(), current_health );

    if ( !bOutOfHealth && current_health <= 0.0f )
    {
        OnOutOfHealthEvent.Broadcast( nullptr, nullptr, nullptr, estimated_magnitude, old_value.GetCurrentValue(), current_health );
    }

    bOutOfHealth = ( current_health <= 0.0f );
}

void UGBFHealthAttributeSet::OnRep_MaxHealth( FGameplayAttributeData old_value )
{
    GAMEPLAYATTRIBUTE_REPNOTIFY( UGBFHealthAttributeSet, MaxHealth, old_value );

    // Call the change callback, but without an instigator
    // This could be changed to an explicit RPC in the future
    OnMaxHealthChangedEvent.Broadcast( nullptr, nullptr, nullptr, GetMaxHealth() - old_value.GetCurrentValue(), old_value.GetCurrentValue(), GetMaxHealth() );
}

void UGBFHealthAttributeSet::OnRep_Shield( FGameplayAttributeData old_value )
{
    GAMEPLAYATTRIBUTE_REPNOTIFY( UGBFHealthAttributeSet, Shield, old_value );

    // Call the change callback, but without an instigator
    // This could be changed to an explicit RPC in the future
    // These events on the client should not be changing attributes
    const float current_shield = GetShield();
    const float estimated_magnitude = current_shield - old_value.GetCurrentValue();
    OnShieldChangedEvent.Broadcast( nullptr, nullptr, nullptr, estimated_magnitude, old_value.GetCurrentValue(), current_shield );

    if ( !bOutOfShield && current_shield <= 0.0f )
    {
        OnOutOfShieldEvent.Broadcast( nullptr, nullptr, nullptr, estimated_magnitude, old_value.GetCurrentValue(), current_shield );
    }

    bOutOfShield = ( current_shield <= 0.0f );
}

void UGBFHealthAttributeSet::OnRep_MaxShield( FGameplayAttributeData old_value )
{
    GAMEPLAYATTRIBUTE_REPNOTIFY( UGBFHealthAttributeSet, MaxShield, old_value );

    // Call the change callback, but without an instigator
    // This could be changed to an explicit RPC in the future
    OnMaxShieldChangedEvent.Broadcast( nullptr, nullptr, nullptr, GetMaxShield() - old_value.GetCurrentValue(), old_value.GetCurrentValue(), GetMaxShield() );
}