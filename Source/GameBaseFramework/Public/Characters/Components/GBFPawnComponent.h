#pragma once

#include "GBFPawnComponentReadyInterface.h"
#include "ModularPawnComponent.h"

#include <CoreMinimal.h>

#include "GBFPawnComponent.generated.h"

UCLASS( Blueprintable, Meta = ( BlueprintSpawnableComponent ) )
class GAMEBASEFRAMEWORK_API UGBFPawnComponent : public UModularPawnComponent, public IGBFPawnComponentReadyInterface
{
    GENERATED_BODY()

public:

    bool IsPawnComponentReadyToInitialize() const override;
};
