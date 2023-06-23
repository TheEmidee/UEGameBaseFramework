#pragma once

#include <Components/PawnComponent.h>
#include <CoreMinimal.h>

#include "ModularPawnComponent.generated.h"

UCLASS()
class MODULARGAMEPLAYACTORS_API UModularPawnComponent : public UPawnComponent
{
    GENERATED_BODY()

public:
    explicit UModularPawnComponent( const FObjectInitializer & object_initializer );

    virtual void OnPossessedBy( AController * new_controller );
    virtual void OnUnPossessed();
};
