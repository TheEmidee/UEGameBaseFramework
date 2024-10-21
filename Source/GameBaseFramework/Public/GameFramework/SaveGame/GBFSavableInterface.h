#pragma once

#include <CoreMinimal.h>
#include <UObject/Interface.h>

#include "GBFSavableInterface.generated.h"

UINTERFACE( MinimalAPI )
class UGBFSavableInterface : public UInterface
{
    GENERATED_BODY()
};

class GAMEBASEFRAMEWORK_API IGBFSavableInterface
{
    GENERATED_BODY()

public:
    virtual FName GetSavableIdentifier() const PURE_VIRTUAL( IGBFSavableInterface::GetIdentifier, return NAME_None; );
};
