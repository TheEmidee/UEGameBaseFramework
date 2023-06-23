#pragma once

#include <CoreMinimal.h>
#include <GameFramework/Actor.h>

#include "ModularActor.generated.h"

UCLASS()
class MODULARGAMEPLAYACTORS_API AModularActor : public AActor
{
    GENERATED_BODY()

public:
    AModularActor();

    void PreInitializeComponents() override;
    void BeginPlay() override;

protected:
    void EndPlay( const EEndPlayReason::Type EndPlayReason ) override;
};
