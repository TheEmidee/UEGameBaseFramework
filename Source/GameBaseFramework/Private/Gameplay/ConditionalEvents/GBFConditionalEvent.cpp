#include "Gameplay/ConditionalEvents/GBFConditionalEvent.h"

#include "Gameplay/ConditionalEvents/GBFConditionalTrigger.h"

UGBFConditionalEvent::UGBFConditionalEvent()
{
    ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    bServerRespectsRemoteAbilityCancellation = false;
    bRetriggerInstancedAbility = false;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
    NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnly;

    TriggerCount = 0;
}

void UGBFConditionalEvent::ActivateAbility( const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilityActivationInfo activation_info, const FGameplayEventData * trigger_event_data )
{
    Super::ActivateAbility( handle, actor_info, activation_info, trigger_event_data );

    for ( auto * trigger : Triggers )
    {
        trigger->Activate();
        trigger->GetOnTriggeredDelegate().AddDynamic( this, &ThisClass::OnTriggerTriggered );
    }
}

void UGBFConditionalEvent::EndAbility( const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilityActivationInfo activation_info, bool replicate_end_ability, bool was_cancelled )
{
    for ( auto * trigger : Triggers )
    {
        trigger->GetOnTriggeredDelegate().RemoveAll( this );
        trigger->Deactivate();
    }

    Super::EndAbility( handle, actor_info, activation_info, replicate_end_ability, was_cancelled );
}

void UGBFConditionalEvent::OnTriggerTriggered()
{
    ++TriggerCount;
    if ( TriggerCount >= Triggers.Num() )
    {
        ApplyOutcomes();
    }
}
