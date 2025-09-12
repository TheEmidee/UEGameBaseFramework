#pragma once

#include "ModularGameState.h"
#include "AbilitySystemInterface.h"
#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"

#include "GBFGameState.generated.h"

class UAbilitySystemComponent;

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFGameState : public AModularGameState, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    AGBFGameState();

    FORCEINLINE UAbilitySystemComponent* GetAbilitySystemComponent() const override
    {
        return AbilitySystemComponent;
    }

    void PostInitializeComponents() override;
    void Tick(float DeltaSeconds) override;
    void SeamlessTravelTransitionCheckpoint(bool ToTransition) override;

private:
    UPROPERTY(VisibleAnywhere)
    UAbilitySystemComponent* AbilitySystemComponent;

    UPROPERTY(Replicated)
    float ServerFPS;
};
