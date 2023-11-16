#pragma once

#include "GAS/Components/GBFAbilitySystemComponent.h"

#include <Abilities/Tasks/AbilityTask.h>
#include <Animation/AnimMontage.h>
#include <CoreMinimal.h>

#include "GBFAT_PlayMontageForMeshAndWaitForEvent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FSWOnPlayMontageForMeshAndWaitForEventDelegate, FGameplayTag, event_tag, FGameplayEventData, event_data );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAT_PlayMontageForMeshAndWaitForEvent final : public UAbilityTask
{
    GENERATED_BODY()

public:
    UGBFAT_PlayMontageForMeshAndWaitForEvent();

    void Activate() override;
    void ExternalCancel() override;
    FString GetDebugString() const override;
    void OnDestroy( bool ability_ended ) override;

    /**
	 * Play a montage and wait for it end. If a gameplay event happens that matches EventTags (or EventTags is empty), the EventReceived delegate will fire with a tag and event data.
	 * If StopWhenAbilityEnds is true, this montage will be aborted if the ability ends normally. It is always stopped when the ability is explicitly cancelled.
	 * On normal execution, OnBlendOut is called when the montage is blending out, and OnCompleted when it is completely done playing
	 * OnInterrupted is called if another montage overwrites this, and OnCancelled is called if the ability or task is cancelled
	 *
	 * @param task_instance_name Set to override the name of this task, for later querying
	 * @param montage_to_play The montage to play on the character
	 * @param event_tags Any gameplay events matching this tag will activate the EventReceived callback. If empty, all events will trigger callback
	 * @param rate Change to play the montage faster or slower
	 * @param stop_when_ability_ends If true, this montage will be aborted if the ability ends normally. It is always stopped when the ability is explicitly cancelled
	 * @param anim_root_motion_translation_scale Change to modify size of root motion or set to 0 to block it entirely
	 */
    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = "TRUE" ) )
    static UGBFAT_PlayMontageForMeshAndWaitForEvent * PlayMontageForMeshAndWaitForEvent(
        UGameplayAbility * owning_ability,
        FName task_instance_name,
        USkeletalMeshComponent * mesh,
        UAnimMontage * montage_to_play,
        FGameplayTagContainer event_tags,
        float rate = 1.f,
        FName start_section = NAME_None,
        bool stop_when_ability_ends = true,
        float anim_root_motion_translation_scale = 1.0f,
        bool replicate_montage = true,
        float override_blend_out_time_for_cancel_ability = -1.0f,
        float override_blend_out_time_for_stop_when_end_ability = -1.0f );

protected:
    /** The montage completely finished playing */
    UPROPERTY( BlueprintAssignable )
    FSWOnPlayMontageForMeshAndWaitForEventDelegate OnCompleted;

    /** The montage started blending out */
    UPROPERTY( BlueprintAssignable )
    FSWOnPlayMontageForMeshAndWaitForEventDelegate OnBlendOut;

    /** The montage was interrupted */
    UPROPERTY( BlueprintAssignable )
    FSWOnPlayMontageForMeshAndWaitForEventDelegate OnInterrupted;

    /** The ability task was explicitly cancelled by another ability */
    UPROPERTY( BlueprintAssignable )
    FSWOnPlayMontageForMeshAndWaitForEventDelegate OnCancelled;

    /** One of the triggering gameplay events happened */
    UPROPERTY( BlueprintAssignable )
    FSWOnPlayMontageForMeshAndWaitForEventDelegate EventReceived;

private:
    /** Checks if the ability is playing a montage and stops that montage, returns true if a montage was stopped, false if not. */
    bool StopPlayingMontage( float override_blend_out_time = -1.f ) const;

    /** Returns our ability system component */
    UGBFAbilitySystemComponent * GetTargetASC() const;

    void OnMontageBlendingOut( UAnimMontage * montage, bool was_interrupted ) const;
    void OnAbilityCancelled() const;
    void OnMontageEnded( UAnimMontage * montage, bool was_interrupted );
    void OnGameplayEvent( FGameplayTag event_tag, const FGameplayEventData * payload ) const;

    // Mesh that the Montage is playing on. Must be owned by the AvatarActor.
    UPROPERTY()
    USkeletalMeshComponent * Mesh;

    /** Montage that is playing */
    UPROPERTY()
    UAnimMontage * MontageToPlay;

    /** List of tags to match against gameplay events */
    UPROPERTY()
    FGameplayTagContainer EventTags;

    /** Playback rate */
    UPROPERTY()
    float Rate;

    /** Section to start montage from */
    UPROPERTY()
    FName StartSection;

    /** Modifies how root motion movement to apply */
    UPROPERTY()
    float AnimRootMotionTranslationScale;

    /** Rather montage should be aborted if ability ends */
    UPROPERTY()
    bool StopsWhenAbilityEnds;

    UPROPERTY()
    bool MustReplicateMontage;

    UPROPERTY()
    float OverrideBlendOutTimeForCancelAbility;

    UPROPERTY()
    float OverrideBlendOutTimeForStopWhenEndAbility;

    FOnMontageBlendingOutStarted BlendingOutDelegate;
    FOnMontageEnded MontageEndedDelegate;
    FDelegateHandle CancelledHandle;
    FDelegateHandle EventHandle;
};