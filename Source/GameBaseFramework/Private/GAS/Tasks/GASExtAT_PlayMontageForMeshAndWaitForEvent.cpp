#include "GAS/Tasks/GASExtAT_PlayMontageForMeshAndWaitForEvent.h"

#include <AbilitySystemGlobals.h>
#include <Animation/AnimInstance.h>
#include <Components/SkeletalMeshComponent.h>
#include <GameFramework/Character.h>

UGASExtAT_PlayMontageForMeshAndWaitForEvent::UGASExtAT_PlayMontageForMeshAndWaitForEvent()
{
    Rate = 1.0f;
    StopsWhenAbilityEnds = true;
}

void UGASExtAT_PlayMontageForMeshAndWaitForEvent::Activate()
{
    if ( Ability == nullptr )
    {
        return;
    }

    if ( Mesh == nullptr )
    {
        UE_LOG( LogTemp, Error, TEXT( "%s invalid Mesh" ), StringCast< TCHAR >( __FUNCTION__ ).Get() );
        return;
    }

    auto montage_was_played = false;

    if ( auto * asc = GetTargetASC() )
    {
        if ( auto * anim_instance = Mesh->GetAnimInstance() )
        {
            // Bind to event callback
            EventHandle = asc->AddGameplayEventTagContainerDelegate( EventTags, FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject( this, &UGASExtAT_PlayMontageForMeshAndWaitForEvent::OnGameplayEvent ) );

            if ( asc->PlayMontageForMesh( Ability, Mesh, MontageToPlay, Rate, StartSection, MustReplicateMontage ) > 0.f )
            {
                // Playing a montage could potentially fire off a callback into game code which could kill this ability! Early out if we are  pending kill.
                if ( ShouldBroadcastAbilityTaskDelegates() == false )
                {
                    return;
                }

                CancelledHandle = Ability->OnGameplayAbilityCancelled.AddUObject( this, &UGASExtAT_PlayMontageForMeshAndWaitForEvent::OnAbilityCancelled );

                BlendingOutDelegate.BindUObject( this, &UGASExtAT_PlayMontageForMeshAndWaitForEvent::OnMontageBlendingOut );
                anim_instance->Montage_SetBlendingOutDelegate( BlendingOutDelegate, MontageToPlay );

                MontageEndedDelegate.BindUObject( this, &UGASExtAT_PlayMontageForMeshAndWaitForEvent::OnMontageEnded );
                anim_instance->Montage_SetEndDelegate( MontageEndedDelegate, MontageToPlay );

                if ( auto * character = Cast< ACharacter >( GetAvatarActor() ) )
                {
                    if ( character->GetLocalRole() == ROLE_Authority || ( character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted ) )
                    {
                        character->SetAnimRootMotionTranslationScale( AnimRootMotionTranslationScale );
                    }
                }

                montage_was_played = true;
            }
        }
        else
        {
            UE_LOG( LogTemp, Warning, TEXT( "UGASExtAT_PlayMontageForMeshAndWaitForEvent call to PlayMontage failed!" ) );
        }
    }
    else
    {
        UE_LOG( LogTemp, Warning, TEXT( "UGASExtAT_PlayMontageForMeshAndWaitForEvent called on invalid AbilitySystemComponent" ) );
    }

    if ( !montage_was_played )
    {
        UE_LOG( LogTemp, Warning, TEXT( "UGASExtAT_PlayMontageForMeshAndWaitForEvent called in Ability %s failed to play montage %s; Task Instance Name %s." ), *Ability->GetName(), *GetNameSafe( MontageToPlay ), *InstanceName.ToString() );
        if ( ShouldBroadcastAbilityTaskDelegates() )
        {
            // ABILITY_LOG(Display, TEXT("%s: OnCancelled"), *GetName());
            OnCancelled.Broadcast( FGameplayTag(), FGameplayEventData() );
        }
    }

    SetWaitingOnAvatar();
}

void UGASExtAT_PlayMontageForMeshAndWaitForEvent::ExternalCancel()
{
    check( AbilitySystemComponent != nullptr );

    OnAbilityCancelled();

    Super::ExternalCancel();
}

FString UGASExtAT_PlayMontageForMeshAndWaitForEvent::GetDebugString() const
{
    UAnimMontage * playing_montage = nullptr;
    if ( Ability != nullptr && Mesh != nullptr )
    {
        if ( auto * anim_instance = Mesh->GetAnimInstance() )
        {
            playing_montage = anim_instance->Montage_IsActive( MontageToPlay ) ? MontageToPlay : anim_instance->GetCurrentActiveMontage();
        }
    }

    return FString::Printf( TEXT( "PlayMontageAndWaitForEvent. MontageToPlay: %s  (Currently Playing): %s" ), *GetNameSafe( MontageToPlay ), *GetNameSafe( playing_montage ) );
}

void UGASExtAT_PlayMontageForMeshAndWaitForEvent::OnDestroy( const bool ability_ended )
{
    // Note: Clearing montage end delegate isn't necessary since its not a multicast and will be cleared when the next montage plays.
    // (If we are destroyed, it will detect this and not do anything)

    // This delegate, however, should be cleared as it is a multicast
    if ( Ability != nullptr )
    {
        Ability->OnGameplayAbilityCancelled.Remove( CancelledHandle );
        if ( ability_ended && StopsWhenAbilityEnds )
        {
            // ReSharper disable once CppExpressionWithoutSideEffects
            StopPlayingMontage( OverrideBlendOutTimeForStopWhenEndAbility );
        }
    }

    if ( auto * asc = GetTargetASC() )
    {
        asc->RemoveGameplayEventTagContainerDelegate( EventTags, EventHandle );
    }

    Super::OnDestroy( ability_ended );
}

UGASExtAT_PlayMontageForMeshAndWaitForEvent * UGASExtAT_PlayMontageForMeshAndWaitForEvent::PlayMontageForMeshAndWaitForEvent(
    UGameplayAbility * owning_ability,
    const FName task_instance_name,
    USkeletalMeshComponent * mesh,
    UAnimMontage * montage_to_play,
    const FGameplayTagContainer event_tags,
    float rate,
    const FName start_section,
    const bool stop_when_ability_ends,
    const float anim_root_motion_translation_scale,
    const bool replicate_montage,
    const float override_blend_out_time_for_cancel_ability,
    const float override_blend_out_time_for_stop_when_end_ability )
{
    UAbilitySystemGlobals::NonShipping_ApplyGlobalAbilityScaler_Rate( rate );

    auto * my_obj = NewAbilityTask< UGASExtAT_PlayMontageForMeshAndWaitForEvent >( owning_ability, task_instance_name );
    my_obj->Mesh = mesh;
    my_obj->MontageToPlay = montage_to_play;
    my_obj->EventTags = event_tags;
    my_obj->Rate = rate;
    my_obj->StartSection = start_section;
    my_obj->AnimRootMotionTranslationScale = anim_root_motion_translation_scale;
    my_obj->StopsWhenAbilityEnds = stop_when_ability_ends;
    my_obj->MustReplicateMontage = replicate_montage;
    my_obj->OverrideBlendOutTimeForCancelAbility = override_blend_out_time_for_cancel_ability;
    my_obj->OverrideBlendOutTimeForStopWhenEndAbility = override_blend_out_time_for_stop_when_end_ability;

    return my_obj;
}

bool UGASExtAT_PlayMontageForMeshAndWaitForEvent::StopPlayingMontage( const float override_blend_out_time ) const
{
    if ( Mesh == nullptr )
    {
        return false;
    }

    auto * asc = GetTargetASC();
    if ( asc == nullptr )
    {
        return false;
    }

    auto * actor_info = Ability->GetCurrentActorInfo();
    if ( actor_info == nullptr )
    {
        return false;
    }

    auto * anim_instance = Mesh->GetAnimInstance();
    if ( anim_instance == nullptr )
    {
        return false;
    }

    // Check if the montage is still playing
    // The ability would have been interrupted, in which case we should automatically stop the montage
    if ( Ability != nullptr )
    {
        if ( asc->GetAnimatingAbilityFromAnyMesh() == Ability && asc->GetCurrentMontageForMesh( Mesh ) == MontageToPlay )
        {
            // Unbind delegates so they don't get called as well
            auto * montage_instance = anim_instance->GetActiveInstanceForMontage( MontageToPlay );
            if ( montage_instance )
            {
                montage_instance->OnMontageBlendingOutStarted.Unbind();
                montage_instance->OnMontageEnded.Unbind();
            }

            asc->CurrentMontageStopForMesh( Mesh, override_blend_out_time );
            return true;
        }
    }

    return false;
}

UGASExtAbilitySystemComponent * UGASExtAT_PlayMontageForMeshAndWaitForEvent::GetTargetASC() const
{
    return Cast< UGASExtAbilitySystemComponent >( AbilitySystemComponent );
}

void UGASExtAT_PlayMontageForMeshAndWaitForEvent::OnMontageBlendingOut( UAnimMontage * montage, const bool was_interrupted ) const
{
    if ( Ability && Ability->GetCurrentMontage() == MontageToPlay )
    {
        if ( montage == MontageToPlay )
        {
            AbilitySystemComponent->ClearAnimatingAbility( Ability );

            // Reset AnimRootMotionTranslationScale
            if ( auto * character = Cast< ACharacter >( GetAvatarActor() ) )
            {
                if ( character->GetLocalRole() == ROLE_Authority || ( character->GetLocalRole() == ROLE_AutonomousProxy && Ability->GetNetExecutionPolicy() == EGameplayAbilityNetExecutionPolicy::LocalPredicted ) )
                {
                    character->SetAnimRootMotionTranslationScale( 1.f );
                }
            }
        }
    }

    if ( was_interrupted )
    {
        if ( ShouldBroadcastAbilityTaskDelegates() )
        {
            OnInterrupted.Broadcast( FGameplayTag(), FGameplayEventData() );
        }
    }
    else
    {
        if ( ShouldBroadcastAbilityTaskDelegates() )
        {
            OnBlendOut.Broadcast( FGameplayTag(), FGameplayEventData() );
        }
    }
}

void UGASExtAT_PlayMontageForMeshAndWaitForEvent::OnAbilityCancelled() const
{
    // :TODO: Merge this fix back to engine, it was calling the wrong callback

    if ( StopPlayingMontage( OverrideBlendOutTimeForCancelAbility ) )
    {
        // Let the BP handle the interrupt as well
        if ( ShouldBroadcastAbilityTaskDelegates() )
        {
            OnCancelled.Broadcast( FGameplayTag(), FGameplayEventData() );
        }
    }
}

void UGASExtAT_PlayMontageForMeshAndWaitForEvent::OnMontageEnded( UAnimMontage * montage, const bool was_interrupted )
{
    if ( !was_interrupted )
    {
        if ( ShouldBroadcastAbilityTaskDelegates() )
        {
            OnCompleted.Broadcast( FGameplayTag(), FGameplayEventData() );
        }
    }

    EndTask();
}

void UGASExtAT_PlayMontageForMeshAndWaitForEvent::OnGameplayEvent( const FGameplayTag event_tag, const FGameplayEventData * payload ) const
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        auto temp_data = *payload;
        temp_data.EventTag = event_tag;

        EventReceived.Broadcast( event_tag, temp_data );
    }
}