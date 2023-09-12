#include "Gameplay/ConditionalEvents/Triggers/GBFTimePassedTrigger.h"

#include "DVEDataValidator.h"

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

#if WITH_EDITOR
EDataValidationResult UGBFTimePassedTrigger::IsDataValid( FDataValidationContext & context ) const
{
    Super::IsDataValid( context );

    return FDVEDataValidator( context )
        .IsGreaterThan( VALIDATOR_GET_PROPERTY( Time ), 0.0f )
        .Result();
}
#endif

void UGBFTimePassedTrigger::OnTimerElapsed()
{
    OnTriggeredDelegate.Broadcast( this, true );
}
