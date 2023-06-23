#pragma once

#include <GameFramework/PlayerState.h>

#include "ModularPlayerState.generated.h"

/** Minimal class that supports extension by game feature plugins */
UCLASS( Blueprintable )
class MODULARGAMEPLAYACTORS_API AModularPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    //~ Begin AActor interface
    void PreInitializeComponents() override;
    void BeginPlay() override;
    void EndPlay( const EEndPlayReason::Type EndPlayReason ) override;
    void Reset() override;
    //~ End AActor interface

protected:
    //~ Begin APlayerState interface
    void CopyProperties( APlayerState * PlayerState );
    //~ End APlayerState interface
};
