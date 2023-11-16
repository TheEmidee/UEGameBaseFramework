#include "GAS/GBFMovementAttributeSet.h"

#include <Net/UnrealNetwork.h>

UGBFMovementAttributeSet::UGBFMovementAttributeSet()
{
    MaxWalkSpeed = 0.0f;
}

void UGBFMovementAttributeSet::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
    Super::GetLifetimeReplicatedProps( OutLifetimeProps );

    DOREPLIFETIME_CONDITION_NOTIFY( UGBFMovementAttributeSet, MaxWalkSpeed, COND_None, REPNOTIFY_Always );
}

void UGBFMovementAttributeSet::OnRep_MaxWalkSpeed( FGameplayAttributeData old_value )
{
    GAMEPLAYATTRIBUTE_REPNOTIFY( UGBFMovementAttributeSet, MaxWalkSpeed, old_value );
}