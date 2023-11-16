#include "ConditionalEvents/Triggers/GBFTriggerBoxTrigger.h"

#include "DVEDataValidator.h"
#include "Triggers/Components/GBFTriggerManagerComponent.h"

void UGBFTriggerBoxTrigger::Activate()
{
    if ( !ensureMsgf( TriggerBoxSoftObject.IsValid(), TEXT( "%s: TriggerBox reference is not valid!" ), StringCast< TCHAR >( __FUNCTION__ ).Get() ) )
    {
        return;
    }

    if ( const auto * trigger_box = TriggerBoxSoftObject.Get() )
    {
        if ( auto * trigger_manager = trigger_box->GetTriggerManagerComponent() )
        {
            trigger_manager->OnActorInsideTriggerCountChanged().AddDynamic( this, &ThisClass::OnActorsInsideTriggerCountChanged );
        }
    }
}

void UGBFTriggerBoxTrigger::Deactivate()
{
    if ( !TriggerBoxSoftObject.IsValid() )
    {
        return;
    }

    if ( const auto * trigger_box = TriggerBoxSoftObject.Get() )
    {
        if ( auto * trigger_manager = trigger_box->GetTriggerManagerComponent() )
        {
            trigger_manager->OnActorInsideTriggerCountChanged().RemoveDynamic( this, &ThisClass::OnActorsInsideTriggerCountChanged );
        }
    }
}

#if WITH_EDITOR
EDataValidationResult UGBFTriggerBoxTrigger::IsDataValid( FDataValidationContext & context ) const
{
    Super::IsDataValid( context );

    return FDVEDataValidator( context )
        .NotNull( VALIDATOR_GET_PROPERTY( TriggerBoxSoftObject ) )
        .Result();
}
#endif

void UGBFTriggerBoxTrigger::OnActorsInsideTriggerCountChanged( int count )
{
    if ( const auto * trigger_box = TriggerBoxSoftObject.Get() )
    {
        if ( const auto * trigger_manager = trigger_box->GetTriggerManagerComponent() )
        {
            OnTriggeredDelegate.Broadcast( this, count >= trigger_manager->GetExpectedActorCount() );
        }
    }
}
