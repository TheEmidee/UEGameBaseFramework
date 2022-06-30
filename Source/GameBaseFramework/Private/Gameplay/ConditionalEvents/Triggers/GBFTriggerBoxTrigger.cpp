#include "Gameplay/ConditionalEvents/Triggers/GBFTriggerBoxTrigger.h"

#include "Gameplay/Components/GBFTriggerManagerComponent.h"

void UGBFTriggerBoxTrigger::Activate()
{
    if ( !ensureMsgf( TriggerBoxSoftObject.IsValid(), TEXT( "%s: TriggerBox reference is not valid!" ), TEXT( __FUNCTION__ ) ) )
    {
        return;
    }

    const auto * trigger_box = TriggerBoxSoftObject.Get();
    trigger_box->GetTriggerManagerComponent()->OnTriggerBoxActivated().AddDynamic( this, &ThisClass::OnTriggerBoxActivated );
}

void UGBFTriggerBoxTrigger::Deactivate()
{
    if ( !ensureMsgf( TriggerBoxSoftObject.IsValid(), TEXT( "%s: TriggerBox reference is not valid!" ), TEXT( __FUNCTION__ ) ) )
    {
        return;
    }

    const auto * trigger_box = TriggerBoxSoftObject.Get();
    trigger_box->GetTriggerManagerComponent()->OnTriggerBoxActivated().RemoveDynamic( this, &ThisClass::OnTriggerBoxActivated );
}

void UGBFTriggerBoxTrigger::OnTriggerBoxActivated( AActor * /*activator*/ )
{
    OnTriggeredDelegate.Broadcast();
}