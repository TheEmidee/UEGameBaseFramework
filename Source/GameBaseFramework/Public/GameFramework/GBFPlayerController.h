#pragma once

#include "GameFramework/PlayerController.h"

#include "GBFPlayerController.generated.h"

class UGBFPlatformInputSwitcherComponent;

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFPlayerController : public APlayerController
{
    GENERATED_BODY()

public:

    AGBFPlayerController();

#if PLATFORM_DESKTOP
    FORCEINLINE UGBFPlatformInputSwitcherComponent * GetPlatformInputSwitcherComponent() const;
#endif

private:

    UPROPERTY( VisibleAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = "true" ) )
    UGBFPlatformInputSwitcherComponent * PlatformInputSwitcherComponent;
};

#if PLATFORM_DESKTOP

UGBFPlatformInputSwitcherComponent * AGBFPlayerController::GetPlatformInputSwitcherComponent() const
{
    return PlatformInputSwitcherComponent;
}

#endif