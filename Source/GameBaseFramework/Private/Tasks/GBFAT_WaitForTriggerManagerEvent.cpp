#include "Tasks/GBFAT_WaitForTriggerManagerEvent.h"

#include "Gameplay/Components/GBFTriggerManagerComponent.h"

UGBFAT_WaitForTriggerManagerEvent * UGBFAT_WaitForTriggerManagerEvent::WaitForTriggerManagerEvent( UGameplayAbility * owning_ability,
    UGBFTriggerManagerComponent * trigger_manager_component,
    const bool broadcast_trigger_count_on_activate,
    const bool trigger_once /*= false*/ )
{
    auto * my_obj = NewAbilityTask< UGBFAT_WaitForTriggerManagerEvent >( owning_ability );
    my_obj->TriggerManagerComponent = trigger_manager_component;
    my_obj->bBroadcastTriggerCountOnActivate = broadcast_trigger_count_on_activate;
    my_obj->bTriggerOnce = trigger_once;
    return my_obj;
}

void UGBFAT_WaitForTriggerManagerEvent::Activate()
{
    Super::Activate();

    SetWaitingOnAvatar();

    if ( TriggerManagerComponent != nullptr )
    {
        TriggerManagerComponent->OnTriggerBoxActivated().AddDynamic( this, &UGBFAT_WaitForTriggerManagerEvent::OnTriggerActivated );
        TriggerManagerComponent->OnActorInsideTriggerCountChanged().AddDynamic( this, &UGBFAT_WaitForTriggerManagerEvent::OnActorInsideTriggerCountChanged );

        if ( bBroadcastTriggerCountOnActivate )
        {
            const auto & actors_in_trigger = TriggerManagerComponent->GetActorsInTrigger();
            const auto actor_count = actors_in_trigger.Num();
            auto * last_actor = actor_count > 0 ? actors_in_trigger.Last() : nullptr;

            OnActorInsideTriggerCountChangedDelegate.Broadcast( last_actor, actor_count );
        }
    }
}

void UGBFAT_WaitForTriggerManagerEvent::OnDestroy( const bool in_owner_finished )
{
    if ( TriggerManagerComponent != nullptr )
    {
        TriggerManagerComponent->OnTriggerBoxActivated().RemoveDynamic( this, &UGBFAT_WaitForTriggerManagerEvent::OnTriggerActivated );
        TriggerManagerComponent->OnActorInsideTriggerCountChanged().RemoveDynamic( this, &UGBFAT_WaitForTriggerManagerEvent::OnActorInsideTriggerCountChanged );
    }

    Super::OnDestroy( in_owner_finished );
}

void UGBFAT_WaitForTriggerManagerEvent::OnTriggerActivated( AActor * activator )
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        OnTriggerActivatedDelegate.Broadcast( activator, TriggerManagerComponent->GetActorsInTrigger().Num() );
        CheckShouldEndTask();
    }
}

void UGBFAT_WaitForTriggerManagerEvent::OnActorInsideTriggerCountChanged( const int actor_count )
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        const auto & actors_in_trigger = TriggerManagerComponent->GetActorsInTrigger();
        auto * last_actor = actors_in_trigger.Num() > 0 ? actors_in_trigger.Last() : nullptr;

        OnActorInsideTriggerCountChangedDelegate.Broadcast( last_actor, actor_count );
        CheckShouldEndTask();
    }
}

void UGBFAT_WaitForTriggerManagerEvent::CheckShouldEndTask()
{
    if ( bTriggerOnce )
    {
        EndTask();
    }
}