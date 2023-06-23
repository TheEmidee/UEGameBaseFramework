#pragma once

#include <GameFramework/GameState.h>

#include "ModularGameState.generated.h"

/** Pair this with a ModularGameModeBase */
UCLASS( Blueprintable )
class MODULARGAMEPLAYACTORS_API AModularGameStateBase : public AGameStateBase
{
    GENERATED_BODY()

public:
    //~ Begin AActor interface
    void PreInitializeComponents() override;
    void BeginPlay() override;
    void EndPlay( const EEndPlayReason::Type EndPlayReason ) override;
    //~ End AActor interface
};

/** Pair this with a ModularGameState */
UCLASS( Blueprintable )
class MODULARGAMEPLAYACTORS_API AModularGameState : public AGameState
{
    GENERATED_BODY()

public:
    //~ Begin AActor interface
    void PreInitializeComponents() override;
    void BeginPlay() override;
    void EndPlay( const EEndPlayReason::Type EndPlayReason ) override;
    //~ End AActor interface

protected:
    //~ Begin AGameState interface
    void HandleMatchHasStarted() override;
    //~ Begin AGameState interface
};
