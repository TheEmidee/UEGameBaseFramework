#include "GAS/Tasks/GBFAT_WaitTargetDataUsingActor.h"

#include "GAS/Targeting/GBFGameplayAbilityTargetActor.h"

#include <Abilities/GameplayAbilityTargetActor.h>
#include <AbilitySystemComponent.h>

UGBFAT_WaitTargetDataUsingActor * UGBFAT_WaitTargetDataUsingActor::WaitTargetDataWithReusableActor( UGameplayAbility * owning_ability, const FName task_instance_name, const TEnumAsByte< EGameplayTargetingConfirmation::Type > confirmation_type, AGameplayAbilityTargetActor * target_actor, const bool create_key_if_not_valid_for_more_predicting )
{
    auto * my_obj = NewAbilityTask< UGBFAT_WaitTargetDataUsingActor >( owning_ability, task_instance_name ); // Register for task list here, providing a given FName as a key
    my_obj->TargetActor = target_actor;
    my_obj->ConfirmationType = confirmation_type;
    my_obj->CreateKeyIfNotValidForMorePredicting = create_key_if_not_valid_for_more_predicting;
    return my_obj;
}

void UGBFAT_WaitTargetDataUsingActor::Activate()
{
    if ( !IsValid( this ) )
    {
        return;
    }

    if ( Ability != nullptr && TargetActor != nullptr )
    {
        InitializeTargetActor();
        RegisterTargetDataCallbacks();
        FinalizeTargetActor();
    }
    else
    {
        EndTask();
    }
}

void UGBFAT_WaitTargetDataUsingActor::OnTargetDataReplicatedCallback( const FGameplayAbilityTargetDataHandle & data, FGameplayTag activation_tag )
{
    check( AbilitySystemComponent != nullptr );

    auto mutable_data = data;
    AbilitySystemComponent->ConsumeClientReplicatedTargetData( GetAbilitySpecHandle(), GetActivationPredictionKey() );

    /**
     *  Call into the TargetActor to sanitize/verify the data. If this returns false, we are rejecting
     *	the replicated target data and will treat this as a cancel.
     *
     *	This can also be used for bandwidth optimizations. OnReplicatedTargetDataReceived could do an actual
     *	trace/check/whatever server side and use that data. So rather than having the client send that data
     *	explicitly, the client is basically just sending a 'confirm' and the server is now going to do the work
     *	in OnReplicatedTargetDataReceived.
     */
    if ( TargetActor != nullptr && !TargetActor->OnReplicatedTargetDataReceived( mutable_data ) )
    {
        if ( ShouldBroadcastAbilityTaskDelegates() )
        {
            Cancelled.Broadcast( mutable_data );
        }
    }
    else
    {
        if ( ShouldBroadcastAbilityTaskDelegates() )
        {
            ValidData.Broadcast( mutable_data );
        }
    }

    if ( ConfirmationType != EGameplayTargetingConfirmation::CustomMulti )
    {
        EndTask();
    }
}

void UGBFAT_WaitTargetDataUsingActor::OnTargetDataReplicatedCancelledCallback()
{
    check( AbilitySystemComponent != nullptr );
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        Cancelled.Broadcast( FGameplayAbilityTargetDataHandle() );
    }
    EndTask();
}

void UGBFAT_WaitTargetDataUsingActor::OnTargetDataReadyCallback( const FGameplayAbilityTargetDataHandle & data )
{
    check( AbilitySystemComponent != nullptr );
    if ( Ability == nullptr )
    {
        return;
    }

    FScopedPredictionWindow scoped_prediction(
        AbilitySystemComponent.Get(),
        ShouldReplicateDataToServer() && ( CreateKeyIfNotValidForMorePredicting && !AbilitySystemComponent->ScopedPredictionKey.IsValidForMorePrediction() ) );

    if ( IsPredictingClient() )
    {
        if ( !TargetActor->ShouldProduceTargetDataOnServer )
        {
            const FGameplayTag application_tag; // Fixme: where would this be useful?
            AbilitySystemComponent->CallServerSetReplicatedTargetData( GetAbilitySpecHandle(), GetActivationPredictionKey(), data, application_tag, AbilitySystemComponent->ScopedPredictionKey );
        }
        else if ( ConfirmationType == EGameplayTargetingConfirmation::UserConfirmed )
        {
            // We aren't going to send the target data, but we will send a generic confirmed message.
            AbilitySystemComponent->ServerSetReplicatedEvent( EAbilityGenericReplicatedEvent::GenericConfirm, GetAbilitySpecHandle(), GetActivationPredictionKey(), AbilitySystemComponent->ScopedPredictionKey );
        }
    }

    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        ValidData.Broadcast( data );
    }

    if ( ConfirmationType != EGameplayTargetingConfirmation::CustomMulti )
    {
        EndTask();
    }
}

void UGBFAT_WaitTargetDataUsingActor::OnTargetDataCancelledCallback( const FGameplayAbilityTargetDataHandle & Data )
{
    check( AbilitySystemComponent != nullptr );

    FScopedPredictionWindow scoped_prediction( AbilitySystemComponent.Get(), IsPredictingClient() );

    if ( IsPredictingClient() )
    {
        if ( !TargetActor->ShouldProduceTargetDataOnServer )
        {
            AbilitySystemComponent->ServerSetReplicatedTargetDataCancelled( GetAbilitySpecHandle(), GetActivationPredictionKey(), AbilitySystemComponent->ScopedPredictionKey );
        }
        else
        {
            // We aren't going to send the target data, but we will send a generic confirmed message.
            AbilitySystemComponent->ServerSetReplicatedEvent( EAbilityGenericReplicatedEvent::GenericCancel, GetAbilitySpecHandle(), GetActivationPredictionKey(), AbilitySystemComponent->ScopedPredictionKey );
        }
    }
    Cancelled.Broadcast( Data );
    EndTask();
}

void UGBFAT_WaitTargetDataUsingActor::ExternalConfirm( const bool end_task )
{
    check( AbilitySystemComponent != nullptr );
    if ( TargetActor != nullptr )
    {
        if ( TargetActor->ShouldProduceTargetData() )
        {
            TargetActor->ConfirmTargetingAndContinue();
        }
    }
    Super::ExternalConfirm( end_task );
}

void UGBFAT_WaitTargetDataUsingActor::ExternalCancel()
{
    check( AbilitySystemComponent != nullptr );
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        Cancelled.Broadcast( FGameplayAbilityTargetDataHandle() );
    }
    Super::ExternalCancel();
}

void UGBFAT_WaitTargetDataUsingActor::InitializeTargetActor() const
{
    check( TargetActor != nullptr );
    check( Ability != nullptr );

    TargetActor->PrimaryPC = Ability->GetCurrentActorInfo()->PlayerController.Get();

    // If we spawned the target actor, always register the callbacks for when the data is ready.
    TargetActor->TargetDataReadyDelegate.AddUObject( const_cast< UGBFAT_WaitTargetDataUsingActor * >( this ), &UGBFAT_WaitTargetDataUsingActor::OnTargetDataReadyCallback );
    TargetActor->CanceledDelegate.AddUObject( const_cast< UGBFAT_WaitTargetDataUsingActor * >( this ), &UGBFAT_WaitTargetDataUsingActor::OnTargetDataCancelledCallback );
}

void UGBFAT_WaitTargetDataUsingActor::FinalizeTargetActor() const
{
    check( TargetActor != nullptr );
    check( Ability != nullptr );

    TargetActor->StartTargeting( Ability );

    if ( TargetActor->ShouldProduceTargetData() )
    {
        // If instant confirm, then stop targeting immediately.
        // Note this is kind of bad: we should be able to just call a static func on the CDO to do this.
        // But then we wouldn't get to set ExposeOnSpawnParameters.
        if ( ConfirmationType == EGameplayTargetingConfirmation::Instant )
        {
            TargetActor->ConfirmTargeting();
        }
        else if ( ConfirmationType == EGameplayTargetingConfirmation::UserConfirmed )
        {
            // Bind to the Cancel/Confirm Delegates (called from local confirm or from replicated confirm)
            TargetActor->BindToConfirmCancelInputs();
        }
    }
}

void UGBFAT_WaitTargetDataUsingActor::RegisterTargetDataCallbacks()
{
    if ( !ensure( IsValid( this ) ) )
    {
        return;
    }

    check( Ability != nullptr );

    const auto is_locally_controlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
    const auto should_produce_target_data_on_server = TargetActor->ShouldProduceTargetDataOnServer;

    // If not locally controlled (server for remote client), see if TargetData was already sent
    // else register callback for when it does get here.
    if ( !is_locally_controlled )
    {
        // Register with the TargetData callbacks if we are expecting client to send them
        if ( !should_produce_target_data_on_server )
        {
            const auto spec_handle = GetAbilitySpecHandle();
            const auto activation_prediction_key = GetActivationPredictionKey();

            // Since multi-fire is supported, we still need to hook up the callbacks
            AbilitySystemComponent->AbilityTargetDataSetDelegate( spec_handle, activation_prediction_key ).AddUObject( this, &UGBFAT_WaitTargetDataUsingActor::OnTargetDataReplicatedCallback );
            AbilitySystemComponent->AbilityTargetDataCancelledDelegate( spec_handle, activation_prediction_key ).AddUObject( this, &UGBFAT_WaitTargetDataUsingActor::OnTargetDataReplicatedCancelledCallback );

            AbilitySystemComponent->CallReplicatedTargetDataDelegatesIfSet( spec_handle, activation_prediction_key );

            SetWaitingOnRemotePlayerData();
        }
    }
}

void UGBFAT_WaitTargetDataUsingActor::OnDestroy( const bool ability_ended )
{
    if ( TargetActor != nullptr )
    {
        if ( auto * trace_target_actor = Cast< AGBFGameplayAbilityTargetActor >( TargetActor ) )
        {
            trace_target_actor->StopTargeting();
        }
        else
        {
            // TargetActor doesn't have a StopTargeting function
            TargetActor->SetActorTickEnabled( false );

            // Clear added callbacks
            TargetActor->TargetDataReadyDelegate.RemoveAll( this );
            TargetActor->CanceledDelegate.RemoveAll( this );

            AbilitySystemComponent->GenericLocalConfirmCallbacks.RemoveDynamic( TargetActor, &AGameplayAbilityTargetActor::ConfirmTargeting );
            AbilitySystemComponent->GenericLocalCancelCallbacks.RemoveDynamic( TargetActor, &AGameplayAbilityTargetActor::CancelTargeting );
            TargetActor->GenericDelegateBoundASC = nullptr;
        }
    }

    Super::OnDestroy( ability_ended );
}

bool UGBFAT_WaitTargetDataUsingActor::ShouldReplicateDataToServer() const
{
    if ( Ability == nullptr || TargetActor == nullptr )
    {
        return false;
    }

    // Send TargetData to the server IFF we are the client and this isn't a GameplayTargetActor that can produce data on the server
    const auto info = Ability->GetCurrentActorInfo();
    if ( !info->IsNetAuthority() && !TargetActor->ShouldProduceTargetDataOnServer )
    {
        return true;
    }

    return false;
}
