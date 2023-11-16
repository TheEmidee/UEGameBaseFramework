#include "GAS/Tasks/GBFAT_WaitTriggerManagerEvent.h"

#include "Triggers/Components/GBFTriggerManagerComponent.h"

UGBFAT_WaitTriggerManagerTriggered * UGBFAT_WaitTriggerManagerTriggered::WaitTriggerManagerTriggered( UGameplayAbility * owning_ability, UGBFTriggerManagerComponent * trigger_manager_component, bool trigger_on_activation, bool trigger_once )
{
    return CreateTask< UGBFAT_WaitTriggerManagerTriggered >( owning_ability, trigger_manager_component, trigger_on_activation, trigger_once );
}

void UGBFAT_WaitTriggerManagerEventBase::Activate()
{
    Super::Activate();

    SetWaitingOnAvatar();

    if ( TriggerManagerComponent != nullptr )
    {
        BindToTriggerManagerEvents();

        if ( bBroadcastOnActivation )
        {
            if ( ShouldBroadcastAbilityTaskDelegates() )
            {
                TryBroadcastEvent();
            }
        }
    }
}

void UGBFAT_WaitTriggerManagerEventBase::OnDestroy( bool in_owner_finished )
{
    Super::OnDestroy( in_owner_finished );

    if ( TriggerManagerComponent != nullptr )
    {
        UnbindFromTriggerManagerEvents();
    }
}

void UGBFAT_WaitTriggerManagerEventBase::CheckShouldEndTask()
{
    if ( bTriggerOnce )
    {
        EndTask();
    }
}

void UGBFAT_WaitTriggerManagerTriggered::BindToTriggerManagerEvents()
{
    TriggerManagerComponent->OnTriggerBoxActivated().AddDynamic( this, &ThisClass::OnTriggerActivated );
}

void UGBFAT_WaitTriggerManagerTriggered::TryBroadcastEvent()
{
    if ( TriggerManagerComponent->IsTriggered() )
    {
        const auto & actors_in_trigger = TriggerManagerComponent->GetActorsInTrigger();
        auto * last_actor = actors_in_trigger.Num() > 0 ? actors_in_trigger.Last() : nullptr;

        OnTriggerTriggeredDelegate.Broadcast( last_actor );

        CheckShouldEndTask();
    }
}

void UGBFAT_WaitTriggerManagerTriggered::UnbindFromTriggerManagerEvents()
{
    TriggerManagerComponent->OnTriggerBoxActivated().RemoveDynamic( this, &ThisClass::OnTriggerActivated );
}

void UGBFAT_WaitTriggerManagerTriggered::OnTriggerActivated( UGBFTriggerManagerComponent * /*component*/, AActor * activator )
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        OnTriggerTriggeredDelegate.Broadcast( activator );
        CheckShouldEndTask();
    }
}

UGBFAT_WaitTriggerManagerActorsInsideChange * UGBFAT_WaitTriggerManagerActorsInsideChange::WaitTriggerManagerActorsInsideCountChanged( UGameplayAbility * owning_ability, UGBFTriggerManagerComponent * trigger_manager_component, bool trigger_on_activation, bool trigger_once )
{
    return CreateTask< UGBFAT_WaitTriggerManagerActorsInsideChange >( owning_ability, trigger_manager_component, trigger_on_activation, trigger_once );
}

void UGBFAT_WaitTriggerManagerActorsInsideChange::BindToTriggerManagerEvents()
{
    TriggerManagerComponent->OnActorInsideTriggerCountChanged().AddDynamic( this, &ThisClass::OnActorInsideTriggerCountChanged );
}

void UGBFAT_WaitTriggerManagerActorsInsideChange::TryBroadcastEvent()
{
    const auto & actors_in_trigger = TriggerManagerComponent->GetActorsInTrigger();
    const auto actor_count = actors_in_trigger.Num();

    OnActorInsideCountChangedDelegate.Broadcast( actor_count );
}

void UGBFAT_WaitTriggerManagerActorsInsideChange::UnbindFromTriggerManagerEvents()
{
    TriggerManagerComponent->OnActorInsideTriggerCountChanged().RemoveDynamic( this, &ThisClass::OnActorInsideTriggerCountChanged );
}

void UGBFAT_WaitTriggerManagerActorsInsideChange::OnActorInsideTriggerCountChanged( int actor_count )
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        OnActorInsideCountChangedDelegate.Broadcast( actor_count );
        CheckShouldEndTask();
    }
}

UGBFAT_WaitTriggerManagerActorOverlapStatusChange * UGBFAT_WaitTriggerManagerActorOverlapStatusChange::WaitTriggerManagerActorOverlapStatusChanged( UGameplayAbility * owning_ability, UGBFTriggerManagerComponent * trigger_manager_component, bool trigger_on_activation, bool trigger_once )
{
    return CreateTask< UGBFAT_WaitTriggerManagerActorOverlapStatusChange >( owning_ability, trigger_manager_component, trigger_on_activation, trigger_once );
}

void UGBFAT_WaitTriggerManagerActorOverlapStatusChange::BindToTriggerManagerEvents()
{
    TriggerManagerComponent->OnActorOverlapStatusChanged().AddDynamic( this, &ThisClass::OnActorOverlapStatusChanged );
}

void UGBFAT_WaitTriggerManagerActorOverlapStatusChange::TryBroadcastEvent()
{
    const auto & actors_in_trigger = TriggerManagerComponent->GetActorsInTrigger();

    for ( auto * actor_in_trigger : actors_in_trigger )
    {
        OnActorOverlapStatusChangedDelegate.Broadcast( actor_in_trigger, true );
    }
}

void UGBFAT_WaitTriggerManagerActorOverlapStatusChange::UnbindFromTriggerManagerEvents()
{
    TriggerManagerComponent->OnActorOverlapStatusChanged().RemoveDynamic( this, &ThisClass::OnActorOverlapStatusChanged );
}

void UGBFAT_WaitTriggerManagerActorOverlapStatusChange::OnActorOverlapStatusChanged( AActor * actor, bool is_inside )
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        OnActorOverlapStatusChangedDelegate.Broadcast( actor, is_inside );
        CheckShouldEndTask();
    }
}
