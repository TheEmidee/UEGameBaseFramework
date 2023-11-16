#pragma once

#include <AbilitySystemGlobals.h>

#include "GBFAbilitySystemGlobals.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAbilitySystemGlobals : public UAbilitySystemGlobals
{
    GENERATED_BODY()

    FGameplayEffectContext * AllocGameplayEffectContext() const override;
};

