#pragma once

#include "Characters/Components/GBFPawnComponent.h"

#include <CoreMinimal.h>
#include <GameplayAbilitySpecHandle.h>

#include "GBFHeroComponent.generated.h"

struct FGBFInputMappingContextAndPriority;
struct FGBFMappableConfigPair;
class UGBFInputComponent;
class UGBFInputConfig;
class UGBFCameraMode;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFHeroComponent : public UGBFPawnComponent
{
    GENERATED_BODY()

public:
    UGBFHeroComponent();

    /** True if this player has sent the BindInputsNow event and is prepared for bindings */
    bool IsReadyToBindInputs() const;

    FName GetFeatureName() const override;
    bool CanChangeInitState( UGameFrameworkComponentManager * manager, FGameplayTag current_state, FGameplayTag desired_state ) const override;
    void HandleChangeInitState( UGameFrameworkComponentManager * manager, FGameplayTag current_state, FGameplayTag desired_state ) override;
    void OnActorInitStateChanged( const FActorInitStateChangedParams & params ) override;

    /** Adds mode-specific input config */
    void AddAdditionalInputConfig( const UGBFInputConfig * input_config );

    /** Removes a mode-specific input config if it has been added */
    void RemoveAdditionalInputConfig( const UGBFInputConfig * input_config );

    static const FName NAME_BindInputsNow;
    static const FName NAME_ActorFeatureName;

    UFUNCTION( BlueprintPure, Category = "GameBaseFramework|Hero" )
    static UGBFHeroComponent * FindHeroComponent( const AActor * actor );

    /** Overrides the camera from an active gameplay ability */
    void SetAbilityCameraMode( TSubclassOf< UGBFCameraMode > camera_mode, const FGameplayAbilitySpecHandle & owning_spec_handle );

    /** Clears the camera override if it is set */
    void ClearAbilityCameraMode( const FGameplayAbilitySpecHandle & owning_spec_handle );

protected:
    void OnRegister() override;
    void BindToRequiredOnActorInitStateChanged() override;
    void InitializePlayerInput( UInputComponent * player_input_component );
    void Input_AbilityInputTagPressed( FGameplayTag input_tag );
    void Input_AbilityInputTagReleased( FGameplayTag input_tag );
    virtual void BindNativeActions( UGBFInputComponent * input_component, const UGBFInputConfig * input_config );

private:
    TSubclassOf< UGBFCameraMode > DetermineCameraMode() const;

    FSimpleMulticastDelegate::FDelegate OnPawnReadyToInitializeDelegate;

    // True when player input bindings have been applyed, will never be true for non-players
    bool bReadyToBindInputs;

    /**
     * Input Configs that should be added to this player when initializing the input. These configs
     * will NOT be registered with the settings because they are added at runtime. If you want the config
     * pair to be in the settings, then add it via the GameFeatureAction_AddInputConfig
     *
     * NOTE: You should only add to this if you do not have a game feature plugin accessible to you.
     * If you do, then use the GameFeatureAction_AddInputConfig instead.
     */

    PRAGMA_DISABLE_DEPRECATION_WARNINGS
    UE_DEPRECATED( 5.3, "DefaultInputConfigs have been deprecated, use DefaultInputMappings instead" )
    TArray< FGBFMappableConfigPair > DefaultInputConfigs;
    PRAGMA_ENABLE_DEPRECATION_WARNINGS

    UPROPERTY( EditAnywhere )
    TArray< FGBFInputMappingContextAndPriority > DefaultInputMappings;

    /** Camera mode set by an ability. */
    UPROPERTY()
    TSubclassOf< UGBFCameraMode > AbilityCameraMode;

    /** Spec handle for the last ability to set a camera mode. */
    FGameplayAbilitySpecHandle AbilityCameraModeOwningSpecHandle;
};

FORCEINLINE bool UGBFHeroComponent::IsReadyToBindInputs() const
{
    return bReadyToBindInputs;
}