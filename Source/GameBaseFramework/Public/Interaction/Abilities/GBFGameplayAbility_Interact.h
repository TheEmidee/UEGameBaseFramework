#pragma once

#include "GAS/Abilities/GBFGameplayAbility.h"
#include "Interaction/GBFInteractionOption.h"

#include "GBFGameplayAbility_Interact.generated.h"

class UGBFInteractableComponent;
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
    void EndAbility( const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo * ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled ) override;

    UFUNCTION( BlueprintCallable )
    void UpdateInteractions( const FGameplayAbilityTargetDataHandle & target_data_handle );
    void ResetAllInteractions();

protected:
    UFUNCTION( BlueprintImplementableEvent )
    void LookForInteractables();

private:
    struct InputMappingContextInfos
    {
        InputMappingContextInfos() = default;

        InputMappingContextInfos( const TWeakObjectPtr< UEnhancedInputLocalPlayerSubsystem > & EnhancedSystem, const TWeakObjectPtr< UInputMappingContext > & InputMappingContext ) :
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

        InputBindingInfos( const TWeakObjectPtr< UGBFInputComponent > & InputComponent, uint32 Handle ) :
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

        OptionHandle( const TWeakObjectPtr< UGBFInteractableComponent > & interactable_component, const TWeakObjectPtr< UAbilitySystemComponent > & target_ability_system, const FGameplayAbilitySpecHandle & interaction_ability_handle, const TWeakObjectPtr< UGBFInteractionEventCustomization > & event_customization ) :
            InteractableComponent( interactable_component ),
            TargetAbilitySystem( target_ability_system ),
            InteractionAbilityHandle( interaction_ability_handle ),
            EventCustomization( event_customization )
        {
        }

        TWeakObjectPtr< UGBFInteractableComponent > InteractableComponent;
        TWeakObjectPtr< UAbilitySystemComponent > TargetAbilitySystem;
        FGameplayAbilitySpecHandle InteractionAbilityHandle;
        TWeakObjectPtr< UGBFInteractionEventCustomization > EventCustomization;
    };

    struct WidgetInfosHandle
    {
        WidgetInfosHandle() = default;

        WidgetInfosHandle( const TWeakObjectPtr< UGBFInteractableComponent > & interactable_component, const FGBFInteractionWidgetInfos & widget_infos ) :
            InteractableComponent( interactable_component ),
            WidgetInfos( widget_infos )
        {
        }

        TWeakObjectPtr< UGBFInteractableComponent > InteractableComponent;
        FGBFInteractionWidgetInfos WidgetInfos;
    };

    struct InteractableTargetContext
    {
        void Reset();

        TArray< WidgetInfosHandle > WidgetInfosHandles;
        TArray< OptionHandle > OptionHandles;
        TArray< InputMappingContextInfos > InputMappingContextInfos;
        TArray< InputBindingInfos > BindActionHandles;
    };

    struct InteractableTargetInfos
    {
        InteractableTargetInfos() = default;

        InteractableTargetInfos( const TWeakObjectPtr< AActor > & actor, const TWeakObjectPtr< UGBFInteractableComponent > & interactable_component, const EGBFInteractionGroup group ) :
            Actor( actor ),
            InteractableComponent( interactable_component ),
            Group( group )
        {
        }

        TWeakObjectPtr< AActor > Actor;
        TWeakObjectPtr< UGBFInteractableComponent > InteractableComponent;
        EGBFInteractionGroup Group;
    };

    void UpdateInteractableOptions( const TArray< UGBFInteractableComponent * > & interactable_components );
    void OnPressCallBack( OptionHandle interaction_option );
    void UpdateIndicators();
    void GetTargetInfos( TArray< InteractableTargetInfos > & target_infos, const TArray< UGBFInteractableComponent * > & interactable_components ) const;
    void ResetUnusedInteractions( const TArray< InteractableTargetInfos > & target_infos );
    void RegisterInteractions( const TArray< InteractableTargetInfos > & target_infos );
    void RegisterInteraction( const InteractableTargetInfos & target_infos );

    UPROPERTY()
    TArray< TObjectPtr< UGBFIndicatorDescriptor > > Indicators;

    UPROPERTY( EditDefaultsOnly )
    float InteractionScanRate;

    UPROPERTY( EditDefaultsOnly )
    float InteractionScanRange;

    UPROPERTY( EditDefaultsOnly )
    TEnumAsByte< ECollisionChannel > InteractionTraceChannel;

    TMap< TWeakObjectPtr< AActor >, InteractableTargetContext > InteractableTargetContexts;
};