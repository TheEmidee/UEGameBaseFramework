#include "Gameplay/ConditionalEvents/GBFConditionalTrigger.h"

#include "Gameplay/Components/GBFTriggerManagerComponent.h"

void UGBFConditionalTrigger::Activate()
{
}

void UGBFConditionalTrigger::Deactivate()
{
}

void UGBFTriggerBoxTrigger::Activate()
{
    if ( !ensureMsgf( !TriggerBoxSoftObject.IsNull(), TEXT( "%s: TriggerBox reference is not valid!" ), TEXT( __FUNCTION__ ) ) )
    {
        return;
    }

    auto * trigger_box = TriggerBoxSoftObject.Get();
    trigger_box->Multicast_ActivateTrigger( true );
    trigger_box->GetTriggerManagerComponent()->OnTriggerBoxActivated().AddDynamic( this, &ThisClass::OnTriggerBoxActivated );
}

void UGBFTriggerBoxTrigger::Deactivate()
{
    if ( !ensureMsgf( !TriggerBoxSoftObject.IsNull(), TEXT( "%s: TriggerBox reference is not valid!" ), TEXT( __FUNCTION__ ) ) )
    {
        return;
    }

    auto * trigger_box = TriggerBoxSoftObject.Get();
    trigger_box->GetTriggerManagerComponent()->OnTriggerBoxActivated().RemoveDynamic( this, &ThisClass::OnTriggerBoxActivated );
    trigger_box->Multicast_DeactivateTrigger();
}

void UGBFTriggerBoxTrigger::OnTriggerBoxActivated( AActor * /*activator*/ )
{
    OnTriggeredDelegate.Broadcast();
}
