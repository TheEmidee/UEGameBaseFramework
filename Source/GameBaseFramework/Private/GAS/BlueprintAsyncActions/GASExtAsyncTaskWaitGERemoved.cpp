#include "GAS/BlueprintAsyncActions/GASExtAsyncTaskWaitGERemoved.h"

#include <AbilitySystemComponent.h>

UGASExtAsyncTaskWaitGERemoved * UGASExtAsyncTaskWaitGERemoved::WaitGameplayEffectRemovedWithTags( UAbilitySystemComponent * ability_system_component, FGameplayTagContainer gameplay_effect_owned_tags, bool trigger_if_no_initial_matching_effect /*= true*/ )
{
    return WaitGameplayEffectRemovedWithQuery( ability_system_component, FGameplayEffectQuery::MakeQuery_MatchAllEffectTags( gameplay_effect_owned_tags ), trigger_if_no_initial_matching_effect );
}

UGASExtAsyncTaskWaitGERemoved * UGASExtAsyncTaskWaitGERemoved::WaitGameplayEffectRemovedWithQuery( UAbilitySystemComponent * ability_system_component, FGameplayEffectQuery query, bool trigger_if_no_initial_matching_effect /*= true*/ )
{
    auto * result = NewObject< UGASExtAsyncTaskWaitGERemoved >();
    result->ASC = ability_system_component;
    result->Query = query;
    result->bTriggerIfNoInitialMatchingGameplayEffect = trigger_if_no_initial_matching_effect;
    return result;
}

void UGASExtAsyncTaskWaitGERemoved::Activate()
{
    Super::Activate();

    if ( !IsValid( ASC ) )
    {
        SetReadyToDestroy();
        return;
    }

    if ( bTriggerIfNoInitialMatchingGameplayEffect && ASC->GetActiveEffects( Query ).Num() == 0 )
    {
        Broadcast( FActiveGameplayEffectHandle() );
        return;
    }

    ASC->OnAnyGameplayEffectRemovedDelegate().AddUObject( this, &ThisClass::OnAnyGameplayEffectRemoved );
}

void UGASExtAsyncTaskWaitGERemoved::SetReadyToDestroy()
{
    Super::SetReadyToDestroy();

    OnGameplayEffectRemovedDelegate.RemoveAll( this );
}

void UGASExtAsyncTaskWaitGERemoved::OnAnyGameplayEffectRemoved( const FActiveGameplayEffect & active_gameplay_effect )
{
    if ( Query.Matches( active_gameplay_effect ) )
    {
        Broadcast( active_gameplay_effect.Handle );
    }
}

void UGASExtAsyncTaskWaitGERemoved::Broadcast( const FActiveGameplayEffectHandle & active_gameplay_effect_handle )
{
    if ( !OnGameplayEffectRemovedDelegate.IsBound() )
    {
        SetReadyToDestroy();
        return;
    }

    OnGameplayEffectRemovedDelegate.Broadcast( active_gameplay_effect_handle );
}