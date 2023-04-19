#pragma once

#include "Abilities/GBFGameplayAbility.h"

#include "GBFGameplayAbility_Interact.generated.h"

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
    void UpdateInteractions( const TArray< FGBFInteractionOption > & interactive_options );

    UFUNCTION( BlueprintCallable )
    void TriggerInteraction();

protected:
    UPROPERTY( BlueprintReadWrite )
    TArray< FGBFInteractionOption > CurrentOptions;

    // :TODO: Add Indicator system
    // UPROPERTY()
    // TArray< TObjectPtr< UIndicatorDescriptor > > Indicators;

    UPROPERTY( EditDefaultsOnly )
    float InteractionScanRate = 0.1f;

    UPROPERTY( EditDefaultsOnly )
    float InteractionScanRange = 500;

    UPROPERTY( EditDefaultsOnly )
    TSoftClassPtr< UUserWidget > DefaultInteractionWidgetClass;
};