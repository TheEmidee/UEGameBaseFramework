#include "GAS/GBFAttributeSet.h"

#include "GAS/Components/GASExtAbilitySystemComponent.h"

UWorld * UGBFAttributeSet::GetWorld() const
{
    const UObject * outer = GetOuter();
    check( outer );

    return outer->GetWorld();
}

UGASExtAbilitySystemComponent * UGBFAttributeSet::GetAbilitySystemComponent() const
{
    return Cast< UGASExtAbilitySystemComponent >( GetOwningAbilitySystemComponent() );
}
