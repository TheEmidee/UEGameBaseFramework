#pragma once

#include "GBFAttributeSet.h"

#include <CoreMinimal.h>

#include "GBFHealthAttributeSet.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFHealthAttributeSet : public UGBFAttributeSet
{
    GENERATED_BODY()

public:
    UGBFHealthAttributeSet();

    FGBFAttributeEvent & OnOutOfHealth() const;
    FGBFAttributeEvent & OnOutOfShield() const;
    FGBFAttributeEvent & OnShieldAbsorbedDamage() const;
    FGBFAttributeEvent & OnDamaged() const;
    FGBFAttributeEvent & OnHealthChanged() const;
    FGBFAttributeEvent & OnMaxHealthChanged() const;
    FGBFAttributeEvent & OnShieldChanged() const;
    FGBFAttributeEvent & OnMaxShieldChanged() const;

    bool PreGameplayEffectExecute( FGameplayEffectModCallbackData & data ) override;
    void PreAttributeChange( const FGameplayAttribute & attribute, float & new_value ) override;
    void PreAttributeBaseChange( const FGameplayAttribute & attribute, float & new_value ) const override;
    void PostAttributeChange( const FGameplayAttribute & attribute, float old_value, float new_value ) override;
    void PostGameplayEffectExecute( const FGameplayEffectModCallbackData & data ) override;
    void GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const override;

    ATTRIBUTE_ACCESSORS( UGBFHealthAttributeSet, Health )
    ATTRIBUTE_ACCESSORS( UGBFHealthAttributeSet, MaxHealth )
    ATTRIBUTE_ACCESSORS( UGBFHealthAttributeSet, Shield )
    ATTRIBUTE_ACCESSORS( UGBFHealthAttributeSet, MaxShield )

protected:
    void ClampAttribute( const FGameplayAttribute & attribute, float & new_value ) const;

    UFUNCTION()
    virtual void OnRep_Health( FGameplayAttributeData old_value );

    UFUNCTION()
    virtual void OnRep_MaxHealth( FGameplayAttributeData old_value );

    UFUNCTION()
    virtual void OnRep_Shield( FGameplayAttributeData old_value );

    UFUNCTION()
    virtual void OnRep_MaxShield( FGameplayAttributeData old_value );

    UPROPERTY( BlueprintReadOnly, ReplicatedUsing = OnRep_Health, meta = ( AllowPrivateAccess = "true" ) )
    FGameplayAttributeData Health;

    UPROPERTY( BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, meta = ( AllowPrivateAccess = "true" ) )
    FGameplayAttributeData MaxHealth;

    UPROPERTY( BlueprintReadOnly, ReplicatedUsing = OnRep_Shield, meta = ( AllowPrivateAccess = "true" ) )
    FGameplayAttributeData Shield;

    UPROPERTY( BlueprintReadOnly, ReplicatedUsing = OnRep_MaxShield, meta = ( AllowPrivateAccess = "true" ) )
    FGameplayAttributeData MaxShield;

    mutable FGBFAttributeEvent OnOutOfHealthEvent;
    mutable FGBFAttributeEvent OnOutOfShieldEvent;
    mutable FGBFAttributeEvent OnShieldAbsorbedDamageEvent;
    mutable FGBFAttributeEvent OnDamageEvent;
    mutable FGBFAttributeEvent OnHealthChangedEvent;
    mutable FGBFAttributeEvent OnMaxHealthChangedEvent;
    mutable FGBFAttributeEvent OnShieldChangedEvent;
    mutable FGBFAttributeEvent OnMaxShieldChangedEvent;

    bool bOutOfHealth;
    bool bOutOfShield;

    float MaxHealthBeforeAttributeChange;
    float HealthBeforeAttributeChange;
    float MaxShieldBeforeAttributeChange;
    float ShieldBeforeAttributeChange;
};

FORCEINLINE FGBFAttributeEvent & UGBFHealthAttributeSet::OnOutOfHealth() const
{
    return OnOutOfHealthEvent;
}

FORCEINLINE FGBFAttributeEvent & UGBFHealthAttributeSet::OnOutOfShield() const
{
    return OnOutOfShieldEvent;
}

FORCEINLINE FGBFAttributeEvent & UGBFHealthAttributeSet::OnShieldAbsorbedDamage() const
{
    return OnShieldAbsorbedDamageEvent;
}

FORCEINLINE FGBFAttributeEvent & UGBFHealthAttributeSet::OnDamaged() const
{
    return OnDamageEvent;
}

FORCEINLINE FGBFAttributeEvent & UGBFHealthAttributeSet::OnHealthChanged() const
{
    return OnHealthChangedEvent;
}

FORCEINLINE FGBFAttributeEvent & UGBFHealthAttributeSet::OnMaxHealthChanged() const
{
    return OnMaxHealthChangedEvent;
}

FORCEINLINE FGBFAttributeEvent & UGBFHealthAttributeSet::OnShieldChanged() const
{
    return OnShieldChangedEvent;
}

FORCEINLINE FGBFAttributeEvent & UGBFHealthAttributeSet::OnMaxShieldChanged() const
{
    return OnMaxShieldChangedEvent;
}