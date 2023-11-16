#pragma once

#include <AbilitySystemComponent.h>
#include <AttributeSet.h>
#include <CoreMinimal.h>

#include "GBFAttributeSet.generated.h"

#define ATTRIBUTE_ACCESSORS( ClassName, PropertyName )           \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER( ClassName, PropertyName ) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER( PropertyName )               \
    GAMEPLAYATTRIBUTE_VALUE_SETTER( PropertyName )               \
    GAMEPLAYATTRIBUTE_VALUE_INITTER( PropertyName )

/**
 * Delegate used to broadcast attribute events, some of these parameters may be null on clients:
 * @param EffectInstigator	The original instigating actor for this event
 * @param EffectCauser		The physical actor that caused the change
 * @param EffectSpec		The full effect spec for this change
 * @param EffectMagnitude	The raw magnitude, this is before clamping
 * @param OldValue			The value of the attribute before it was changed
 * @param NewValue			The value after it was changed
 */
DECLARE_MULTICAST_DELEGATE_SixParams( FGBFAttributeEvent, AActor * /*EffectInstigator*/, AActor * /*EffectCauser*/, const FGameplayEffectSpec * /*EffectSpec*/, float /*EffectMagnitude*/, float /*OldValue*/, float /*NewValue*/ );

class UGBFAbilitySystemComponent;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAttributeSet : public UAttributeSet
{
    GENERATED_BODY()

public:
    UWorld * GetWorld() const override;
    UGBFAbilitySystemComponent * GetAbilitySystemComponent() const;
};