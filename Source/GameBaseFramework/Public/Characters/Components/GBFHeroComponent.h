#pragma once

#include "Characters/Components/GBFPawnComponent.h"

#include <CoreMinimal.h>

#include "GBFHeroComponent.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFHeroComponent : public UGBFPawnComponent
{
    GENERATED_BODY()

public:
    UGBFHeroComponent();

    bool HasPawnInitialized() const;

    /** True if this player has sent the BindInputsNow event and is prepared for bindings */
    bool IsReadyToBindInputs() const;

    static const FName NAME_BindInputsNow;

    UFUNCTION( BlueprintPure, Category = "GameBaseFramework|Hero" )
    static UGBFHeroComponent * FindHeroComponent( const AActor * actor );

protected:
    void OnRegister() override;
    void OnUnregister() override;
    bool IsPawnComponentReadyToInitialize() const override;
    void OnPawnReadyToInitialize();
    virtual void InitializePlayerInput( UInputComponent * player_input_component );

private:
    FSimpleMulticastDelegate::FDelegate OnPawnReadyToInitializeDelegate;

    // True when the pawn has fully finished initialization
    bool bPawnHasInitialized;

    // True when player input bindings have been applyed, will never be true for non-players
    bool bReadyToBindInputs;
};

FORCEINLINE bool UGBFHeroComponent::HasPawnInitialized() const
{
    return bPawnHasInitialized;
}

FORCEINLINE bool UGBFHeroComponent::IsReadyToBindInputs() const
{
    return bReadyToBindInputs;
}