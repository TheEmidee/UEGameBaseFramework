#pragma once

#include "GameFramework/GBFGameModeBase.h"
#include "GBFGameModeLogin.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFGameModeLogin : public AGBFGameModeBase
{
    GENERATED_BODY()

public:

    void StartPlay() override;

    virtual void InitializeLocalPlayer( int controller_index );

    UFUNCTION( BlueprintImplementableEvent )
    void ReceiveInitializeLocalPlayer( int controller_index );
};
