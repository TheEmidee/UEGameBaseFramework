#include "Abilities/Tasks/GBFAbilityTask_WaitActorDeath.h"

UGBFAbilityTask_WaitActorDeath * UGBFAbilityTask_WaitActorDeath::WaitActorDeath( UGameplayAbility * owning_ability, AActor * actor, EGBFDeathState death_state )
{
    return WaitActorDeathWithComponent( owning_ability, UGBFHealthComponent::FindHealthComponent( actor ), death_state );
}

UGBFAbilityTask_WaitActorDeath * UGBFAbilityTask_WaitActorDeath::WaitActorDeathWithComponent( UGameplayAbility * owning_ability, UGBFHealthComponent * health_component, EGBFDeathState death_state )
{
    auto * my_obj = NewAbilityTask< UGBFAbilityTask_WaitActorDeath >( owning_ability );
    my_obj->HealthComponent = health_component;
    my_obj->DeathState = death_state;
    return my_obj;
}

void UGBFAbilityTask_WaitActorDeath::Activate()
{
    Super::Activate();

    SetWaitingOnAvatar();

    if ( HealthComponent != nullptr )
    {
        switch ( DeathState )
        {
            case EGBFDeathState::DeathStarted:
            {
                HealthComponent->OnDeathStarted().AddDynamic( this, &ThisClass::OnDeathEvent );
            }
            break;
            case EGBFDeathState::DeathFinished:
            {
                HealthComponent->OnDeathFinished().AddDynamic( this, &ThisClass::OnDeathEvent );
            }
            break;
            default:
            {
                checkNoEntry();
            };
        }
    }
}

void UGBFAbilityTask_WaitActorDeath::OnDestroy( bool ability_ended )
{
    Super::OnDestroy( ability_ended );

    if ( HealthComponent != nullptr )
    {
        switch ( DeathState )
        {
            case EGBFDeathState::DeathStarted:
            {
                HealthComponent->OnDeathStarted().RemoveDynamic( this, &ThisClass::OnDeathEvent );
            }
            break;
            case EGBFDeathState::DeathFinished:
            {
                HealthComponent->OnDeathFinished().RemoveDynamic( this, &ThisClass::OnDeathEvent );
            }
            break;
            default:
            {
                checkNoEntry();
            };
        }
    }
}

void UGBFAbilityTask_WaitActorDeath::OnDeathEvent( AActor * owning_actor )
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        OnActorDeathDelegate.Broadcast( owning_actor );
    }

    EndTask();
}
