#include "GAS/BlueprintAsyncActions/GBFAsyncTaskWaitGERemoved.h"

#include <AbilitySystemComponent.h>

UGBFAsyncTaskWaitGERemoved * UGBFAsyncTaskWaitGERemoved::WaitGameplayEffectRemovedWithTags( UAbilitySystemComponent * ability_system_component, FGameplayTagContainer gameplay_effect_owned_tags, bool trigger_if_no_initial_matching_effect /*= true*/ )
{
    return WaitGameplayEffectRemovedWithQuery( ability_system_component, FGameplayEffectQuery::MakeQuery_MatchAllEffectTags( gameplay_effect_owned_tags ), trigger_if_no_initial_matching_effect );
}

UGBFAsyncTaskWaitGERemoved * UGBFAsyncTaskWaitGERemoved::WaitGameplayEffectRemovedWithQuery( UAbilitySystemComponent * ability_system_component, FGameplayEffectQuery query, bool trigger_if_no_initial_matching_effect /*= true*/ )
{
    auto * result = NewObject< UGBFAsyncTaskWaitGERemoved >();
    result->ASC = ability_system_component;
    result->Query = query;
    result->bTriggerIfNoInitialMatchingGameplayEffect = trigger_if_no_initial_matching_effect;
    return result;
}

void UGBFAsyncTaskWaitGERemoved::Activate()
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

void UGBFAsyncTaskWaitGERemoved::SetReadyToDestroy()
{
    Super::SetReadyToDestroy();

    OnGameplayEffectRemovedDelegate.RemoveAll( this );
}

void UGBFAsyncTaskWaitGERemoved::OnAnyGameplayEffectRemoved( const FActiveGameplayEffect & active_gameplay_effect )
{
    if ( Query.Matches( active_gameplay_effect ) )
    {
        Broadcast( active_gameplay_effect.Handle );
    }
}

void UGBFAsyncTaskWaitGERemoved::Broadcast( const FActiveGameplayEffectHandle & active_gameplay_effect_handle )
{
    if ( !OnGameplayEffectRemovedDelegate.IsBound() )
    {
        SetReadyToDestroy();
        return;
    }

    OnGameplayEffectRemovedDelegate.Broadcast( active_gameplay_effect_handle );
}