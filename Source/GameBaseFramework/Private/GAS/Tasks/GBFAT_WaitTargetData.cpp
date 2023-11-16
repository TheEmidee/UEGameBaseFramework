#include "GAS/Tasks/GBFAT_WaitTargetData.h"

#include <AbilitySystemComponent.h>
#include <Engine/World.h>
#include <TimerManager.h>

FGBFWaitTargetDataReplicationOptions::FGBFWaitTargetDataReplicationOptions()
{
    bShouldProduceTargetDataOnServer = false;
    bCreateKeyIfNotValidForMorePredicting = true;
}

UGBFAT_WaitTargetData::UGBFAT_WaitTargetData()
{
    bEndTaskWhenTargetDataSent = true;
    TargetDataProductionRate = 0.0f;
}

void UGBFAT_WaitTargetData::Activate()
{
    if ( !IsValid( this ) )
    {
        return;
    }

    check( IsValid( Ability ) );

    // If not locally controlled (server for remote client), see if TargetData was already sent
    // else register callback for when it does get here.
    if ( !Ability->GetCurrentActorInfo()->IsLocallyControlled() )
    {
        // Register with the TargetData callbacks if we are expecting client to send them
        if ( !ReplicationOptions.bShouldProduceTargetDataOnServer )
        {
            const auto spec_handle = GetAbilitySpecHandle();
            const auto activation_prediction_key = GetActivationPredictionKey();

            // Since multi-fire is supported, we still need to hook up the callbacks
            AbilitySystemComponent->AbilityTargetDataSetDelegate( spec_handle, activation_prediction_key ).AddUObject( this, &UGBFAT_WaitTargetData::OnTargetDataReplicatedCallback );
            AbilitySystemComponent->CallReplicatedTargetDataDelegatesIfSet( spec_handle, activation_prediction_key );
            SetWaitingOnRemotePlayerData();
        }
    }

    TryProduceTargetData();
}

void UGBFAT_WaitTargetData::OnDestroy( const bool ability_ended )
{
    if ( const auto * world = GetWorld() )
    {
        world->GetTimerManager().ClearTimer( TimerHandle );
    }

    if ( AbilitySystemComponent != nullptr )
    {
        const auto spec_handle = GetAbilitySpecHandle();
        const auto activation_prediction_key = GetActivationPredictionKey();
        AbilitySystemComponent->AbilityTargetDataSetDelegate( spec_handle, activation_prediction_key ).RemoveAll( this );
    }

    Super::OnDestroy( ability_ended );
}

// ReSharper disable once CppMemberFunctionMayBeStatic
bool UGBFAT_WaitTargetData::OnReplicatedTargetDataReceived( const FGameplayAbilityTargetDataHandle & data ) const
{
    return true;
}

void UGBFAT_WaitTargetData::OnTargetDataReplicatedCallback( const FGameplayAbilityTargetDataHandle & data, FGameplayTag activation_tag )
{
    check( AbilitySystemComponent != nullptr );

    AbilitySystemComponent->ConsumeClientReplicatedTargetData( GetAbilitySpecHandle(), GetActivationPredictionKey() );

    if ( !OnReplicatedTargetDataReceived( data ) )
    {
        if ( ShouldBroadcastAbilityTaskDelegates() )
        {
            Cancelled.Broadcast( data );
        }
    }
    else if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        ValidData.Broadcast( data );
    }

    if ( bEndTaskWhenTargetDataSent )
    {
        EndTask();
    }
}

void UGBFAT_WaitTargetData::TryProduceTargetData()
{
    if ( ShouldProduceTargetData() )
    {
        const auto target_data_handle = ProduceTargetData();
        SendTargetData( target_data_handle );
    }
}

void UGBFAT_WaitTargetData::SendTargetData( const FGameplayAbilityTargetDataHandle & data )
{
    check( AbilitySystemComponent != nullptr );
    if ( Ability == nullptr )
    {
        return;
    }

    FScopedPredictionWindow scoped_prediction(
        AbilitySystemComponent.Get(),
        ShouldReplicateDataToServer() && ( ReplicationOptions.bCreateKeyIfNotValidForMorePredicting && !AbilitySystemComponent->ScopedPredictionKey.IsValidForMorePrediction() ) );

    if ( IsPredictingClient() )
    {
        if ( !ReplicationOptions.bShouldProduceTargetDataOnServer )
        {
            static const FGameplayTag ApplicationTag; // Fixme: where would this be useful?

            AbilitySystemComponent->CallServerSetReplicatedTargetData(
                GetAbilitySpecHandle(),
                GetActivationPredictionKey(),
                data,
                ApplicationTag,
                AbilitySystemComponent->ScopedPredictionKey );
        }
    }

    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        ValidData.Broadcast( data );
    }

    if ( bEndTaskWhenTargetDataSent )
    {
        EndTask();
    }
    else
    {
        if ( TargetDataProductionRate > 0.0f )
        {
            GetWorld()->GetTimerManager().SetTimer( TimerHandle, this, &ThisClass::TryProduceTargetData, TargetDataProductionRate, false );
        }
    }
}

bool UGBFAT_WaitTargetData::ShouldReplicateDataToServer() const
{
    if ( Ability == nullptr )
    {
        return false;
    }

    const auto actor_info = Ability->GetCurrentActorInfo();
    return !actor_info->IsNetAuthority();
}

bool UGBFAT_WaitTargetData::ShouldProduceTargetData() const
{
    // return true if we are locally owned, or (we are the server and this is a gameplay target ability that can produce target data server side)
    return Ability->GetCurrentActorInfo()->IsLocallyControlled() || ReplicationOptions.bShouldProduceTargetDataOnServer;
}