#pragma once

#include "GameFramework/GameModeBase.h"
#include "GBFGameModeBase.generated.h"

UCLASS()

class GAMEBASEFRAMEWORK_API AGBFGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:

    virtual bool CanPauseGame() const;

    virtual void HandleAppSuspended();
    virtual void HandleAppResumed();
};
