#include "GAS/GBFAbilitySystemGlobals.h"

#include "GAS/GBFAbilityTypesBase.h"

FGameplayEffectContext * UGBFAbilitySystemGlobals::AllocGameplayEffectContext() const
{
    return new FGBFGameplayEffectContext();
}
