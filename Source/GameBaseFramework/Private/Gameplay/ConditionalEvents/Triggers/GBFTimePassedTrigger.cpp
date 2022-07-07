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
EDataValidationResult UGBFTimePassedTrigger::IsDataValid( TArray< FText > & validation_errors )
{
    Super::IsDataValid( validation_errors );

    return FDVEDataValidator( validation_errors )
        .IsGreaterThan( VALIDATOR_GET_PROPERTY( Time ), 0.0f )
        .Result();
}
#endif

void UGBFTimePassedTrigger::OnTimerElapsed()
{
    OnTriggeredDelegate.Broadcast();
}
