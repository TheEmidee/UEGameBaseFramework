#pragma once

#include <GameFramework/Character.h>

#include "ModularCharacter.generated.h"

UCLASS( Blueprintable )
class MODULARGAMEPLAYACTORS_API AModularCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    //~ Begin AActor Interface
    void PreInitializeComponents() override;
    void BeginPlay() override;
    void EndPlay( const EEndPlayReason::Type EndPlayReason ) override;
    //~ End AActor Interface

    void UnPossessed() override;
    void PossessedBy( AController * new_controller ) override;
};
