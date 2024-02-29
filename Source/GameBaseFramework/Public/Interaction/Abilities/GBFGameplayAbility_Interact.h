#pragma once

#include "GAS/Abilities/GBFGameplayAbility.h"
#include "Interaction/GBFInteractionOption.h"

#include "GBFGameplayAbility_Interact.generated.h"

class UGBFInputComponent;
class UEnhancedInputLocalPlayerSubsystem;
class UEnhancedInputUserSettings;
class UGBFIndicatorDescriptor;
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
    struct InputConfigInfos
    {
        bool IsValid() const;

        TWeakObjectPtr< UGBFHeroComponent > HeroComponent;
        TWeakObjectPtr< UGBFInputConfig > InputConfig;
    };

    struct InputMappingContextInfos
    {
        bool IsValid() const;

        TWeakObjectPtr< UEnhancedInputLocalPlayerSubsystem > EnhancedSystem;
        TWeakObjectPtr< UInputMappingContext > InputMappingContext;
    };

    struct InputBindingInfos
    {
        TWeakObjectPtr< UGBFInputComponent > InputComponent;
        uint32 Handle;
    };

    struct OptionHandle
    {
        TScriptInterface< IGBFInteractableTarget > InteractableTarget;
        TWeakObjectPtr< UAbilitySystemComponent > TargetAbilitySystem;
        FGameplayAbilitySpecHandle InteractionAbilityHandle;
    };

    struct WidgetInfosHandle
    {
        TScriptInterface< IGBFInteractableTarget > InteractableTarget;
        FGBFInteractionWidgetInfos WidgetInfos;
    };

    struct Context
    {
        void Reset();

        TArray< WidgetInfosHandle > WidgetInfosHandles;
        TArray< OptionHandle > OptionHandles;
        TArray< InputConfigInfos > InputConfigInfos;
        TArray< InputMappingContextInfos > InputMappingContextInfos;
        TArray< FGameplayAbilitySpecHandle > GrantedAbilities;
        TArray< InputBindingInfos > BindActionHandles;
    };

    UFUNCTION( BlueprintImplementableEvent )
    void LookForInteractables();

    UPROPERTY()
    TArray< TObjectPtr< UGBFIndicatorDescriptor > > Indicators;

    UPROPERTY( EditDefaultsOnly )
    float InteractionScanRate;

    UPROPERTY( EditDefaultsOnly )
    float InteractionScanRange;

    UPROPERTY( EditDefaultsOnly )
    TEnumAsByte< ECollisionChannel > InteractionTraceChannel;

    UPROPERTY( EditDefaultsOnly )
    TSoftClassPtr< UUserWidget > DefaultInteractionWidgetClass;

    Context Context;

private:
    void UpdateInteractableOptions( const TArray< TScriptInterface< IGBFInteractableTarget > > & interactable_targets );
    void OnPressCallBack( OptionHandle interaction_option );
};