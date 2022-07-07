#pragma once

#include "ModularGameState.h"

#include <AbilitySystemInterface.h>
#include <CoreMinimal.h>

#include "GBFGameState.generated.h"

class UGASExtAbilitySystemComponent;
UCLASS()
class GAMEBASEFRAMEWORK_API AGBFGameState : public AModularGameState, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:

    AGBFGameState();

    UAbilitySystemComponent * GetAbilitySystemComponent() const override;
    void PostInitializeComponents() override;
    void Tick( float DeltaSeconds ) override;

private:

    UPROPERTY( VisibleAnywhere )
    UGASExtAbilitySystemComponent * AbilitySystemComponent;

    UPROPERTY( Replicated )
    float ServerFPS;
};
