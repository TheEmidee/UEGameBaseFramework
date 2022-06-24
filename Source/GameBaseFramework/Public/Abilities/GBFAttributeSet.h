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

DECLARE_MULTICAST_DELEGATE_FourParams( FGBFAttributeEvent, AActor * /*EffectInstigator*/, AActor * /*EffectCauser*/, const FGameplayEffectSpec & /*EffectSpec*/, float /*EffectMagnitude*/ );

class UGASExtAbilitySystemComponent;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAttributeSet : public UAttributeSet
{
    GENERATED_BODY()

public:
    UWorld * GetWorld() const override;
    UGASExtAbilitySystemComponent * GetAbilitySystemComponent() const;
};