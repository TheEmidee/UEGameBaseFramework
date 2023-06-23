#pragma once

#include <GameFramework/PlayerController.h>

#include "ModularPlayerController.generated.h"

/** Minimal class that supports extension by game feature plugins */
UCLASS( Blueprintable )
class MODULARGAMEPLAYACTORS_API AModularPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    //~ Begin AActor interface
    void PreInitializeComponents() override;
    void EndPlay( const EEndPlayReason::Type EndPlayReason ) override;
    //~ End AActor interface

    //~ Begin APlayerController interface
    void ReceivedPlayer() override;
    void PlayerTick( float DeltaTime ) override;
    //~ End APlayerController interface
};
