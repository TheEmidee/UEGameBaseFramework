#include "Tasks/GBFWaitForTriggerManagerEvent.h"

#include "Gameplay/Components/GBFTriggerManagerComponent.h"

UGBFWaitForTriggerManagerEvent * UGBFWaitForTriggerManagerEvent::WaitForTriggerManagerEvent( UObject * world_context_object,
    UGBFTriggerManagerComponent * trigger_manager_component,
    const bool broadcast_trigger_count_on_activate,
    const bool trigger_once )
{
    auto * my_obj = NewObject< UGBFWaitForTriggerManagerEvent >();
    my_obj->TriggerManagerComponent = trigger_manager_component;
    my_obj->bBroadcastTriggerCountOnActivate = broadcast_trigger_count_on_activate;
    my_obj->bTriggerOnce = trigger_once;
    return my_obj;
}

void UGBFWaitForTriggerManagerEvent::Activate()
{
    Super::Activate();

    if ( TriggerManagerComponent == nullptr )
    {
        SetReadyToDestroy();
        return;
    }

    TriggerManagerComponent->OnTriggerBoxActivated().AddDynamic( this, &ThisClass::OnTriggerActivated );
    TriggerManagerComponent->OnActorInsideTriggerCountChanged().AddDynamic( this, &ThisClass::OnActorInsideTriggerCountChanged );

    if ( bBroadcastTriggerCountOnActivate )
    {
        const auto & actors_in_trigger = TriggerManagerComponent->GetActorsInTrigger();
        const auto actor_count = actors_in_trigger.Num();
        auto * last_actor = actor_count > 0 ? actors_in_trigger.Last() : nullptr;

        OnActorInsideTriggerCountChangedDelegate.Broadcast( last_actor, actor_count );
    }
}

void UGBFWaitForTriggerManagerEvent::SetReadyToDestroy()
{
    if ( TriggerManagerComponent != nullptr )
    {
        TriggerManagerComponent->OnTriggerBoxActivated().RemoveDynamic( this, &ThisClass::OnTriggerActivated );
        TriggerManagerComponent->OnActorInsideTriggerCountChanged().RemoveDynamic( this, &ThisClass::OnActorInsideTriggerCountChanged );
    }

    Super::SetReadyToDestroy();
}

void UGBFWaitForTriggerManagerEvent::OnTriggerActivated( AActor * activator )
{
    OnTriggerActivatedDelegate.Broadcast( activator, TriggerManagerComponent->GetActorsInTrigger().Num() );
    CheckShouldEndTask();
}

void UGBFWaitForTriggerManagerEvent::OnActorInsideTriggerCountChanged( const int actor_count )
{
    const auto & actors_in_trigger = TriggerManagerComponent->GetActorsInTrigger();
    auto * last_actor = actors_in_trigger.Num() > 0 ? actors_in_trigger.Last() : nullptr;

    OnActorInsideTriggerCountChangedDelegate.Broadcast( last_actor, actor_count );
}

void UGBFWaitForTriggerManagerEvent::CheckShouldEndTask()
{
    if ( bTriggerOnce )
    {
        SetReadyToDestroy();
    }
}