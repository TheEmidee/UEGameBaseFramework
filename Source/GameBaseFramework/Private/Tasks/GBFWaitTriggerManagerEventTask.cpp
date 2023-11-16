#include "Tasks/GBFWaitTriggerManagerEventTask.h"

#include "Triggers/Components/GBFTriggerManagerComponent.h"

void UGBFWaitTriggerManagerEventTaskBase::Activate()
{
    Super::Activate();

    if ( TriggerManagerComponent == nullptr )
    {
        SetReadyToDestroy();
        return;
    }

    BindToTriggerManagerEvents();

    if ( bBroadcastOnActivation )
    {
        TryBroadcastEvent();
    }
}

void UGBFWaitTriggerManagerEventTaskBase::SetReadyToDestroy()
{
    if ( TriggerManagerComponent != nullptr )
    {
        UnbindFromTriggerManagerEvents();
    }

    Super::SetReadyToDestroy();
}

void UGBFWaitTriggerManagerEventTaskBase::CheckShouldEndTask()
{
    if ( bTriggerOnce )
    {
        SetReadyToDestroy();
    }
}

UGBFWaitTriggerManagerTriggeredTask * UGBFWaitTriggerManagerTriggeredTask::WaitForTriggerManagerTriggered( UObject * world_context_object, UGBFTriggerManagerComponent * trigger_manager_component, bool broadcast_on_activation, bool trigger_once )
{
    return CreateTask< UGBFWaitTriggerManagerTriggeredTask >( world_context_object, trigger_manager_component, broadcast_on_activation, trigger_once );
}

void UGBFWaitTriggerManagerTriggeredTask::BindToTriggerManagerEvents()
{
    TriggerManagerComponent->OnTriggerBoxActivated().AddDynamic( this, &ThisClass::OnTriggerActivated );
}

void UGBFWaitTriggerManagerTriggeredTask::UnbindFromTriggerManagerEvents()
{
    TriggerManagerComponent->OnTriggerBoxActivated().RemoveDynamic( this, &ThisClass::OnTriggerActivated );
}

void UGBFWaitTriggerManagerTriggeredTask::TryBroadcastEvent()
{
    if ( TriggerManagerComponent->IsTriggered() )
    {
        const auto & actors_in_trigger = TriggerManagerComponent->GetActorsInTrigger();
        auto * last_actor = actors_in_trigger.Num() > 0 ? actors_in_trigger.Last() : nullptr;

        OnTriggerTriggeredDelegate.Broadcast( last_actor );

        CheckShouldEndTask();
    }
}

void UGBFWaitTriggerManagerTriggeredTask::OnTriggerActivated( UGBFTriggerManagerComponent * /*component*/, AActor * activator )
{
    OnTriggerTriggeredDelegate.Broadcast( activator );
    CheckShouldEndTask();
}

UGBFWaitTriggerManagerActorsInsideCountChangedTask * UGBFWaitTriggerManagerActorsInsideCountChangedTask::WaitForTriggerManagerActorsInsideCountChanged( UObject * world_context_object, UGBFTriggerManagerComponent * trigger_manager_component, bool broadcast_on_activation, bool trigger_once )
{
    return CreateTask< UGBFWaitTriggerManagerActorsInsideCountChangedTask >( world_context_object, trigger_manager_component, broadcast_on_activation, trigger_once );
}

void UGBFWaitTriggerManagerActorsInsideCountChangedTask::BindToTriggerManagerEvents()
{
    TriggerManagerComponent->OnActorInsideTriggerCountChanged().AddDynamic( this, &ThisClass::OnActorInsideTriggerCountChanged );
}

void UGBFWaitTriggerManagerActorsInsideCountChangedTask::UnbindFromTriggerManagerEvents()
{
    TriggerManagerComponent->OnActorInsideTriggerCountChanged().RemoveDynamic( this, &ThisClass::OnActorInsideTriggerCountChanged );
}

void UGBFWaitTriggerManagerActorsInsideCountChangedTask::TryBroadcastEvent()
{
    const auto & actors_in_trigger = TriggerManagerComponent->GetActorsInTrigger();
    const auto actor_count = actors_in_trigger.Num();

    OnActorInsideCountChangedDelegate.Broadcast( actor_count );
}

void UGBFWaitTriggerManagerActorsInsideCountChangedTask::OnActorInsideTriggerCountChanged( int actor_count )
{
    OnActorInsideCountChangedDelegate.Broadcast( actor_count );
    CheckShouldEndTask();
}

UGBFWaitTriggerManagerActorOverlapStatusChangedTask * UGBFWaitTriggerManagerActorOverlapStatusChangedTask::WaitForTriggerManagerActorOverlapStatusChanged( UObject * world_context_object, UGBFTriggerManagerComponent * trigger_manager_component, bool broadcast_on_activation, bool trigger_once )
{
    return CreateTask< UGBFWaitTriggerManagerActorOverlapStatusChangedTask >( world_context_object, trigger_manager_component, broadcast_on_activation, trigger_once );
}

void UGBFWaitTriggerManagerActorOverlapStatusChangedTask::BindToTriggerManagerEvents()
{
    TriggerManagerComponent->OnActorOverlapStatusChanged().AddDynamic( this, &ThisClass::OnActorOverlapStatusChanged );
}

void UGBFWaitTriggerManagerActorOverlapStatusChangedTask::UnbindFromTriggerManagerEvents()
{
    TriggerManagerComponent->OnActorOverlapStatusChanged().RemoveDynamic( this, &ThisClass::OnActorOverlapStatusChanged );
}

void UGBFWaitTriggerManagerActorOverlapStatusChangedTask::TryBroadcastEvent()
{
    const auto & actors_in_trigger = TriggerManagerComponent->GetActorsInTrigger();

    for ( auto * actor_in_trigger : actors_in_trigger )
    {
        OnActorOverlapStatusChangedDelegate.Broadcast( actor_in_trigger, true );
    }
}

void UGBFWaitTriggerManagerActorOverlapStatusChangedTask::OnActorOverlapStatusChanged( AActor * actor, bool is_inside_trigger )
{
    OnActorOverlapStatusChangedDelegate.Broadcast( actor, is_inside_trigger );
    CheckShouldEndTask();
}
