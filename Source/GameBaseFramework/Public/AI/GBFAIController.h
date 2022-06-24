#pragma once

#include "ModularAIController.h"

#include <CoreMinimal.h>

#include "GBFAIController.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFAIController : public AModularAIController
{
    GENERATED_BODY()

public:
    explicit AGBFAIController( const FObjectInitializer & object_initializer );

    void OnRep_PlayerState() override;
    void CleanupPlayerState() override;
    void InitPlayerState() override;

    void ServerRestartController();

protected:
    void OnUnPossess() override;
    virtual void OnPlayerStateChanged();

private:
    void BroadcastOnPlayerStateChanged();

    UPROPERTY()
    APlayerState * LastSeenPlayerState;
};
