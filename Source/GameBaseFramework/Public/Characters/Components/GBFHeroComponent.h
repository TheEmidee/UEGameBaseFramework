#pragma once

#include "Characters/Components/GBFPawnComponent.h"

#include <CoreMinimal.h>
#include <GameplayAbilitySpecHandle.h>

#include "GBFHeroComponent.generated.h"

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
    virtual void InitializePlayerInput( UInputComponent * player_input_component );

private:
    TSubclassOf< UGBFCameraMode > DetermineCameraMode() const;

    FSimpleMulticastDelegate::FDelegate OnPawnReadyToInitializeDelegate;

    // True when player input bindings have been applyed, will never be true for non-players
    bool bReadyToBindInputs;

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