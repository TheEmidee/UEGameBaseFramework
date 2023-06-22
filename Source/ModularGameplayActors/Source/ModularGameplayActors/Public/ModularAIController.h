#pragma once

#include <AIController.h>

#include "ModularAIController.generated.h"

UCLASS( Blueprintable )
class MODULARGAMEPLAYACTORS_API AModularAIController : public AAIController
{
    GENERATED_BODY()

public:
    //~ Begin AActor Interface
    void PreInitializeComponents() override;
    void BeginPlay() override;
    void EndPlay( const EEndPlayReason::Type EndPlayReason ) override;
    //~ End AActor Interface
};
