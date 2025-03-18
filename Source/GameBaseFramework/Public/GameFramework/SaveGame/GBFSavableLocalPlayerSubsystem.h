#pragma once

#include "GBFSaveGame.h"

#include <CoreMinimal.h>
#include <Subsystems/LocalPlayerSubsystem.h>

#include "GBFSavableLocalPlayerSubsystem.generated.h"

UCLASS( abstract )
class GAMEBASEFRAMEWORK_API UGBFSavableLocalPlayerSubsystem : public ULocalPlayerSubsystem, public IGBFSaveGameSystemSavableInterface
{
    GENERATED_BODY()

public:
    void Initialize( FSubsystemCollectionBase & collection ) override;
    void Deinitialize() override;

    void PlayerControllerChanged( APlayerController * new_player_controller ) override;
};
