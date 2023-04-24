#pragma once

#include "Interaction/GBFInteractionQuery.h"
#include "Interaction/Tasks/GBFAT_WaitForInteractableTargets.h"

#include "GBFAT_WaitForInteractableTargets_SingleLineTrace.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAT_WaitForInteractableTargets_SingleLineTrace : public UGBFAT_WaitForInteractableTargets
{
    GENERATED_BODY()

    UGBFAT_WaitForInteractableTargets_SingleLineTrace( const FObjectInitializer & object_initializer );

    void Activate() override;

    /** Wait until we trace new set of interactables.  This task automatically loops. */
    UFUNCTION( BlueprintCallable, Category = "Ability|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = true ) )
    static UGBFAT_WaitForInteractableTargets_SingleLineTrace * WaitForInteractableTargets_SingleLineTrace(
        UGameplayAbility * owning_ability,
        FGBFInteractionQuery interaction_query,
        FCollisionProfileName trace_profile,
        FGameplayAbilityTargetingLocationInfo start_location,
        float interaction_scan_range = 100,
        float interaction_scan_rate = 0.100,
        bool aim_with_player_controller = true,
        bool show_debug = false );

private:
    void OnDestroy( bool ability_ended ) override;
    void PerformTrace();

    UPROPERTY()
    FGBFInteractionQuery InteractionQuery;

    UPROPERTY()
    FGameplayAbilityTargetingLocationInfo StartLocation;

    UPROPERTY()
    uint8 bAimWithPlayerController : 1;

    float InteractionScanRange = 100;
    float InteractionScanRate = 0.100;
    bool bShowDebug = false;

    FTimerHandle TimerHandle;
};