#include "GAS/Tasks/GBFAT_ServerWaitClientTargetData.h"

#include <AbilitySystemComponent.h>

UGBFAT_ServerWaitClientTargetData * UGBFAT_ServerWaitClientTargetData::ServerWaitForClientTargetData( UGameplayAbility * owning_ability, const FName task_instance_name, const bool trigger_once )
{
    auto * my_obj = NewAbilityTask< UGBFAT_ServerWaitClientTargetData >( owning_ability, task_instance_name );
    my_obj->TriggerOnce = trigger_once;
    return my_obj;
}

void UGBFAT_ServerWaitClientTargetData::Activate()
{
    if ( Ability == nullptr || !Ability->GetCurrentActorInfo()->IsNetAuthority() )
    {
        return;
    }

    const auto spec_handle = GetAbilitySpecHandle();
    const auto activation_prediction_key = GetActivationPredictionKey();
    AbilitySystemComponent->AbilityTargetDataSetDelegate( spec_handle, activation_prediction_key ).AddUObject( this, &UGBFAT_ServerWaitClientTargetData::OnTargetDataReplicatedCallback );
}

void UGBFAT_ServerWaitClientTargetData::OnTargetDataReplicatedCallback( const FGameplayAbilityTargetDataHandle & data, FGameplayTag /* activation_tag */ )
{
    const auto mutable_data = data;
    AbilitySystemComponent->ConsumeClientReplicatedTargetData( GetAbilitySpecHandle(), GetActivationPredictionKey() );

    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        ValidData.Broadcast( mutable_data );
    }

    if ( TriggerOnce )
    {
        EndTask();
    }
}

void UGBFAT_ServerWaitClientTargetData::OnDestroy( const bool ability_ended )
{
    if ( AbilitySystemComponent.IsValid() )
    {
        const auto spec_handle = GetAbilitySpecHandle();
        const auto activation_prediction_key = GetActivationPredictionKey();
        AbilitySystemComponent->AbilityTargetDataSetDelegate( spec_handle, activation_prediction_key ).RemoveAll( this );
    }

    Super::OnDestroy( ability_ended );
}
