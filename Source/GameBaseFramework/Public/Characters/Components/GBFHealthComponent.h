﻿#pragma once

#include <Components/ActorComponent.h>
#include <CoreMinimal.h>
#include <GameplayCueInterface.h>

#include "GBFHealthComponent.generated.h"

class UGBFHealthAttributeSet;
class UGBFAbilitySystemComponent;
class UGBFHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FGBFOnDeathDelegate, AActor *, owning_actor );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams( FGBFOnHealthAttributeChangedDelegate, UGBFHealthComponent *, health_component, float, old_value, float, new_value, AActor *, instigator );

UENUM( BlueprintType )
enum class EGBFDeathState : uint8
{
    NotDead = 0,
    DeathStarted,
    DeathFinished
};

UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class GAMEBASEFRAMEWORK_API UGBFHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGBFHealthComponent();

    UFUNCTION( BlueprintPure, Category = "GameBaseFramework|Health" )
    bool IsAlive() const;

    UFUNCTION( BlueprintPure, Category = "GameBaseFramework|Health" )
    EGBFDeathState GetDeathState() const;

    FGBFOnDeathDelegate & OnDeathStarted();
    FGBFOnDeathDelegate & OnDeathFinished();

    FGBFOnHealthAttributeChangedDelegate & OnHealthChanged();
    FGBFOnHealthAttributeChangedDelegate & OnMaxHealthChanged();

#if WITH_EDITOR
    FGameplayCueTag GetOnDamagedGameplayCueTag() const;
#endif

    void InitializeWithAbilitySystem( UGBFAbilitySystemComponent * asc );
    void UninitializeFromAbilitySystem();
    float GetHealth() const;
    float GetMaxHealth() const;
    float GetHealthNormalized() const;
    virtual void DamageSelfDestruct( AActor * killer ) const;
    void StartDeath();
    void FinishDeath();

    UFUNCTION( BlueprintPure, Category = "GameBaseFramework|Health" )
    static UGBFHealthComponent * FindHealthComponent( const AActor * actor );

    UFUNCTION( BlueprintCallable, Category = "GameBaseFramework|Health" )
    void Revive();

protected:
    void OnUnregister() override;
    virtual void OnStartDeath();
    virtual void OnFinishDeath();
    virtual void HandleOutOfHealth( AActor * damage_instigator, AActor * damage_causer, const FGameplayEffectSpec * damage_effect_spec, float damage_magnitude, const float old_value, const float new_value );
    virtual void ClearGameplayTags();

    UPROPERTY( ReplicatedUsing = OnRep_DeathState )
    EGBFDeathState DeathState;

    UPROPERTY()
    UGBFAbilitySystemComponent * AbilitySystemComponent;

private:
    void HandleHealthChanged( AActor * damage_instigator, AActor * damage_causer, const FGameplayEffectSpec * damage_effect_spec, float damage_magnitude, float old_value, float new_value );
    void HandleMaxHealthChanged( AActor * damage_instigator, AActor * damage_causer, const FGameplayEffectSpec * damage_effect_spec, float damage_magnitude, float old_value, float new_value );
    void HandleShieldAbsorbedDamage( AActor * damage_instigator, AActor * damage_causer, const FGameplayEffectSpec * damage_effect_spec, float damage_magnitude, float old_value, float new_value );
    void HandleOnDamaged( AActor * damage_instigator, AActor * damage_causer, const FGameplayEffectSpec * damage_effect_spec, float damage_magnitude, float old_value, float new_value );

    UFUNCTION()
    virtual void OnRep_DeathState( EGBFDeathState old_death_state );

    UPROPERTY()
    const UGBFHealthAttributeSet * HealthAttributeSet;

    UPROPERTY( BlueprintAssignable )
    FGBFOnHealthAttributeChangedDelegate OnHealthChangedDelegate;

    UPROPERTY( BlueprintAssignable )
    FGBFOnHealthAttributeChangedDelegate OnMaxHealthChangedDelegate;

    UPROPERTY( BlueprintAssignable )
    FGBFOnDeathDelegate OnDeathStartedDelegate;

    UPROPERTY( BlueprintAssignable )
    FGBFOnDeathDelegate OnDeathFinishedDelegate;

    UPROPERTY( EditDefaultsOnly )
    FGameplayCueTag OnShieldAbsorbedDamagedGameplayCueTag;

    UPROPERTY( EditDefaultsOnly )
    FGameplayCueTag OnDamagedGameplayCueTag;
};

#if WITH_EDITOR
FORCEINLINE FGameplayCueTag UGBFHealthComponent::GetOnDamagedGameplayCueTag() const
{
    return OnDamagedGameplayCueTag;
}
#endif

FORCEINLINE bool UGBFHealthComponent::IsAlive() const
{
    return DeathState == EGBFDeathState::NotDead;
}

FORCEINLINE EGBFDeathState UGBFHealthComponent::GetDeathState() const
{
    return DeathState;
}

FORCEINLINE FGBFOnDeathDelegate & UGBFHealthComponent::OnDeathStarted()
{
    return OnDeathStartedDelegate;
}

FORCEINLINE FGBFOnDeathDelegate & UGBFHealthComponent::OnDeathFinished()
{
    return OnDeathFinishedDelegate;
}

FORCEINLINE FGBFOnHealthAttributeChangedDelegate & UGBFHealthComponent::OnHealthChanged()
{
    return OnHealthChangedDelegate;
}

FORCEINLINE FGBFOnHealthAttributeChangedDelegate & UGBFHealthComponent::OnMaxHealthChanged()
{
    return OnMaxHealthChangedDelegate;
}