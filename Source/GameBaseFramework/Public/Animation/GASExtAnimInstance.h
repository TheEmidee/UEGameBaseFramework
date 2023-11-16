#pragma once

#include <Animation/AnimInstance.h>
#include <CoreMinimal.h>
#include <GameplayEffectTypes.h>

#include "GASExtAnimInstance.generated.h"

class UAbilitySystemComponent;

UCLASS()
class GAMEBASEFRAMEWORK_API UGASExtAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    virtual void InitializeWithAbilitySystem( UAbilitySystemComponent * asc );

protected:
#if WITH_EDITOR
    EDataValidationResult IsDataValid( FDataValidationContext & context ) const override;
#endif // WITH_EDITOR

    void NativeInitializeAnimation() override;

    // Gameplay tags that can be mapped to blueprint variables. The variables will automatically update as the tags are added or removed.
    // These should be used instead of manually querying for the gameplay tags.
    UPROPERTY( EditDefaultsOnly, Category = "GameplayTags" )
    FGameplayTagBlueprintPropertyMap GameplayTagPropertyMap;
};
