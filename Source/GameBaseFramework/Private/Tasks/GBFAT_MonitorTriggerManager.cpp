#include "Tasks/GBFAT_MonitorTriggerManager.h"

#include "Gameplay/Components/GBFTriggerManagerComponent.h"

UGBFAT_MonitorTriggerManager * UGBFAT_MonitorTriggerManager::MonitorTriggerManager( UGameplayAbility * owning_ability, UGBFTriggerManagerComponent * trigger_manager_component )
{
    auto * my_obj = NewAbilityTask< UGBFAT_MonitorTriggerManager >( owning_ability );
    my_obj->TriggerManagerComponent = trigger_manager_component;
    return my_obj;
}

void UGBFAT_MonitorTriggerManager::Activate()
{
    Super::Activate();

    SetWaitingOnAvatar();

    if ( TriggerManagerComponent != nullptr )
    {
        TriggerManagerComponent->OnTriggerBoxActivated().AddDynamic( this, &UGBFAT_MonitorTriggerManager::OnTriggerActivated );
        TriggerManagerComponent->OnActorInsideTriggerCountChanged().AddDynamic( this, &UGBFAT_MonitorTriggerManager::OnActorInsideTriggerCountChanged );
    }
}

void UGBFAT_MonitorTriggerManager::OnDestroy( bool bInOwnerFinished )
{
    if ( TriggerManagerComponent != nullptr )
    {
        TriggerManagerComponent->OnTriggerBoxActivated().RemoveDynamic( this, &UGBFAT_MonitorTriggerManager::OnTriggerActivated );
        TriggerManagerComponent->OnActorInsideTriggerCountChanged().RemoveDynamic( this, &UGBFAT_MonitorTriggerManager::OnActorInsideTriggerCountChanged );
    }

    Super::OnDestroy( bInOwnerFinished );
}

void UGBFAT_MonitorTriggerManager::OnTriggerActivated( AActor * activator )
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        OnTriggerActivatedDelegate.Broadcast( activator, TriggerManagerComponent->GetActorsInTrigger().Num() );
    }
}

void UGBFAT_MonitorTriggerManager::OnActorInsideTriggerCountChanged( int actor_count )
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        const auto actors_in_trigger = TriggerManagerComponent->GetActorsInTrigger();
        const auto last_actor = actors_in_trigger.Num() > 0 ? actors_in_trigger.Last() : nullptr;

        OnActorInsideTriggerCountChangedDelegate.Broadcast( last_actor, actor_count );
    }
}
