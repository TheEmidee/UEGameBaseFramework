#pragma once

#include "GAS/GBFGameplayAbility.h"

#include "GBFGameplayAbility_Interact.generated.h"

struct FGBFInteractionOption;
class IGBFInteractableTarget;
class UGBFAT_WaitForInteractableTargets;

/**
 * UGBFGameplayAbility_Interact
 *
 * Gameplay ability used for character interacting
 */
UCLASS( Abstract )
class GAMEBASEFRAMEWORK_API UGBFGameplayAbility_Interact : public UGBFGameplayAbility
{
    GENERATED_BODY()

public:
    UGBFGameplayAbility_Interact();

    void ActivateAbility( const FGameplayAbilitySpecHandle handle, const FGameplayAbilityActorInfo * actor_info, const FGameplayAbilityActivationInfo activation_info, const FGameplayEventData * trigger_event_data ) override;

    UFUNCTION( BlueprintCallable )
    void UpdateInteractions( const FGameplayAbilityTargetDataHandle & target_data_handle );

    UFUNCTION( BlueprintCallable )
    void TriggerInteraction();

protected:
    UFUNCTION( BlueprintImplementableEvent )
    void LookForInteractables();

    UPROPERTY( BlueprintReadWrite )
    TArray< FGBFInteractionOption > CurrentOptions;

    // :TODO: Add Indicator system
    // UPROPERTY()
    // TArray< TObjectPtr< UIndicatorDescriptor > > Indicators;

    UPROPERTY( EditDefaultsOnly )
    float InteractionScanRate;

    UPROPERTY( EditDefaultsOnly )
    float InteractionScanRange;

    UPROPERTY( EditDefaultsOnly )
    TEnumAsByte< ECollisionChannel > InteractionTraceChannel;

    UPROPERTY( EditDefaultsOnly )
    TSoftClassPtr< UUserWidget > DefaultInteractionWidgetClass;

private:
    void UpdateInteractableOptions( const TArray< TScriptInterface< IGBFInteractableTarget > > & interactable_targets );
};