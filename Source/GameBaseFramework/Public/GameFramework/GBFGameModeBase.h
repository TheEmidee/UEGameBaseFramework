#pragma once

#include "GameFramework/GameModeBase.h"
#include "GBFGameModeBase.generated.h"

UCLASS()

class GAMEBASEFRAMEWORK_API AGBFGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:

    void StartPlay() override;
    void EndPlay( EEndPlayReason::Type reason ) override;

    virtual bool CanPauseGame() const;

    virtual void HandleAppSuspended();
    virtual void HandleAppResumed();
};
