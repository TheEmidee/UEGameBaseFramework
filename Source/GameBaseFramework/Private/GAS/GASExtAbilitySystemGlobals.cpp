#include "GAS/GASExtAbilitySystemGlobals.h"

#include "GAS/GASExtAbilityTypesBase.h"

FGameplayEffectContext * UGASExtAbilitySystemGlobals::AllocGameplayEffectContext() const
{
    return new FGASExtGameplayEffectContext();
}
