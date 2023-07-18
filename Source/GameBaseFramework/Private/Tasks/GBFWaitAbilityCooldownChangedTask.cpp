#include "Tasks/GBFWaitAbilityCooldownChangedTask.h"

#include <AbilitySystemComponent.h>

UGBFWaitAbilityCooldownChangedTask * UGBFWaitAbilityCooldownChangedTask::ListenForCooldownChange( UAbilitySystemComponent * ability_system_component, FGameplayTagContainer cooldown_tags, bool use_server_cooldown )
{
    auto * listen_for_cooldown_change = NewObject< UGBFWaitAbilityCooldownChangedTask >();
    listen_for_cooldown_change->ASC = ability_system_component;
    listen_for_cooldown_change->CooldownTags = cooldown_tags;
    listen_for_cooldown_change->bUseServerCooldown = use_server_cooldown;

    if ( !IsValid( ability_system_component ) || cooldown_tags.Num() < 1 )
    {
        listen_for_cooldown_change->EndTask();
        return nullptr;
    }

    ability_system_component->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject( listen_for_cooldown_change, &ThisClass::OnActiveGameplayEffectAddedCallback );

    TArray< FGameplayTag > cooldown_tag_array;
    cooldown_tags.GetGameplayTagArray( cooldown_tag_array );

    for ( const auto cooldown_tag : cooldown_tag_array )
    {
        ability_system_component->RegisterGameplayTagEvent( cooldown_tag, EGameplayTagEventType::NewOrRemoved ).AddUObject( listen_for_cooldown_change, &ThisClass::CooldownTagChanged );
    }

    return listen_for_cooldown_change;
}

void UGBFWaitAbilityCooldownChangedTask::EndTask()
{
    if ( IsValid( ASC ) )
    {
        ASC->OnActiveGameplayEffectAddedDelegateToSelf.RemoveAll( this );

        TArray< FGameplayTag > cooldown_tag_array;
        CooldownTags.GetGameplayTagArray( cooldown_tag_array );

        for ( const auto cooldown_tag : cooldown_tag_array )
        {
            ASC->RegisterGameplayTagEvent( cooldown_tag, EGameplayTagEventType::NewOrRemoved ).RemoveAll( this );
        }
    }

    SetReadyToDestroy();
    MarkAsGarbage();
}

void UGBFWaitAbilityCooldownChangedTask::OnActiveGameplayEffectAddedCallback( UAbilitySystemComponent * target, const FGameplayEffectSpec & spec_applied, FActiveGameplayEffectHandle active_handle )
{
    FGameplayTagContainer asset_tags;
    spec_applied.GetAllAssetTags( asset_tags );

    FGameplayTagContainer granted_tags;
    spec_applied.GetAllGrantedTags( granted_tags );

    TArray< FGameplayTag > cooldown_tag_array;
    CooldownTags.GetGameplayTagArray( cooldown_tag_array );

    for ( auto cooldown_tag : cooldown_tag_array )
    {
        if ( asset_tags.HasTagExact( cooldown_tag ) || granted_tags.HasTagExact( cooldown_tag ) )
        {
            float time_remaining = 0.0f;
            float duration = 0.0f;
            // Expecting cooldown tag to always be first tag
            const FGameplayTagContainer cooldown_tag_container( granted_tags.GetByIndex( 0 ) );
            GetCooldownRemainingForTag( cooldown_tag_container, time_remaining, duration );

            if ( ASC->GetOwnerRole() == ROLE_Authority )
            {
                // Player is Server
                OnCooldownBegin.Broadcast( cooldown_tag, time_remaining, duration );
            }
            else if ( !bUseServerCooldown && spec_applied.GetContext().GetAbilityInstance_NotReplicated() )
            {
                // Client using predicted cooldown
                OnCooldownBegin.Broadcast( cooldown_tag, time_remaining, duration );
            }
            else if ( bUseServerCooldown && spec_applied.GetContext().GetAbilityInstance_NotReplicated() == nullptr )
            {
                // Client using Server's cooldown. This is Server's corrective cooldown GE.
                OnCooldownBegin.Broadcast( cooldown_tag, time_remaining, duration );
            }
            else if ( bUseServerCooldown && spec_applied.GetContext().GetAbilityInstance_NotReplicated() )
            {
                // Client using Server's cooldown but this is predicted cooldown GE.
                // This can be useful to gray out abilities until Server's cooldown comes in.
                OnCooldownBegin.Broadcast( cooldown_tag, -1.0f, -1.0f );
            }
        }
    }
}

void UGBFWaitAbilityCooldownChangedTask::CooldownTagChanged( const FGameplayTag cooldown_tag, const int32 new_count )
{
    if ( new_count == 0 )
    {
        OnCooldownEnd.Broadcast( cooldown_tag, -1.0f, -1.0f );
    }
}

bool UGBFWaitAbilityCooldownChangedTask::GetCooldownRemainingForTag( FGameplayTagContainer cooldown_tags, float & time_remaining, float & cooldown_duration ) const
{
    if ( IsValid( ASC ) && cooldown_tags.Num() > 0 )
    {
        time_remaining = 0.f;
        cooldown_duration = 0.f;

        const auto query = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags( cooldown_tags );
        if ( TArray< TPair< float, float > > duration_and_time_remaining = ASC->GetActiveEffectsTimeRemainingAndDuration( query );
             duration_and_time_remaining.Num() > 0 )
        {
            auto best_idx = 0;
            auto longest_time = duration_and_time_remaining[ 0 ].Key;
            for ( auto index = 1; index < duration_and_time_remaining.Num(); ++index )
            {
                if ( duration_and_time_remaining[ index ].Key > longest_time )
                {
                    longest_time = duration_and_time_remaining[ index ].Key;
                    best_idx = index;
                }
            }

            time_remaining = duration_and_time_remaining[ best_idx ].Key;
            cooldown_duration = duration_and_time_remaining[ best_idx ].Value;

            return true;
        }
    }

    return false;
}
