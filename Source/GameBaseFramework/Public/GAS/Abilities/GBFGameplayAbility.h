#pragma once

#include "GAS/GBFAbilitySystemFunctionLibrary.h"
#include "GAS/GBFAbilityTypesBase.h"

#include <Abilities/GameplayAbility.h>
#include <CoreMinimal.h>

#include "GBFGameplayAbility.generated.h"

class UGBFAbilitySystemComponent;
class ASWCharacterPlayerBase;
class ASWCharacterBase;

USTRUCT()
struct GAMEBASEFRAMEWORK_API FGBFAbilityMeshMontage
{
    GENERATED_BODY()

    FGBFAbilityMeshMontage() :
        Mesh( nullptr ),
        Montage( nullptr )
    {
    }

    FGBFAbilityMeshMontage( USkeletalMeshComponent * mesh, UAnimMontage * montage ) :
        Mesh( mesh ),
        Montage( montage )
    {
    }

    UPROPERTY()
    USkeletalMeshComponent * Mesh;

    UPROPERTY()
    UAnimMontage * Montage;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UGBFGameplayAbility();

    EGBFAbilityActivationGroup GetActivationGroup() const;
    EGBFAbilityActivationPolicy GetActivationPolicy() const;

    UFUNCTION( BlueprintPure, meta = ( DisplayName = "GetInstancingPolicy" ) )
    TEnumAsByte< EGameplayAbilityInstancingPolicy::Type > K2_GetInstancingPolicy() const;

    UFUNCTION( BlueprintPure, DisplayName = "IsLocallyControlled" )
    bool K2_IsLocallyControlled() const;

    UFUNCTION( BlueprintPure )
    UGBFAbilitySystemComponent * GetGBFAbilitySystemComponent() const;

    UFUNCTION( BlueprintPure )
    UGBFAbilitySystemComponent * GetGBFAbilitySystemComponentFromActorInfo() const;

    UFUNCTION( BlueprintPure )
    AController * GetControllerFromActorInfo() const;

    // Same as calling K2_EndAbility. Meant for use with batching system to end the ability externally.
    void ExternalEndAbility();

    void SetCurrentMontageForMesh( USkeletalMeshComponent * mesh, UAnimMontage * current_montage );

    /** Returns the currently playing montage for this ability, if any */
    UFUNCTION( BlueprintPure, Category = Animation )
    UAnimMontage * GetCurrentMontageForMesh( USkeletalMeshComponent * mesh ) const;

    // Returns true if the requested activation group is a valid transition.
    UFUNCTION( BlueprintCallable, BlueprintPure = false, Category = "Ability", Meta = ( ExpandBoolAsExecs = "ReturnValue" ) )
    bool CanChangeActivationGroup( EGBFAbilityActivationGroup new_group ) const;

    // Tries to change the activation group.  Returns true if it successfully changed.
    UFUNCTION( BlueprintCallable, BlueprintPure = false, Category = "Ability", Meta = ( ExpandBoolAsExecs = "ReturnValue" ) )
    bool ChangeActivationGroup( EGBFAbilityActivationGroup new_group );

    virtual int32 GetInputID() const;

    void SetCanBeCanceled( bool can_be_canceled ) override;
    void TryActivateAbilityOnSpawn( const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilitySpec & spec ) const;

    virtual void OnPawnAvatarSet();
    bool DoesAbilitySatisfyTagRequirements( const UAbilitySystemComponent & ability_system_component, const FGameplayTagContainer * source_tags, const FGameplayTagContainer * target_tags, FGameplayTagContainer * optional_relevant_tags ) const override;

    UFUNCTION( BlueprintCallable, Category = "GameplayCue", Meta = ( AutoCreateRefTerm = "parameters", GameplayTagFilter = "GameplayCue" ) )
    void ExecuteGameplayCueLocal( const FGameplayTag gameplay_cue_tag, const FGameplayCueParameters & parameters );

    UFUNCTION( BlueprintCallable, Category = "GameplayCue", Meta = ( AutoCreateRefTerm = "parameters", GameplayTagFilter = "GameplayCue" ) )
    void AddGameplayCueLocal( const FGameplayTag gameplay_cue_tag, const FGameplayCueParameters & parameters );

    UFUNCTION( BlueprintCallable, Category = "GameplayCue", Meta = ( AutoCreateRefTerm = "parameters", GameplayTagFilter = "GameplayCue" ) )
    void RemoveGameplayCueLocal( const FGameplayTag gameplay_cue_tag, const FGameplayCueParameters & parameters );

    UFUNCTION( BlueprintCallable, Category = "GBF|Ability" )
    UGBFHeroComponent * GetHeroComponentFromActorInfo() const;

    // Sets the ability's camera mode.
    UFUNCTION( BlueprintCallable, Category = "GBF|Ability" )
    void SetCameraMode( TSubclassOf< UGBFCameraMode > camera_mode );

    // Clears the ability's camera mode.  Automatically called if needed when the ability ends.
    UFUNCTION( BlueprintCallable, Category = "GBF|Ability" )
    void ClearCameraMode();

protected:
    UFUNCTION( BlueprintImplementableEvent, Category = Ability, DisplayName = "OnPawnAvatarSet" )
    void K2_OnPawnAvatarSet();

    bool CanActivateAbility( const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo * actor_info, const FGameplayTagContainer * source_tags, const FGameplayTagContainer * target_tags, FGameplayTagContainer * optional_relevant_tags ) const override;
    void EndAbility( const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilityActivationInfo activation_info, bool replicate_end_ability, bool was_cancelled ) override;
    void OnGiveAbility( const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilitySpec & spec ) override;
    void OnRemoveAbility( const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilitySpec & spec ) override;

    UFUNCTION( BlueprintCallable )
    void AutoEndTaskWhenAbilityEnds( UAbilityTask * task );

    UFUNCTION( BlueprintImplementableEvent )
    void ReceiveOnGiveAbility( const FGameplayAbilityActorInfo & actor_info, const FGameplayAbilitySpec & spec );

    UFUNCTION( BlueprintImplementableEvent )
    void ReceiveOnRemoveAbility( const FGameplayAbilityActorInfo & actor_info, const FGameplayAbilitySpec & spec );

    // Override this function if you want to add tags to the optional relevant tags when an ability is blocked
    virtual void AddBlockedAbilityOptionalRelevantTags( const FGameplayTagContainer & ability_system_component_tags, FGameplayTagContainer * optional_relevant_tags ) const;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Activation" )
    EGBFAbilityActivationGroup ActivationGroup;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Activation" )
    EGBFAbilityActivationPolicy ActivationPolicy;

private:
    bool FindAbilityMeshMontage( FGBFAbilityMeshMontage & ability_mesh_montage, USkeletalMeshComponent * mesh ) const;

    /** Immediately jumps the active montage to a section */
    UFUNCTION( BlueprintCallable, Category = "Ability|Animation" )
    void MontageJumpToSectionForMesh( USkeletalMeshComponent * mesh, FName section_name );

    /** Sets pending section on active montage */
    UFUNCTION( BlueprintCallable, Category = "Ability|Animation" )
    void MontageSetNextSectionNameForMesh( USkeletalMeshComponent * mesh, FName from_section_name, FName to_section_name );

    /**
     * Stops the current animation montage.
     *
     * @param mesh
     * @param override_blend_out_time If >= 0, will override the BlendOutTime parameter on the AnimMontage instance
     */
    UFUNCTION( BlueprintCallable, Category = "Ability|Animation", Meta = ( AdvancedDisplay = "OverrideBlendOutTime" ) )
    void MontageStopForMesh( USkeletalMeshComponent * mesh, float override_blend_out_time = -1.0f );

    /**
     * Stops all currently animating montages
     *
     * @param override_blend_out_time If >= 0, will override the BlendOutTime parameter on the AnimMontage instance
     */
    UFUNCTION( BlueprintCallable, Category = "Ability|Animation", Meta = ( AdvancedDisplay = "OverrideBlendOutTime" ) )
    void MontageStopForAllMeshes( float override_blend_out_time = -1.0f );

    /** Active montages being played by this ability */
    UPROPERTY()
    TArray< FGBFAbilityMeshMontage > CurrentAbilityMeshMontages;

    TArray< UAbilityTask * > TasksToEndWhenAbilityEnds;

    // Current camera mode set by the ability.
    TSubclassOf< UGBFCameraMode > ActiveCameraMode;
};

FORCEINLINE EGBFAbilityActivationGroup UGBFGameplayAbility::GetActivationGroup() const
{
    return ActivationGroup;
}

FORCEINLINE EGBFAbilityActivationPolicy UGBFGameplayAbility::GetActivationPolicy() const
{
    return ActivationPolicy;
}

FORCEINLINE TEnumAsByte< EGameplayAbilityInstancingPolicy::Type > UGBFGameplayAbility::K2_GetInstancingPolicy() const
{
    return InstancingPolicy;
}