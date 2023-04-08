#pragma once

#include <CoreMinimal.h>
#include <UObject/Interface.h>

#include "GBFPawnComponentReadyInterface.generated.h"

UINTERFACE( MinimalAPI )
class UGBFPawnComponentReadyInterface : public UInterface
{
    GENERATED_BODY()
};

// :TODO: Remove when everything works
class GAMEBASEFRAMEWORK_API IGBFPawnComponentReadyInterface
{
    GENERATED_BODY()

public:

    virtual bool IsPawnComponentReadyToInitialize() const = 0;
};
