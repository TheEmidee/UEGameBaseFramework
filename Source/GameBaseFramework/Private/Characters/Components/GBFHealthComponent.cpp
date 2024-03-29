﻿#include "Characters/Components/GBFHealthComponent.h"

#include "GAS/Attributes/GBFHealthAttributeSet.h"
#include "GAS/Components/GBFAbilitySystemComponent.h"
#include "GBFLog.h"
#include "Log/CoreExtLog.h"

#include <GameFramework/PlayerState.h>
#include <GameplayEffectExtension.h>
#include <GameplayEffectTypes.h>
#include <Net/UnrealNetwork.h>

UGBFHealthComponent::UGBFHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    PrimaryComponentTick.bStartWithTickEnabled = false;
    PrimaryComponentTick.bCanEverTick = false;

    SetIsReplicatedByDefault( true );

    AbilitySystemComponent = nullptr;
    HealthAttributeSet = nullptr;
    DeathState = EGBFDeathState::NotDead;
}

void UGBFHealthComponent::InitializeWithAbilitySystem( UGBFAbilitySystemComponent * asc )
{
    const auto * owner = GetOwner();
    check( owner );

    if ( AbilitySystemComponent != nullptr && AbilitySystemComponent != asc )
    {
        UE_LOG( LogGBF, Error, TEXT( "HealthComponent: Health component for owner [%s] has already been initialized with an ability system." ), *GetNameSafe( owner ) );
        return;
    }

    AbilitySystemComponent = asc;
    if ( AbilitySystemComponent == nullptr )
    {
        UE_LOG( LogGBF, Error, TEXT( "HealthComponent: Cannot initialize health component for owner [%s] with NULL ability system." ), *GetNameSafe( owner ) );
        return;
    }

    HealthAttributeSet = AbilitySystemComponent->GetSet< UGBFHealthAttributeSet >();
    if ( HealthAttributeSet == nullptr )
    {
        UE_LOG( LogGBF, Error, TEXT( "HealthComponent: Cannot initialize health component for owner [%s] with NULL health set on the ability system." ), *GetNameSafe( owner ) );
        return;
    }

    // Register to listen for attribute changes.
    HealthAttributeSet->OnHealthChanged().AddUObject( this, &ThisClass::HandleHealthChanged );
    HealthAttributeSet->OnMaxHealthChanged().AddUObject( this, &ThisClass::HandleMaxHealthChanged );
    HealthAttributeSet->OnOutOfHealth().AddUObject( this, &ThisClass::HandleOutOfHealth );
    HealthAttributeSet->OnShieldAbsorbedDamage().AddUObject( this, &ThisClass::HandleShieldAbsorbedDamage );
    HealthAttributeSet->OnDamaged().AddUObject( this, &ThisClass::HandleOnDamaged );

    Revive();
}

void UGBFHealthComponent::UninitializeFromAbilitySystem()
{
    ClearGameplayTags();

    if ( HealthAttributeSet != nullptr )
    {
        HealthAttributeSet->OnHealthChanged().RemoveAll( this );
        HealthAttributeSet->OnMaxHealthChanged().RemoveAll( this );
        HealthAttributeSet->OnOutOfHealth().RemoveAll( this );
        HealthAttributeSet->OnShieldAbsorbedDamage().RemoveAll( this );
        HealthAttributeSet->OnDamaged().RemoveAll( this );
    }

    HealthAttributeSet = nullptr;
    AbilitySystemComponent = nullptr;
}

float UGBFHealthComponent::GetHealth() const
{
    return HealthAttributeSet ? HealthAttributeSet->GetHealth() : 0.0f;
}

float UGBFHealthComponent::GetMaxHealth() const
{
    return HealthAttributeSet ? HealthAttributeSet->GetMaxHealth() : 0.0f;
}

float UGBFHealthComponent::GetHealthNormalized() const
{
    if ( HealthAttributeSet != nullptr )
    {
        const auto health = HealthAttributeSet->GetHealth();
        const auto max_health = HealthAttributeSet->GetMaxHealth();

        return max_health > 0.0f
                   ? health / max_health
                   : 0.0f;
    }

    return 0.0f;
}

void UGBFHealthComponent::DamageSelfDestruct( AActor * /*killer*/ ) const
{
    ensureAlwaysMsgf( false, TEXT( "Implement this function in your own implementation" ) );
}

void UGBFHealthComponent::StartDeath()
{
    if ( DeathState != EGBFDeathState::NotDead )
    {
        return;
    }

    DeathState = EGBFDeathState::DeathStarted;

    OnStartDeath();

    auto * owner = GetOwner();
    check( owner );

    OnDeathStartedDelegate.Broadcast( owner );

    owner->ForceNetUpdate();
}

void UGBFHealthComponent::FinishDeath()
{
    if ( DeathState != EGBFDeathState::DeathStarted )
    {
        return;
    }

    DeathState = EGBFDeathState::DeathFinished;

    OnFinishDeath();

    auto * owner = GetOwner();
    check( owner );

    OnDeathFinishedDelegate.Broadcast( owner );

    owner->ForceNetUpdate();
}

UGBFHealthComponent * UGBFHealthComponent::FindHealthComponent( const AActor * actor )
{
    return ( actor ? actor->FindComponentByClass< UGBFHealthComponent >() : nullptr );
}

void UGBFHealthComponent::Revive()
{
    if ( AbilitySystemComponent == nullptr )
    {
        return;
    }

    ClearGameplayTags();
    DeathState = EGBFDeathState::NotDead;

    // TEMP: Reset attributes to default values.  Eventually this will be driven by a spread sheet.
    AbilitySystemComponent->SetNumericAttributeBase( UGBFHealthAttributeSet::GetHealthAttribute(), HealthAttributeSet->GetMaxHealth() );

    OnHealthChangedDelegate.Broadcast( this, HealthAttributeSet->GetHealth(), HealthAttributeSet->GetHealth(), nullptr );
    OnMaxHealthChangedDelegate.Broadcast( this, HealthAttributeSet->GetHealth(), HealthAttributeSet->GetHealth(), nullptr );
}

void UGBFHealthComponent::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
    Super::GetLifetimeReplicatedProps( OutLifetimeProps );

    DOREPLIFETIME( ThisClass, DeathState );
}

void UGBFHealthComponent::OnUnregister()
{
    UninitializeFromAbilitySystem();

    Super::OnUnregister();
}

void UGBFHealthComponent::OnStartDeath()
{
}

void UGBFHealthComponent::OnFinishDeath()
{
}

void UGBFHealthComponent::HandleOutOfHealth( AActor * /*damage_instigator*/, AActor * /*damage_causer*/, const FGameplayEffectSpec * /*damage_effect_spec*/, float /*damage_magnitude*/, const float /*old_value*/, const float /*new_value*/ )
{
    ensureAlwaysMsgf( false, TEXT( "Implement this function in your own implementation" ) );
}

void UGBFHealthComponent::ClearGameplayTags()
{
}

void UGBFHealthComponent::HandleHealthChanged( AActor * damage_instigator, AActor * /*damage_causer*/, const FGameplayEffectSpec * /*damage_effect_spec*/, float /*damage_magnitude*/, const float old_value, const float new_value )
{
    OnHealthChangedDelegate.Broadcast( this, old_value, new_value, damage_instigator );
}

void UGBFHealthComponent::HandleMaxHealthChanged( AActor * damage_instigator, AActor * /*damage_causer*/, const FGameplayEffectSpec * /*damage_effect_spec*/, float /*damage_magnitude*/, const float old_value, const float new_value )
{
    OnMaxHealthChangedDelegate.Broadcast( this, old_value, new_value, damage_instigator );
}

void UGBFHealthComponent::HandleShieldAbsorbedDamage( AActor * /*damage_instigator*/, AActor * /*damage_causer*/, const FGameplayEffectSpec * damage_effect_spec, float /*damage_magnitude*/, const float /*old_value*/, const float /*new_value*/ )
{
#if WITH_SERVER_CODE
    if ( AbilitySystemComponent != nullptr && OnShieldAbsorbedDamagedGameplayCueTag.IsValid() )
    {
        const FGameplayCueParameters parameters( damage_effect_spec->GetContext() );
        AbilitySystemComponent->ExecuteGameplayCue( OnShieldAbsorbedDamagedGameplayCueTag.GameplayCueTag, parameters );
    }
#endif
}

void UGBFHealthComponent::HandleOnDamaged( AActor * /*damage_instigator*/, AActor * /*damage_causer*/, const FGameplayEffectSpec * damage_effect_spec, float /*damage_magnitude*/, const float /*old_value*/, const float /*new_value*/ )
{
#if WITH_SERVER_CODE
    if ( AbilitySystemComponent != nullptr && OnDamagedGameplayCueTag.IsValid() )
    {
        const FGameplayCueParameters parameters( damage_effect_spec->GetContext() );
        AbilitySystemComponent->ExecuteGameplayCue( OnDamagedGameplayCueTag.GameplayCueTag, parameters );
    }
#endif
}

void UGBFHealthComponent::OnRep_DeathState( EGBFDeathState old_death_state )
{
    const auto new_death_state = DeathState;

    if ( old_death_state > new_death_state )
    {
        // Clear gameplay tags on clients too when we are revived
        if ( new_death_state == EGBFDeathState::NotDead )
        {
            ClearGameplayTags();
        }

        // :TODO: Commented for now because a revive can trigger this. which should not give a warning
        // The server is trying to set us back but we've already predicted past the server state.
        // UE_LOG( LogGBF, Warning, TEXT( "HealthComponent: Predicted past server death state [%d] -> [%d] for owner [%s]." ), static_cast< uint8 >( old_death_state ), static_cast< uint8 >( new_death_state ), *GetNameSafe( GetOwner() ) );
        return;
    }

    // Revert the death state for now since we rely on StartDeath and FinishDeath to change it.
    DeathState = old_death_state;

    if ( old_death_state == EGBFDeathState::NotDead )
    {
        if ( new_death_state == EGBFDeathState::DeathStarted )
        {
            StartDeath();
        }
        else if ( new_death_state == EGBFDeathState::DeathFinished )
        {
            StartDeath();
            FinishDeath();
        }
        else
        {
            UE_LOG( LogGBF, Error, TEXT( "HealthComponent: Invalid death transition [%d] -> [%d] for owner [%s]." ), static_cast< uint8 >( old_death_state ), static_cast< uint8 >( new_death_state ), *GetNameSafe( GetOwner() ) );
        }
    }
    else if ( old_death_state == EGBFDeathState::DeathStarted )
    {
        if ( new_death_state == EGBFDeathState::DeathFinished )
        {
            FinishDeath();
        }
        else
        {
            UE_LOG( LogGBF, Error, TEXT( "HealthComponent: Invalid death transition [%d] -> [%d] for owner [%s]." ), static_cast< uint8 >( old_death_state ), static_cast< uint8 >( new_death_state ), *GetNameSafe( GetOwner() ) );
        }
    }

    ensureMsgf( ( DeathState == new_death_state ), TEXT( "HealthComponent: Death transition failed [%d] -> [%d] for owner [%s]." ), static_cast< uint8 >( old_death_state ), static_cast< uint8 >( new_death_state ), *GetNameSafe( GetOwner() ) );
}
