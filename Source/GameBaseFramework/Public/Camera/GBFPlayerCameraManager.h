#pragma once

#include <Camera/PlayerCameraManager.h>
#include <CoreMinimal.h>

#include "GBFPlayerCameraManager.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFPlayerCameraManager : public APlayerCameraManager
{
    GENERATED_BODY()

protected:
    void DisplayDebug( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) override;
};
