#pragma once

#include <GameFramework/Pawn.h>

#include "ModularPawn.generated.h"

/** Minimal class that supports extension by game feature plugins */
UCLASS( Blueprintable )
class MODULARGAMEPLAYACTORS_API AModularPawn : public APawn
{
    GENERATED_BODY()

public:
    //~ Begin AActor interface
    void PreInitializeComponents() override;
    void BeginPlay() override;
    void EndPlay( const EEndPlayReason::Type EndPlayReason ) override;
    //~ End AActor interface

    void UnPossessed() override;
    void PossessedBy( AController * new_controller ) override;
};
