#include "GAS/Abilities/GBFGameplayAbility_Death.h"

#include "Characters/Components/GBFHealthComponent.h"
#include "GBFLog.h"

#include <AbilitySystemComponent.h>

UGBFGameplayAbility_Death::UGBFGameplayAbility_Death()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

    bAutoStartDeath = true;

    // You may want in your subclass to set the trigger tag and add a specific tag to this ability
    /*
    FAbilityTriggerData trigger_data;
    trigger_data.TriggerTag = Event_Shared_Death;
    trigger_data.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
    AbilityTriggers.Add( trigger_data );

    AbilityTags.AddTag( Ability_Type_StatusChange_Death );
    */
}

void UGBFGameplayAbility_Death::ActivateAbility( const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilityActivationInfo activation_info, const FGameplayEventData * trigger_event_data )
{
    check( actor_info );

    auto * asc = CastChecked< UAbilitySystemComponent >( actor_info->AbilitySystemComponent.Get() );

    /* You may want to cancel abilities that may survive death
     *
    FGameplayTagContainer ability_types_to_ignore;
    ability_types_to_ignore.AddTag( Ability_Behavior_SurvivesDeath );

    asc->CancelAbilities( nullptr, &ability_types_to_ignore, this );
    */

    SetCanBeCanceled( false );

    if ( !ChangeActivationGroup( EGBFAbilityActivationGroup::ExclusiveBlocking ) )
    {
        UE_LOG( LogGBF, Error, TEXT( "UGBFGameplayAbility_Death::ActivateAbility: Ability [%s] failed to change activation group to blocking." ), *GetName() );
    }

    if ( bAutoStartDeath )
    {
        StartDeath();
    }

    Super::ActivateAbility( handle, actor_info, activation_info, trigger_event_data );
}

void UGBFGameplayAbility_Death::EndAbility( const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilityActivationInfo activation_info, const bool replicate_end_ability, const bool was_cancelled )
{
    check( actor_info );

    // Always try to finish the death when the ability ends in case the ability doesn't.
    // This won't do anything if the death hasn't been started.
    FinishDeath();

    Super::EndAbility( handle, actor_info, activation_info, replicate_end_ability, was_cancelled );
}

void UGBFGameplayAbility_Death::StartDeath()
{
    if ( auto * health_component = UGBFHealthComponent::FindHealthComponent( GetAvatarActorFromActorInfo() ) )
    {
        if ( health_component->GetDeathState() == EGBFDeathState::NotDead )
        {
            health_component->StartDeath();
        }
    }
}

void UGBFGameplayAbility_Death::FinishDeath()
{
    if ( auto * health_component = UGBFHealthComponent::FindHealthComponent( GetAvatarActorFromActorInfo() ) )
    {
        if ( health_component->GetDeathState() == EGBFDeathState::DeathStarted )
        {
            health_component->FinishDeath();
        }
    }
}
