#include "GBFPlayerController.h"

#include "Components/GBFPlatformInputSwitcherComponent.h"
#include "Components/GBFUIDialogManagerComponent.h"

AGBFPlayerController::AGBFPlayerController()
{
#if PLATFORM_DESKTOP
    PlatformInputSwitcherComponent = CreateDefaultSubobject< UGBFPlatformInputSwitcherComponent >( TEXT( "PlatformInputSwitcherComponent" ) );
#endif

    UIDialogManagerComponent = CreateDefaultSubobject< UGBFUIDialogManagerComponent >( TEXT( "UIDialogManagerComponent" ) );
}