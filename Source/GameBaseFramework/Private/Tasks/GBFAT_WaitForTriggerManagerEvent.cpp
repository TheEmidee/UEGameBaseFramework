﻿#include "Tasks/GBFAT_WaitForTriggerManagerEvent.h"

#include "Gameplay/Components/GBFTriggerManagerComponent.h"

UGBFAT_WaitForTriggerManagerEvent * UGBFAT_WaitForTriggerManagerEvent::WaitForTriggerManagerEvent( UGameplayAbility * owning_ability,
    UGBFTriggerManagerComponent * trigger_manager_component,
    bool broadcast_trigger_count_on_activate )
{
    auto * my_obj = NewAbilityTask< UGBFAT_WaitForTriggerManagerEvent >( owning_ability );
    my_obj->TriggerManagerComponent = trigger_manager_component;
    my_obj->bBroadcastTriggerCountOnActivate = broadcast_trigger_count_on_activate;
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

void UGBFAT_WaitForTriggerManagerEvent::OnDestroy( bool bInOwnerFinished )
{
    if ( TriggerManagerComponent != nullptr )
    {
        TriggerManagerComponent->OnTriggerBoxActivated().RemoveDynamic( this, &UGBFAT_WaitForTriggerManagerEvent::OnTriggerActivated );
        TriggerManagerComponent->OnActorInsideTriggerCountChanged().RemoveDynamic( this, &UGBFAT_WaitForTriggerManagerEvent::OnActorInsideTriggerCountChanged );
    }

    Super::OnDestroy( bInOwnerFinished );
}

void UGBFAT_WaitForTriggerManagerEvent::OnTriggerActivated( AActor * activator )
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        OnTriggerActivatedDelegate.Broadcast( activator, TriggerManagerComponent->GetActorsInTrigger().Num() );
    }
}

void UGBFAT_WaitForTriggerManagerEvent::OnActorInsideTriggerCountChanged( int actor_count )
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        const auto & actors_in_trigger = TriggerManagerComponent->GetActorsInTrigger();
        auto * last_actor = actors_in_trigger.Num() > 0 ? actors_in_trigger.Last() : nullptr;

        OnActorInsideTriggerCountChangedDelegate.Broadcast( last_actor, actor_count );
    }
}
