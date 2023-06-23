#pragma once

#include <GameFramework/GameMode.h>

#include "ModularGameMode.generated.h"

/** Pair this with a ModularGameStateBase */
UCLASS( Blueprintable )
class MODULARGAMEPLAYACTORS_API AModularGameModeBase : public AGameModeBase
{
    GENERATED_BODY()

public:
    AModularGameModeBase();

    void PreInitializeComponents() override;

protected:
    void BeginPlay() override;
    void EndPlay( const EEndPlayReason::Type EndPlayReason ) override;
};

/** Pair this with a ModularGameState */
UCLASS( Blueprintable )
class MODULARGAMEPLAYACTORS_API AModularGameMode : public AGameMode
{
    GENERATED_BODY()

public:
    AModularGameMode();

    bool ReadyToStartMatch_Implementation() override;
    void PreInitializeComponents() override;
    void InitGameState() override;

protected:
    void BeginPlay() override;
    void EndPlay( const EEndPlayReason::Type EndPlayReason ) override;
    void HandleMatchHasStarted() override;
};