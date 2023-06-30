#pragma once

#include <CoreMinimal.h>
#include <GameFramework/GameSession.h>

#include "GBFGameSession.generated.h"

/**
Acts as a game-specific wrapper around the session interface. The game code makes calls to this when it needs to interact with the session interface.
A game session exists only the server, while running an online game.
*/
UCLASS()
// ReSharper disable once CppClassCanBeFinal
class GAMEBASEFRAMEWORK_API AGBFGameSession : public AGameSession
{
    GENERATED_UCLASS_BODY()

public:
    bool ProcessAutoLogin() override;
};