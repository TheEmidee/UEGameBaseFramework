#include "Abilities/GBFCombatAttributeSet.h"

#include <GameplayEffectExtension.h>
#include <Net/UnrealNetwork.h>

UGBFCombatAttributeSet::UGBFCombatAttributeSet()
{
    Damage = 0.0f;
    KnockBackMagnitude = 0.0f;
    KnockBackMultiplier = 0.0f;
}

void UGBFCombatAttributeSet::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
    Super::GetLifetimeReplicatedProps( OutLifetimeProps );

    DOREPLIFETIME_CONDITION_NOTIFY( UGBFCombatAttributeSet, Damage, COND_OwnerOnly, REPNOTIFY_Always );
    DOREPLIFETIME_CONDITION_NOTIFY( UGBFCombatAttributeSet, KnockBackMagnitude, COND_OwnerOnly, REPNOTIFY_Always );
    DOREPLIFETIME_CONDITION_NOTIFY( UGBFCombatAttributeSet, KnockBackMultiplier, COND_OwnerOnly, REPNOTIFY_Always );
}

void UGBFCombatAttributeSet::OnRep_Damage( const FGameplayAttributeData & old_value )
{
    GAMEPLAYATTRIBUTE_REPNOTIFY( UGBFCombatAttributeSet, Damage, old_value );
}

void UGBFCombatAttributeSet::OnRep_KnockBackMagnitude( const FGameplayAttributeData & old_value )
{
    GAMEPLAYATTRIBUTE_REPNOTIFY( UGBFCombatAttributeSet, KnockBackMagnitude, old_value );
}

void UGBFCombatAttributeSet::OnRep_KnockBackMultiplier( const FGameplayAttributeData & old_value )
{
    GAMEPLAYATTRIBUTE_REPNOTIFY( UGBFCombatAttributeSet, KnockBackMultiplier, old_value );
}
