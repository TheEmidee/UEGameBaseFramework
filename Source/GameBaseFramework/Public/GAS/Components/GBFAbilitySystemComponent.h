#pragma once

#include "GAS/GBFAbilityTypesBase.h"

#include <AbilitySystemComponent.h>
#include <CoreMinimal.h>

#include "GBFAbilitySystemComponent.generated.h"

class UGBFAbilitySet;
class UGBFAbilityTagRelationshipMapping;
class UGBFGameplayAbility;

/**
 * Data about montages that were played locally (all montages in case of server. predictive montages in case of client). Never replicated directly.
 */
USTRUCT()
struct GAMEBASEFRAMEWORK_API FGameplayAbilityLocalAnimMontageForMesh
{
    GENERATED_BODY();

public:
    UPROPERTY()
    USkeletalMeshComponent * Mesh;

    UPROPERTY()
    FGameplayAbilityLocalAnimMontage LocalMontageInfo;

    FGameplayAbilityLocalAnimMontageForMesh() :
        Mesh( nullptr )
    {
    }

    explicit FGameplayAbilityLocalAnimMontageForMesh( USkeletalMeshComponent * mesh ) :
        Mesh( mesh )
    {
    }

    FGameplayAbilityLocalAnimMontageForMesh( USkeletalMeshComponent * mesh, FGameplayAbilityLocalAnimMontage & local_montage_infos ) :
        Mesh( mesh ),
        LocalMontageInfo( local_montage_infos )
    {
    }
};

/**
 * Data about montages that is replicated to simulated clients.
 */
USTRUCT()
struct GAMEBASEFRAMEWORK_API FGameplayAbilityRepAnimMontageForMesh
{
    GENERATED_BODY();

public:
    UPROPERTY()
    USkeletalMeshComponent * Mesh;

    UPROPERTY()
    FGameplayAbilityRepAnimMontage RepMontageInfo;

    FGameplayAbilityRepAnimMontageForMesh() :
        Mesh( nullptr )
    {
    }

    explicit FGameplayAbilityRepAnimMontageForMesh( USkeletalMeshComponent * mesh ) :
        Mesh( mesh )
    {
    }

    bool NetSerialize( FArchive & ar, class UPackageMap * map, bool & out_success );
};

template <>
struct TStructOpsTypeTraits< FGameplayAbilityRepAnimMontageForMesh > : public TStructOpsTypeTraitsBase2< FGameplayAbilityRepAnimMontageForMesh >
{
    enum
    {
        WithNetSerializer = true,
    };
};

UCLASS( meta = ( BlueprintSpawnableComponent ) )
class GAMEBASEFRAMEWORK_API UGBFAbilitySystemComponent : public UAbilitySystemComponent
{
    GENERATED_BODY()

public:
    UGBFAbilitySystemComponent();

    bool ShouldDoServerAbilityRPCBatch() const override;

    // ReSharper disable once CppInconsistentNaming
    void GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const override;

    bool GetShouldTick() const override;
    void TickComponent( float delta_time, enum ELevelTick tick_type, FActorComponentTickFunction * this_tick_function ) override;
    void InitAbilityActorInfo( AActor * owner_actor, AActor * avatar_actor ) override;
    void RemoveGameplayCue_Internal( const FGameplayTag gameplay_cue_tag, FActiveGameplayCueContainer & gameplay_cue_container ) override;
    void ApplyAbilityBlockAndCancelTags( const FGameplayTagContainer & ability_tags, UGameplayAbility * requesting_ability, bool enable_block_tags, const FGameplayTagContainer & block_tags, bool execute_cancel_tags, const FGameplayTagContainer & cancel_tags ) override;
    void SetTagRelationshipMapping( UGBFAbilityTagRelationshipMapping * new_mapping );
    /** Looks at ability tags and gathers additional required and blocking tags */
    void GetAdditionalActivationTagRequirements( const FGameplayTagContainer & ability_tags, FGameplayTagContainer & activation_required_tags, FGameplayTagContainer & activation_blocked_tags ) const;
    void AbilityInputTagPressed( FGameplayTag input_tag );
    void AbilityInputTagReleased( FGameplayTag input_tag );
    void ProcessAbilityInput( float delta_time, bool game_is_paused );
    void ClearAbilityInput();

#if WITH_EDITOR
    EDataValidationResult IsDataValid( FDataValidationContext & context ) const override;
#endif

    UFUNCTION( BlueprintPure )
    FGameplayAbilitySpecHandle FindAbilitySpecHandleForClass( const TSubclassOf< UGameplayAbility > & ability_class );

    UFUNCTION( BlueprintCallable )
    void OurCancelAllAbilities();

    // Will cancel any active ability that has this tag
    UFUNCTION( BlueprintCallable )
    void CancelAbilitiesByTag( FGameplayTag tag, bool replicate_cancel_ability );

    UFUNCTION( BlueprintCallable, Category = "GameplayCue", Meta = ( AutoCreateRefTerm = "parameters", GameplayTagFilter = "GameplayCue" ) )
    void ExecuteGameplayCueLocal( const FGameplayTag gameplay_cue_tag, const FGameplayCueParameters & parameters );

    UFUNCTION( BlueprintCallable, Category = "GameplayCue", Meta = ( AutoCreateRefTerm = "parameters", GameplayTagFilter = "GameplayCue" ) )
    void AddGameplayCueLocal( const FGameplayTag gameplay_cue_tag, const FGameplayCueParameters & parameters );

    UFUNCTION( BlueprintCallable, Category = "GameplayCue", Meta = ( AutoCreateRefTerm = "parameters", GameplayTagFilter = "GameplayCue" ) )
    void RemoveGameplayCueLocal( const FGameplayTag gameplay_cue_tag, const FGameplayCueParameters & parameters );

    /** Cancels the ability indicated by passed in spec handle. If handle is not found among reactivated abilities nothing happens. */
    UFUNCTION( BlueprintCallable, Category = "GameplayAbility", DisplayName = "CancelAbilityByHandle" )
    void K2_CancelAbilityHandle( const FGameplayAbilitySpecHandle & ability_handle );

    // ----------------------------------------------------------------------------------------------------------------
    //	AnimMontage Support for multiple USkeletalMeshComponents on the AvatarActor.
    //  Only one ability can be animating at a time though?
    // ----------------------------------------------------------------------------------------------------------------

    // Plays a montage and handles replication and prediction based on passed in ability/activation info
    float PlayMontageForMesh( UGameplayAbility * animating_ability, class USkeletalMeshComponent * mesh, UAnimMontage * new_anim_montage, float play_rate, FName start_section_name = NAME_None, bool must_replicate_montage = true );

    // Plays a montage without updating replication/prediction structures. Used by simulated proxies when replication tells them to play a montage.
    float PlayMontageSimulatedForMesh( USkeletalMeshComponent * mesh, UAnimMontage * new_anim_montage, float play_rate, FName start_section_name = NAME_None );

    // Stops whatever montage is currently playing. Expectation is caller should only be stopping it if they are the current animating ability (or have good reason not to check)
    void CurrentMontageStopForMesh( USkeletalMeshComponent * mesh, float override_blend_out_time = -1.0f );

    // Stops all montages currently playing
    void StopAllCurrentMontages( float override_blend_out_time = -1.0f );

    // Stops current montage if it's the one given as the Montage param
    void StopMontageIfCurrentForMesh( USkeletalMeshComponent * mesh, const UAnimMontage & montage, float override_blend_out_time = -1.0f );

    // Clear the animating ability that is passed in, if it's still currently animating
    void ClearAnimatingAbilityForAllMeshes( UGameplayAbility * ability );

    // Jumps current montage to given section. Expectation is caller should only be stopping it if they are the current animating ability (or have good reason not to check)
    void CurrentMontageJumpToSectionForMesh( USkeletalMeshComponent * mesh, const FName section_name );

    // Sets current montages next section name. Expectation is caller should only be stopping it if they are the current animating ability (or have good reason not to check)
    void CurrentMontageSetNextSectionNameForMesh( USkeletalMeshComponent * mesh, const FName from_section_name, const FName to_section_name );

    // Sets current montage's play rate
    void CurrentMontageSetPlayRateForMesh( USkeletalMeshComponent * mesh, const float play_rate );

    // Returns true if the passed in ability is the current animating ability
    bool IsAnimatingAbilityForAnyMesh( UGameplayAbility * ability ) const;

    // Returns the current animating ability
    UGameplayAbility * GetAnimatingAbilityFromAnyMesh() const;

    // Returns montages that are currently playing
    TArray< UAnimMontage * > GetCurrentMontages() const;

    // Returns the montage that is playing for the mesh
    UAnimMontage * GetCurrentMontageForMesh( USkeletalMeshComponent * mesh ) const;

    // Get SectionID of currently playing AnimMontage
    int32 GetCurrentMontageSectionIDForMesh( USkeletalMeshComponent * mesh ) const;

    // Get SectionName of currently playing AnimMontage
    FName GetCurrentMontageSectionNameForMesh( USkeletalMeshComponent * mesh ) const;

    // Get length in time of current section
    float GetCurrentMontageSectionLengthForMesh( USkeletalMeshComponent * mesh ) const;

    // Returns amount of time left in current section
    float GetCurrentMontageSectionTimeLeftForMesh( USkeletalMeshComponent * mesh ) const;

    template < typename _ATTRIBUTE_SET_CLASS_ >
    _ATTRIBUTE_SET_CLASS_ * GetAttributeSet();

    bool IsActivationGroupBlocked( EGBFAbilityActivationGroup group ) const;
    void AddAbilityToActivationGroup( EGBFAbilityActivationGroup group, UGBFGameplayAbility * ability );
    void RemoveAbilityFromActivationGroup( EGBFAbilityActivationGroup group, const UGBFGameplayAbility * ability );
    void CancelActivationGroupAbilities( EGBFAbilityActivationGroup group, UGBFGameplayAbility * ignore_ability, bool replicate_cancel_ability );

    typedef TFunctionRef< bool( const UGBFGameplayAbility * ability, FGameplayAbilitySpecHandle handle ) > TShouldCancelAbilityFunc;
    void CancelAbilitiesByFunc( const TShouldCancelAbilityFunc & predicate, bool replicate_cancel_ability );

    void CancelInputActivatedAbilities( bool replicate_cancel_ability );

    void AbilitySpecInputPressed( FGameplayAbilitySpec & spec ) override;
    void AbilitySpecInputReleased( FGameplayAbilitySpec & spec ) override;

protected:
    void TryActivateAbilitiesOnSpawn();
    void NotifyAbilityActivated( const FGameplayAbilitySpecHandle Handle, UGameplayAbility * Ability ) override;
    void NotifyAbilityFailed( const FGameplayAbilitySpecHandle Handle, UGameplayAbility * Ability, const FGameplayTagContainer & FailureReason ) override;
    void NotifyAbilityEnded( FGameplayAbilitySpecHandle handle, UGameplayAbility * ability, bool was_cancelled ) override;

    UFUNCTION( BlueprintCallable )
    void K2_RemoveGameplayCue( FGameplayTag gameplay_cue_tag );

    UFUNCTION( BlueprintCallable )
    void K2_ExecuteGameplayCueWithEffectContext( FGameplayTag gameplay_cue_tag, FGameplayEffectContextHandle effect_context );

    UFUNCTION( BlueprintCallable )
    void K2_ExecuteGameplayCueWithParameters( FGameplayTag gameplay_cue_tag, const FGameplayCueParameters & parameters );

    UFUNCTION( BlueprintCallable )
    void K2_AddGameplayCueWithEffectContext( FGameplayTag gameplay_cue_tag, FGameplayEffectContextHandle effect_context );

    UFUNCTION( BlueprintCallable )
    void K2_AddGameplayCueWithParameters( FGameplayTag gameplay_cue_tag, const FGameplayCueParameters & parameters );

private:
    // ----------------------------------------------------------------------------------------------------------------
    //	AnimMontage Support for multiple USkeletalMeshComponents on the AvatarActor.
    //  Only one ability can be animating at a time though?
    // ----------------------------------------------------------------------------------------------------------------

    // Set if montage rep happens while we don't have the animinstance associated with us yet
    UPROPERTY()
    bool bPendingMontageRepForMesh;

    // Data structure for montages that were instigated locally (everything if server, predictive if client. replicated if simulated proxy)
    // Will be max one element per skeletal mesh on the AvatarActor
    UPROPERTY()
    TArray< FGameplayAbilityLocalAnimMontageForMesh > LocalAnimMontageInfoForMeshes;

    // Data structure for replicating montage info to simulated clients
    // Will be max one element per skeletal mesh on the AvatarActor
    UPROPERTY( ReplicatedUsing = OnRep_ReplicatedAnimMontageForMesh )
    TArray< FGameplayAbilityRepAnimMontageForMesh > RepAnimMontageInfoForMeshes;

    // Finds the existing FGameplayAbilityLocalAnimMontageForMesh for the mesh or creates one if it doesn't exist
    FGameplayAbilityLocalAnimMontageForMesh & GetLocalAnimMontageInfoForMesh( USkeletalMeshComponent * mesh );
    // Finds the existing FGameplayAbilityRepAnimMontageForMesh for the mesh or creates one if it doesn't exist
    FGameplayAbilityRepAnimMontageForMesh & GetGameplayAbilityRepAnimMontageForMesh( USkeletalMeshComponent * mesh );

    // Called when a prediction key that played a montage is rejected
    void OnPredictiveMontageRejectedForMesh( USkeletalMeshComponent * mesh, UAnimMontage * predictive_montage ) const;

    // Copy LocalAnimMontageInfo into RepAnimMontageInfo
    void AnimMontage_UpdateReplicatedDataForMesh( USkeletalMeshComponent * mesh );
    void AnimMontage_UpdateReplicatedDataForMesh( FGameplayAbilityRepAnimMontageForMesh & rep_anim_montage_info );

    // Copy over playing flags for duplicate animation data
    void AnimMontage_UpdateForcedPlayFlagsForMesh( FGameplayAbilityRepAnimMontageForMesh & rep_anim_montage_info );

    UFUNCTION()
    void OnRep_ReplicatedAnimMontageForMesh();

    // Returns true if we are ready to handle replicated montage information
    bool IsReadyForReplicatedMontageForMesh() const;

    // RPC function called from CurrentMontageSetNextSectionName, replicates to other clients
    UFUNCTION( Reliable, Server, WithValidation )
    void ServerCurrentMontageSetNextSectionNameForMesh( USkeletalMeshComponent * mesh, UAnimMontage * client_anim_montage, const float client_position, const FName section_name, const FName next_section_name );
    void ServerCurrentMontageSetNextSectionNameForMesh_Implementation( USkeletalMeshComponent * mesh, UAnimMontage * client_anim_montage, const float client_position, const FName section_name, const FName next_section_name );
    bool ServerCurrentMontageSetNextSectionNameForMesh_Validate( USkeletalMeshComponent * mesh, UAnimMontage * client_anim_montage, const float client_position, const FName section_name, const FName next_section_name );

    // RPC function called from CurrentMontageJumpToSection, replicates to other clients
    UFUNCTION( Reliable, Server, WithValidation )
    void ServerCurrentMontageJumpToSectionNameForMesh( USkeletalMeshComponent * mesh, UAnimMontage * client_anim_montage, const FName section_name );
    void ServerCurrentMontageJumpToSectionNameForMesh_Implementation( USkeletalMeshComponent * mesh, UAnimMontage * client_anim_montage, const FName section_name );
    bool ServerCurrentMontageJumpToSectionNameForMesh_Validate( USkeletalMeshComponent * mesh, UAnimMontage * client_anim_montage, const FName section_name );

    // RPC function called from CurrentMontageSetPlayRate, replicates to other clients
    UFUNCTION( Reliable, Server, WithValidation )
    void ServerCurrentMontageSetPlayRateForMesh( USkeletalMeshComponent * mesh, UAnimMontage * client_anim_montage, const float play_rate );
    void ServerCurrentMontageSetPlayRateForMesh_Implementation( USkeletalMeshComponent * mesh, UAnimMontage * client_anim_montage, const float play_rate );
    bool ServerCurrentMontageSetPlayRateForMesh_Validate( USkeletalMeshComponent * mesh, UAnimMontage * client_anim_montage, const float play_rate );

    // If set, this table is used to look up tag relationships for activate and cancel
    UPROPERTY()
    UGBFAbilityTagRelationshipMapping * TagRelationshipMapping;

    TArray< FGameplayAbilitySpecHandle > InputPressedSpecHandles;
    TArray< FGameplayAbilitySpecHandle > InputReleasedSpecHandles;
    TArray< FGameplayAbilitySpecHandle > InputHeldSpecHandles;

    // Number of abilities running in each activation group.
    int32 ActivationGroupCounts[ static_cast< uint8 >( EGBFAbilityActivationGroup::MAX ) ];
};

template < typename _ATTRIBUTE_SET_CLASS_ >
_ATTRIBUTE_SET_CLASS_ * UGBFAbilitySystemComponent::GetAttributeSet()
{
    for ( auto * attribute_set : GetSpawnedAttributes() )
    {
        if ( auto * swarms_attribute_set = Cast< _ATTRIBUTE_SET_CLASS_ >( attribute_set ) )
        {
            return swarms_attribute_set;
        }
    }

    return nullptr;
}

FORCEINLINE bool UGBFAbilitySystemComponent::ShouldDoServerAbilityRPCBatch() const
{
    // :TODO:
    return false;
}