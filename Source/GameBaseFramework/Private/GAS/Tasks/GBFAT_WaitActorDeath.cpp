#include "GAS/Tasks/GBFAT_WaitActorDeath.h"

UGBFAT_WaitActorDeath * UGBFAT_WaitActorDeath::WaitActorDeath( UGameplayAbility * owning_ability, AActor * actor, EGBFDeathState death_state )
{
    return WaitActorDeathWithComponent( owning_ability, UGBFHealthComponent::FindHealthComponent( actor ), death_state );
}

UGBFAT_WaitActorDeath * UGBFAT_WaitActorDeath::WaitActorDeathWithComponent( UGameplayAbility * owning_ability, UGBFHealthComponent * health_component, EGBFDeathState death_state )
{
    auto * my_obj = NewAbilityTask< UGBFAT_WaitActorDeath >( owning_ability );
    my_obj->HealthComponent = health_component;
    my_obj->DeathState = death_state;
    return my_obj;
}

void UGBFAT_WaitActorDeath::Activate()
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
            }
            break;
        }
    }
}

void UGBFAT_WaitActorDeath::OnDestroy( bool ability_ended )
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
            }
            break;
        }
    }
}

void UGBFAT_WaitActorDeath::OnDeathEvent( AActor * owning_actor )
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        OnActorDeathDelegate.Broadcast( owning_actor );
    }

    EndTask();
}
