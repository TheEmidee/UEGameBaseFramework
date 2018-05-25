#include "GBFPlayerController.h"

#include "Components/GBFPlatformInputSwitcherComponent.h"

AGBFPlayerController::AGBFPlayerController()
{
#if PLATFORM_DESKTOP
    PlatformInputSwitcherComponent = CreateDefaultSubobject< UGBFPlatformInputSwitcherComponent >( TEXT( "PlatformInputSwitcherComponent" ) );
#endif
}