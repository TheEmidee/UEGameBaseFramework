#include "Gameplay/ConditionalEvents/Triggers/GBFTimePassedTrigger.h"

#include <Engine/World.h>
#include <TimerManager.h>

void UGBFTimePassedTrigger::Activate()
{
    if ( const auto * world = GetWorld() )
    {
        world->GetTimerManager().SetTimer( TimerHandle, this, &ThisClass::OnTimerElapsed, Time );
    }
}

void UGBFTimePassedTrigger::Deactivate()
{
    if ( const auto * world = GetWorld() )
    {
        world->GetTimerManager().ClearTimer( TimerHandle );
    }
}

void UGBFTimePassedTrigger::OnTimerElapsed()
{
    OnTriggeredDelegate.Broadcast();
}
