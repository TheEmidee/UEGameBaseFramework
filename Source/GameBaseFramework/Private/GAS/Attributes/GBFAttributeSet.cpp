#include "GAS/Attributes/GBFAttributeSet.h"

#include "GAS/Components/GBFAbilitySystemComponent.h"

UWorld * UGBFAttributeSet::GetWorld() const
{
    const UObject * outer = GetOuter();
    check( outer );

    return outer->GetWorld();
}

UGBFAbilitySystemComponent * UGBFAttributeSet::GetAbilitySystemComponent() const
{
    return Cast< UGBFAbilitySystemComponent >( GetOwningAbilitySystemComponent() );
}
