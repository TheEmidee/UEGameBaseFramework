#include "Gameplay/ConditionalEvents/GBFConditionalEventAbility.h"

#include "DVEDataValidator.h"
#include "Gameplay/ConditionalEvents/GBFConditionalTrigger.h"

UGBFConditionalEventAbility::UGBFConditionalEventAbility()
{
    ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    bServerRespectsRemoteAbilityCancellation = false;
    bRetriggerInstancedAbility = false;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
    NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnly;

    bEndAbilityAfterOutcomes = true;
    TriggerCount = 0;
}

void UGBFConditionalEventAbility::ActivateAbility( const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilityActivationInfo activation_info, const FGameplayEventData * trigger_event_data )
{
    Super::ActivateAbility( handle, actor_info, activation_info, trigger_event_data );

    for ( auto * trigger : Triggers )
    {
        trigger->Activate();
        trigger->GetOnTriggeredDelegate().AddDynamic( this, &ThisClass::OnTriggersTriggered );
    }
}

void UGBFConditionalEventAbility::EndAbility( const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilityActivationInfo activation_info, bool replicate_end_ability, bool was_cancelled )
{
    for ( auto * trigger : Triggers )
    {
        trigger->GetOnTriggeredDelegate().RemoveAll( this );
        trigger->Deactivate();
    }

    Super::EndAbility( handle, actor_info, activation_info, replicate_end_ability, was_cancelled );
}

#if WITH_EDITOR
EDataValidationResult UGBFConditionalEventAbility::IsDataValid( TArray< FText > & validation_errors )
{
    const auto result = Super::IsDataValid( validation_errors );

    if ( GetClass()->HasAllClassFlags( CLASS_Abstract ) )
    {
        return result;
    }

    // Call IsDataValid on all triggers since they are created inline
    for ( auto * trigger : Triggers )
    {
        if ( trigger != nullptr )
        {
            trigger->IsDataValid( validation_errors );
        }
    }

    return FDVEDataValidator( validation_errors )
        .NoNullItem( VALIDATOR_GET_PROPERTY( Triggers ) )
        .NotEmpty( VALIDATOR_GET_PROPERTY( Triggers ) )
        .Result();
}
#endif

void UGBFConditionalEventAbility::OnTriggersTriggered()
{
    ++TriggerCount;
    if ( TriggerCount >= Triggers.Num() )
    {
        ExecuteOutcomes();

        if ( bEndAbilityAfterOutcomes )
        {
            K2_EndAbility();
        }
    }
}
