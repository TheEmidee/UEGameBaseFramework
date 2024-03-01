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
        InputConfigInfos() = default;

        InputConfigInfos( const TWeakObjectPtr<UGBFHeroComponent> & HeroComponent, const TWeakObjectPtr<UGBFInputConfig> & InputConfig ) :
            HeroComponent( HeroComponent ),
            InputConfig( InputConfig )
        {
        }

        bool IsValid() const;

        TWeakObjectPtr< UGBFHeroComponent > HeroComponent;
        TWeakObjectPtr< UGBFInputConfig > InputConfig;
    };

    struct InputMappingContextInfos
    {
        InputMappingContextInfos() = default;

        InputMappingContextInfos( const TWeakObjectPtr<UEnhancedInputLocalPlayerSubsystem> & EnhancedSystem, const TWeakObjectPtr<UInputMappingContext> & InputMappingContext ) :
            EnhancedSystem( EnhancedSystem ),
            InputMappingContext( InputMappingContext )
        {
        }

        bool IsValid() const;

        TWeakObjectPtr< UEnhancedInputLocalPlayerSubsystem > EnhancedSystem;
        TWeakObjectPtr< UInputMappingContext > InputMappingContext;
    };

    struct InputBindingInfos
    {
        InputBindingInfos() = default;

        InputBindingInfos( const TWeakObjectPtr<UGBFInputComponent> & InputComponent, uint32 Handle ) :
            InputComponent( InputComponent ),
            Handle( Handle )
        {
        }

        TWeakObjectPtr< UGBFInputComponent > InputComponent;
        uint32 Handle;
    };

    struct OptionHandle
    {
        OptionHandle() = default;

        OptionHandle( const TScriptInterface<IGBFInteractableTarget> & InteractableTarget, const TWeakObjectPtr<UAbilitySystemComponent> & TargetAbilitySystem, const FGameplayAbilitySpecHandle & InteractionAbilityHandle ) :
            InteractableTarget( InteractableTarget ),
            TargetAbilitySystem( TargetAbilitySystem ),
            InteractionAbilityHandle( InteractionAbilityHandle )
        {
        }

        TScriptInterface< IGBFInteractableTarget > InteractableTarget;
        TWeakObjectPtr< UAbilitySystemComponent > TargetAbilitySystem;
        FGameplayAbilitySpecHandle InteractionAbilityHandle;
    };

    struct WidgetInfosHandle
    {
        WidgetInfosHandle() = default;

        WidgetInfosHandle( const TScriptInterface<IGBFInteractableTarget> & InteractableTarget, const FGBFInteractionWidgetInfos & WidgetInfos ) :
            InteractableTarget( InteractableTarget ),
            WidgetInfos( WidgetInfos )
        {
        }

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

    struct InteractableTargetInfos
    {
        InteractableTargetInfos() = default;

        InteractableTargetInfos( const TWeakObjectPtr<AActor> & Actor, EGBFInteractionGroup Group ) :
            Actor( Actor ),
            Group( Group )
        {
        }

        TWeakObjectPtr< AActor > Actor;
        EGBFInteractionGroup Group;
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
    TArray< InteractableTargetInfos > TargetInfos;

private:
    void UpdateInteractableOptions( const TArray< TScriptInterface< IGBFInteractableTarget > > & interactable_targets );
    void OnPressCallBack( OptionHandle interaction_option );
};