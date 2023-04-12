#pragma once

#include "Characters/Components/GBFPawnComponent.h"

#include <CoreMinimal.h>

#include "GBFHeroComponent.generated.h"

class UGBFInputComponent;
class UGBFInputConfig;

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

protected:
    void OnRegister() override;
    void BindToRequiredOnActorInitStateChanged() override;
    void InitializePlayerInput( UInputComponent * player_input_component );
    void Input_AbilityInputTagPressed( FGameplayTag input_tag );
    void Input_AbilityInputTagReleased( FGameplayTag input_tag );
    virtual void BindNativeActions( UGBFInputComponent * input_component, const UGBFInputConfig * input_config );

private:
    FSimpleMulticastDelegate::FDelegate OnPawnReadyToInitializeDelegate;

    // True when player input bindings have been applyed, will never be true for non-players
    bool bReadyToBindInputs;
};

FORCEINLINE bool UGBFHeroComponent::IsReadyToBindInputs() const
{
    return bReadyToBindInputs;
}