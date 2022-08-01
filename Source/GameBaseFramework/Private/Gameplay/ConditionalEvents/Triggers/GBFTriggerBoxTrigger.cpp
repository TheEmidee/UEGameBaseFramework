#include "Gameplay/ConditionalEvents/Triggers/GBFTriggerBoxTrigger.h"

#include "DVEDataValidator.h"
#include "Gameplay/Components/GBFTriggerManagerComponent.h"

void UGBFTriggerBoxTrigger::Activate()
{
    if ( !ensureMsgf( TriggerBoxSoftObject.IsValid(), TEXT( "%s: TriggerBox reference is not valid!" ), StringCast< TCHAR >( __FUNCTION__ ).Get() ) )
    {
        return;
    }

    const auto * trigger_box = TriggerBoxSoftObject.Get();
    trigger_box->GetTriggerManagerComponent()->OnTriggerBoxActivated().AddDynamic( this, &ThisClass::OnTriggerBoxActivated );
}

void UGBFTriggerBoxTrigger::Deactivate()
{
    if ( !TriggerBoxSoftObject.IsValid() )
    {
        return;
    }

    const auto * trigger_box = TriggerBoxSoftObject.Get();
    trigger_box->GetTriggerManagerComponent()->OnTriggerBoxActivated().RemoveDynamic( this, &ThisClass::OnTriggerBoxActivated );
}

#if WITH_EDITOR
EDataValidationResult UGBFTriggerBoxTrigger::IsDataValid( TArray< FText > & validation_errors )
{
    Super::IsDataValid( validation_errors );

    return FDVEDataValidator( validation_errors )
        .NotNull( VALIDATOR_GET_PROPERTY( TriggerBoxSoftObject ) )
        .Result();
}
#endif

void UGBFTriggerBoxTrigger::OnTriggerBoxActivated( AActor * /*activator*/ )
{
    OnTriggeredDelegate.Broadcast();
}
