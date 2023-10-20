#pragma once

#include "GBFGameMode.h"

#include "GBFGameModeLogin.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFGameModeLogin : public AGBFGameMode
{
    GENERATED_BODY()

public:
    void StartPlay() override;

    virtual void InitializeLocalPlayer( int controller_index );

    UFUNCTION( BlueprintImplementableEvent )
    void ReceiveInitializeLocalPlayer( int controller_index );
};
