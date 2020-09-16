#pragma once

#include <GameFramework/GameModeBase.h>

#include "GBFGameModeBase.generated.h"

UCLASS()

class GAMEBASEFRAMEWORK_API AGBFGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:

    AGBFGameModeBase();

    void StartPlay() override;
    void EndPlay( const EEndPlayReason::Type reason ) override;

    virtual bool CanPauseGame() const;
};
