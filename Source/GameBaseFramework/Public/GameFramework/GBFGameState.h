#pragma once

#include "Experiences/GBFExperienceManagerComponent.h"
#include "ModularGameState.h"

#include <AbilitySystemInterface.h>
#include <CoreMinimal.h>

#include "GBFGameState.generated.h"

class UGBFAbilitySystemComponent;

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFGameState : public AModularGameState, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    AGBFGameState();

    UGBFExperienceManagerComponent * GetExperienceManagerComponent() const;
    UAbilitySystemComponent * GetAbilitySystemComponent() const override;
    void PostInitializeComponents() override;
    void Tick( float delta_seconds ) override;
    void SeamlessTravelTransitionCheckpoint( bool to_transition ) override;

private:
    UPROPERTY()
    UGBFExperienceManagerComponent * ExperienceManagerComponent;

    UPROPERTY( VisibleAnywhere )
    UGBFAbilitySystemComponent * AbilitySystemComponent;

    UPROPERTY( Replicated )
    float ServerFPS;
};

FORCEINLINE UGBFExperienceManagerComponent * AGBFGameState::GetExperienceManagerComponent() const
{
    return ExperienceManagerComponent;
}