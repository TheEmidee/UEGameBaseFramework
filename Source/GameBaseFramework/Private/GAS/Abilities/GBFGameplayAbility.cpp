#include "GAS/Abilities/GBFGameplayAbility.h"

#include "Camera/GBFCameraMode.h"
#include "Characters/Components/GBFHeroComponent.h"
#include "GAS/Components/GBFAbilitySystemComponent.h"

#include <Abilities/Tasks/AbilityTask.h>
#include <AbilitySystemGlobals.h>
#include <AbilitySystemLog.h>
#include <GameFramework/Pawn.h>
#include <GameFramework/PlayerController.h>
#include <GameplayTask.h>

#define ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN( FunctionName, ReturnValue )                                                                                \
    {                                                                                                                                                        \
        if ( !ensure( IsInstantiated() ) )                                                                                                                   \
        {                                                                                                                                                    \
            ABILITY_LOG( Error, TEXT( "%s: " #FunctionName " cannot be called on a non-instanced ability. Check the instancing policy." ), *GetPathName() ); \
            return ReturnValue;                                                                                                                              \
        }                                                                                                                                                    \
    }

UGBFGameplayAbility::UGBFGameplayAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    ActivationGroup = EGBFAbilityActivationGroup::Independent;
    ActivationPolicy = EGBFAbilityActivationPolicy::OnInputTriggered;

    ActiveCameraMode = nullptr;
}

bool UGBFGameplayAbility::K2_IsLocallyControlled() const
{
    return IsLocallyControlled();
}

UGBFAbilitySystemComponent * UGBFGameplayAbility::GetGBFAbilitySystemComponent() const
{
    if ( auto * asc = GetAbilitySystemComponentFromActorInfo_Ensured() )
    {
        return Cast< UGBFAbilitySystemComponent >( asc );
    }

    return nullptr;
}

UGBFAbilitySystemComponent * UGBFGameplayAbility::GetGBFAbilitySystemComponentFromActorInfo() const
{
    return CurrentActorInfo
               ? Cast< UGBFAbilitySystemComponent >( CurrentActorInfo->AbilitySystemComponent.Get() )
               : nullptr;
}

AController * UGBFGameplayAbility::GetControllerFromActorInfo() const
{
    if ( CurrentActorInfo )
    {
        if ( auto * pc = CurrentActorInfo->PlayerController.Get() )
        {
            return pc;
        }

        // Look for a player controller or pawn in the owner chain.
        auto * test_actor = CurrentActorInfo->OwnerActor.Get();
        while ( test_actor )
        {
            if ( auto * controller = Cast< AController >( test_actor ) )
            {
                return controller;
            }

            if ( const auto * pawn = Cast< APawn >( test_actor ) )
            {
                return pawn->GetController();
            }

            test_actor = test_actor->GetOwner();
        }
    }

    return nullptr;
}

void UGBFGameplayAbility::ExternalEndAbility()
{
    check( CurrentActorInfo != nullptr );

    const auto replicate_end_ability = true;
    const auto was_cancelled = false;

    EndAbility( CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, replicate_end_ability, was_cancelled );
}

void UGBFGameplayAbility::SetCurrentMontageForMesh( USkeletalMeshComponent * mesh, UAnimMontage * current_montage )
{
    ensure( IsInstantiated() );

    FGBFAbilityMeshMontage ability_mesh_montage;
    if ( FindAbilityMeshMontage( ability_mesh_montage, mesh ) )
    {
        ability_mesh_montage.Montage = current_montage;
    }
    else
    {
        CurrentAbilityMeshMontages.Add( FGBFAbilityMeshMontage( mesh, current_montage ) );
    }
}

UAnimMontage * UGBFGameplayAbility::GetCurrentMontageForMesh( USkeletalMeshComponent * mesh ) const
{
    FGBFAbilityMeshMontage ability_mesh_montage;
    if ( FindAbilityMeshMontage( ability_mesh_montage, mesh ) )
    {
        return ability_mesh_montage.Montage;
    }

    return nullptr;
}

bool UGBFGameplayAbility::CanChangeActivationGroup( EGBFAbilityActivationGroup new_group ) const
{
    if ( !IsInstantiated() || !IsActive() )
    {
        return false;
    }

    if ( ActivationGroup == new_group )
    {
        return true;
    }

    UGBFAbilitySystemComponent * asc = GetGBFAbilitySystemComponentFromActorInfo();
    check( asc );

    if ( ( ActivationGroup != EGBFAbilityActivationGroup::ExclusiveBlocking ) && asc->IsActivationGroupBlocked( new_group ) )
    {
        // This ability can't change groups if it's blocked (unless it is the one doing the blocking).
        return false;
    }

    if ( ( new_group == EGBFAbilityActivationGroup::ExclusiveReplaceable ) && !CanBeCanceled() )
    {
        // This ability can't become replaceable if it can't be canceled.
        return false;
    }

    return true;
}

bool UGBFGameplayAbility::ChangeActivationGroup( const EGBFAbilityActivationGroup new_group )
{
    ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN( ChangeActivationGroup, false );

    if ( !CanChangeActivationGroup( new_group ) )
    {
        return false;
    }

    if ( ActivationGroup != new_group )
    {
        auto * asc = GetGBFAbilitySystemComponentFromActorInfo();
        check( asc );

        asc->RemoveAbilityFromActivationGroup( ActivationGroup, this );
        asc->AddAbilityToActivationGroup( new_group, this );

        ActivationGroup = new_group;
    }

    return true;
}

int32 UGBFGameplayAbility::GetInputID() const
{
    return INDEX_NONE;
}

void UGBFGameplayAbility::SetCanBeCanceled( const bool can_be_canceled )
{
    // The ability can not block canceling if it's replaceable.
    if ( !can_be_canceled && ( ActivationGroup == EGBFAbilityActivationGroup::ExclusiveReplaceable ) )
    {
        UE_LOG( LogTemp, Error, TEXT( "SetCanBeCanceled: Ability [%s] can not block canceling because its activation group is replaceable." ), *GetName() );
        return;
    }

    Super::SetCanBeCanceled( can_be_canceled );
}

void UGBFGameplayAbility::TryActivateAbilityOnSpawn( const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilitySpec & spec ) const
{
    const auto is_predicting = ( spec.ActivationInfo.ActivationMode == EGameplayAbilityActivationMode::Predicting );

    // Try to activate if activation policy is on spawn.
    if ( actor_info && !spec.IsActive() && !is_predicting && ( ActivationPolicy == EGBFAbilityActivationPolicy::OnSpawn ) )
    {
        auto * asc = actor_info->AbilitySystemComponent.Get();
        const auto * avatar_actor = actor_info->AvatarActor.Get();

        // If avatar actor is torn off or about to die, don't try to activate until we get the new one.
        if ( asc && avatar_actor && !avatar_actor->GetTearOff() && ( avatar_actor->GetLifeSpan() <= 0.0f ) )
        {
            const auto is_local_execution = ( NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalPredicted ) || ( NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::LocalOnly );
            const auto is_server_execution = ( NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerOnly ) || ( NetExecutionPolicy == EGameplayAbilityNetExecutionPolicy::ServerInitiated );

            const auto client_should_activate = actor_info->IsLocallyControlled() && is_local_execution;
            const auto server_should_activate = actor_info->IsNetAuthority() && is_server_execution;

            if ( client_should_activate || server_should_activate )
            {
                asc->TryActivateAbility( spec.Handle );
            }
        }
    }
}

void UGBFGameplayAbility::OnPawnAvatarSet()
{
    K2_OnPawnAvatarSet();
}

bool UGBFGameplayAbility::DoesAbilitySatisfyTagRequirements( const UAbilitySystemComponent & ability_system_component, const FGameplayTagContainer * source_tags, const FGameplayTagContainer * target_tags, FGameplayTagContainer * optional_relevant_tags ) const
{
    auto blocked = false;
    auto missing = false;

    const auto & ability_system_globals = UAbilitySystemGlobals::Get();
    const auto & blocked_tag = ability_system_globals.ActivateFailTagsBlockedTag;
    const auto & missing_tag = ability_system_globals.ActivateFailTagsMissingTag;

    // Check if any of this ability's tags are currently blocked
    if ( ability_system_component.AreAbilityTagsBlocked( AbilityTags ) )
    {
        blocked = true;
    }

    static FGameplayTagContainer AllRequiredTags;
    static FGameplayTagContainer AllBlockedTags;

    AllRequiredTags = ActivationRequiredTags;
    AllBlockedTags = ActivationBlockedTags;

    // Expand our ability tags to add additional required/blocked tags
    if ( const auto * gas_ext_asc = Cast< UGBFAbilitySystemComponent >( &ability_system_component ) )
    {
        gas_ext_asc->GetAdditionalActivationTagRequirements( AbilityTags, AllRequiredTags, AllBlockedTags );
    }

    // Check to see the required/blocked tags for this ability
    if ( AllBlockedTags.Num() || AllRequiredTags.Num() )
    {
        static FGameplayTagContainer AbilitySystemComponentTags;

        AbilitySystemComponentTags.Reset();
        ability_system_component.GetOwnedGameplayTags( AbilitySystemComponentTags );

        if ( AbilitySystemComponentTags.HasAny( AllBlockedTags ) )
        {
            AddBlockedAbilityOptionalRelevantTags( AbilitySystemComponentTags, optional_relevant_tags );

            blocked = true;
        }

        if ( !AbilitySystemComponentTags.HasAll( AllRequiredTags ) )
        {
            missing = true;
        }
    }

    if ( source_tags != nullptr )
    {
        if ( SourceBlockedTags.Num() || SourceRequiredTags.Num() )
        {
            if ( source_tags->HasAny( SourceBlockedTags ) )
            {
                blocked = true;
            }

            if ( !source_tags->HasAll( SourceRequiredTags ) )
            {
                missing = true;
            }
        }
    }

    if ( target_tags != nullptr )
    {
        if ( TargetBlockedTags.Num() || TargetRequiredTags.Num() )
        {
            if ( target_tags->HasAny( TargetBlockedTags ) )
            {
                blocked = true;
            }

            if ( !target_tags->HasAll( TargetRequiredTags ) )
            {
                missing = true;
            }
        }
    }

    if ( blocked )
    {
        if ( optional_relevant_tags != nullptr && blocked_tag.IsValid() )
        {
            optional_relevant_tags->AddTag( blocked_tag );
        }
        return false;
    }
    if ( missing )
    {
        if ( optional_relevant_tags != nullptr && missing_tag.IsValid() )
        {
            optional_relevant_tags->AddTag( missing_tag );
        }
        return false;
    }

    return true;
}

void UGBFGameplayAbility::ExecuteGameplayCueLocal( const FGameplayTag gameplay_cue_tag, const FGameplayCueParameters & parameters )
{
    if ( auto * const asc = GetGBFAbilitySystemComponentFromActorInfo() )
    {
        asc->ExecuteGameplayCueLocal( gameplay_cue_tag, parameters );
    }
}

void UGBFGameplayAbility::AddGameplayCueLocal( const FGameplayTag gameplay_cue_tag, const FGameplayCueParameters & parameters )
{
    if ( auto * const asc = GetGBFAbilitySystemComponentFromActorInfo() )
    {
        asc->AddGameplayCueLocal( gameplay_cue_tag, parameters );
    }
}

void UGBFGameplayAbility::RemoveGameplayCueLocal( const FGameplayTag gameplay_cue_tag, const FGameplayCueParameters & parameters )
{
    if ( auto * const asc = GetGBFAbilitySystemComponentFromActorInfo() )
    {
        asc->RemoveGameplayCueLocal( gameplay_cue_tag, parameters );
    }
}

UGBFHeroComponent * UGBFGameplayAbility::GetHeroComponentFromActorInfo() const
{
    return CurrentActorInfo ? UGBFHeroComponent::FindHeroComponent( CurrentActorInfo->AvatarActor.Get() ) : nullptr;
}

void UGBFGameplayAbility::SetCameraMode( TSubclassOf< UGBFCameraMode > camera_mode )
{
    ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN( SetCameraMode, );

    if ( auto * hero_component = GetHeroComponentFromActorInfo() )
    {
        hero_component->SetAbilityCameraMode( camera_mode, CurrentSpecHandle );
        ActiveCameraMode = camera_mode;
    }
}

void UGBFGameplayAbility::ClearCameraMode()
{
    ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN( ClearCameraMode, );

    if ( ActiveCameraMode != nullptr )
    {
        if ( auto * hero_component = GetHeroComponentFromActorInfo() )
        {
            hero_component->ClearAbilityCameraMode( CurrentSpecHandle );
        }

        ActiveCameraMode = nullptr;
    }
}

bool UGBFGameplayAbility::CanActivateAbility( const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo * actor_info, const FGameplayTagContainer * source_tags, const FGameplayTagContainer * target_tags, FGameplayTagContainer * optional_relevant_tags ) const
{
    if ( !actor_info || !actor_info->AbilitySystemComponent.IsValid() )
    {
        return false;
    }

    auto * ability_system_component = CastChecked< UGBFAbilitySystemComponent >( actor_info->AbilitySystemComponent.Get() );

    if ( !Super::CanActivateAbility( handle, actor_info, source_tags, target_tags, optional_relevant_tags ) )
    {
        return false;
    }

    if ( ability_system_component->IsActivationGroupBlocked( ActivationGroup ) )
    {
        // :TODO: Ability Failure
        /*if ( OptionalRelevantTags )
        {
            OptionalRelevantTags->AddTag( GameplayTags.Ability_ActivateFail_ActivationGroup );
        }*/
        return false;
    }

    return true;
}

void UGBFGameplayAbility::EndAbility( const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilityActivationInfo activation_info, const bool replicate_end_ability, const bool was_cancelled )
{
    Super::EndAbility( handle, actor_info, activation_info, replicate_end_ability, was_cancelled );

    for ( auto * task : TasksToEndWhenAbilityEnds )
    {
        if ( IsValid( task ) && task->GetState() != EGameplayTaskState::Finished )
        {
            task->EndTask();
        }
    }

    TasksToEndWhenAbilityEnds.Reset();
}

void UGBFGameplayAbility::OnGiveAbility( const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilitySpec & spec )
{
    Super::OnGiveAbility( actor_info, spec );

    ReceiveOnGiveAbility( *actor_info, spec );

    TryActivateAbilityOnSpawn( actor_info, spec );
}

void UGBFGameplayAbility::OnRemoveAbility( const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilitySpec & spec )
{
    ReceiveOnRemoveAbility( *actor_info, spec );

    Super::OnRemoveAbility( actor_info, spec );
}

void UGBFGameplayAbility::AutoEndTaskWhenAbilityEnds( UAbilityTask * task )
{
    if ( task != nullptr )
    {
        TasksToEndWhenAbilityEnds.AddUnique( task );
    }
}

void UGBFGameplayAbility::AddBlockedAbilityOptionalRelevantTags( const FGameplayTagContainer & ability_system_component_tags, FGameplayTagContainer * optional_relevant_tags ) const
{
}

bool UGBFGameplayAbility::FindAbilityMeshMontage( FGBFAbilityMeshMontage & ability_mesh_montage, USkeletalMeshComponent * mesh ) const
{
    for ( const auto & mesh_montage : CurrentAbilityMeshMontages )
    {
        if ( mesh_montage.Mesh == mesh )
        {
            ability_mesh_montage = mesh_montage;
            return true;
        }
    }

    return false;
}

void UGBFGameplayAbility::MontageJumpToSectionForMesh( USkeletalMeshComponent * mesh, const FName section_name )
{
    check( CurrentActorInfo != nullptr );

    if ( auto * ability_system_component = Cast< UGBFAbilitySystemComponent >( GetAbilitySystemComponentFromActorInfo_Checked() ) )
    {
        if ( ability_system_component->IsAnimatingAbilityForAnyMesh( this ) )
        {
            ability_system_component->CurrentMontageJumpToSectionForMesh( mesh, section_name );
        }
    }
}

void UGBFGameplayAbility::MontageSetNextSectionNameForMesh( USkeletalMeshComponent * mesh, const FName from_section_name, const FName to_section_name )
{
    check( CurrentActorInfo != nullptr );

    if ( auto * ability_system_component = Cast< UGBFAbilitySystemComponent >( GetAbilitySystemComponentFromActorInfo_Checked() ) )
    {
        if ( ability_system_component->IsAnimatingAbilityForAnyMesh( this ) )
        {
            ability_system_component->CurrentMontageSetNextSectionNameForMesh( mesh, from_section_name, to_section_name );
        }
    }
}

void UGBFGameplayAbility::MontageStopForMesh( USkeletalMeshComponent * mesh, const float override_blend_out_time )
{
    check( CurrentActorInfo != nullptr );

    if ( auto * ability_system_component = Cast< UGBFAbilitySystemComponent >( GetAbilitySystemComponentFromActorInfo_Checked() ) )
    {
        // We should only stop the current montage if we are the animating ability
        if ( ability_system_component->IsAnimatingAbilityForAnyMesh( this ) )
        {
            ability_system_component->CurrentMontageStopForMesh( mesh, override_blend_out_time );
        }
    }
}

void UGBFGameplayAbility::MontageStopForAllMeshes( const float override_blend_out_time )
{
    check( CurrentActorInfo != nullptr );

    if ( auto * ability_system_component = Cast< UGBFAbilitySystemComponent >( GetAbilitySystemComponentFromActorInfo_Checked() ) )
    {
        if ( ability_system_component->IsAnimatingAbilityForAnyMesh( this ) )
        {
            ability_system_component->StopAllCurrentMontages( override_blend_out_time );
        }
    }
}

#undef ENSURE_ABILITY_IS_INSTANTIATED_OR_RETURN