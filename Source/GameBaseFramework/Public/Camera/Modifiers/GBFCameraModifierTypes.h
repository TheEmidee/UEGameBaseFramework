#pragma once

#include <CoreMinimal.h>

#include "GBFCameraModifierTypes.generated.h"

UENUM()
enum class EGBFCameraModifierAttributeOperation : uint8
{
    Add,
    Multiply,
    Override
};