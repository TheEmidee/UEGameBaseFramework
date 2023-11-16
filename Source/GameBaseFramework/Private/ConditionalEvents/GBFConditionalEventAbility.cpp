#include "ConditionalEvents/GBFConditionalEventAbility.h"

#include "ConditionalEvents/GBFConditionalTrigger.h"
#include "DVEDataValidator.h"

UGBFConditionalEventAbility::UGBFConditionalEventAbility()
{
    ReplicationPolicy = EGameplayAbilityReplicationPolicy::ReplicateNo;
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    bServerRespectsRemoteAbilityCancellation = false;
    bRetriggerInstancedAbility = false;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
    NetSecurityPolicy = EGameplayAbilityNetSecurityPolicy::ServerOnly;

    bEndAbilityAfterOutcomes = true;
    bExecuteOutcomesOnlyOnce = true;
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
    DeactivateTriggers();

    Super::EndAbility( handle, actor_info, activation_info, replicate_end_ability, was_cancelled );
}

#if WITH_EDITOR
EDataValidationResult UGBFConditionalEventAbility::IsDataValid( FDataValidationContext & context ) const
{
    const auto result = Super::IsDataValid( context );

    if ( GetClass()->HasAllClassFlags( CLASS_Abstract ) )
    {
        return result;
    }

    // Call IsDataValid on all triggers since they are created inline
    for ( const auto * trigger : Triggers )
    {
        if ( trigger != nullptr )
        {
            trigger->IsDataValid( context );
        }
    }

    return FDVEDataValidator( context )
        .NoNullItem( VALIDATOR_GET_PROPERTY( Triggers ) )
        .NotEmpty( VALIDATOR_GET_PROPERTY( Triggers ) )
        .Result();
}
#endif

void UGBFConditionalEventAbility::OnTriggersTriggered( UGBFConditionalTrigger * trigger, bool triggered )
{
    if ( triggered )
    {
        TriggeredTriggers.AddUnique( trigger );

        if ( TriggeredTriggers.Num() >= Triggers.Num() )
        {
            ExecuteOutcomes();

            if ( bEndAbilityAfterOutcomes )
            {
                K2_EndAbility();
            }

            if ( bExecuteOutcomesOnlyOnce )
            {
                DeactivateTriggers();
            }
        }

        return;
    }

    TriggeredTriggers.Remove( trigger );
}

void UGBFConditionalEventAbility::DeactivateTriggers()
{
    for ( auto * trigger : Triggers )
    {
        trigger->GetOnTriggeredDelegate().RemoveAll( this );
        trigger->Deactivate();
    }
}