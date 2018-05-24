#include "GBFPlayerController.h"

#include "Components/GBFPlatformInputSwitcherComponent.h"

AGBFPlayerController::AGBFPlayerController()
{
    PlatformInputSwitcherComponent = CreateDefaultSubobject< UGBFPlatformInputSwitcherComponent >( TEXT( "PlatformInputSwitcherComponent" ) );
}