#pragma once

#include "ModularPlayerControllerComponent.h"

#include <Components/GameFrameworkInitStateInterface.h>
#include <CoreMinimal.h>

#include "GBFControllerComponent.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFControllerComponent : public UModularPlayerControllerComponent, public IGameFrameworkInitStateInterface
{
    GENERATED_BODY()

public:
    void CheckDefaultInitialization() override;

    void OnPossess( APawn * pawn ) override;
    void OnUnPossess() override;

protected:
    void OnRegister() override;
    void BeginPlay() override;
    void EndPlay( const EEndPlayReason::Type end_play_reason ) override;
    virtual void BindToRequiredOnActorInitStateChanged();
};
