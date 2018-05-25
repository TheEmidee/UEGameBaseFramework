#include "GBFUMGBlueprintLibrary.h"

#include "Blueprint/UserWidget.h"

#include "Components/GBFPlatformInputSwitcherComponent.h"
#include "GameFramework/GBFPlayerController.h"

UGBFPlatformInputSwitcherComponent * UGBFUMGBlueprintLibrary::GetPlatformInputSwitcherComponentFromOwningPlayer( bool & success, UUserWidget * widget )
{
    if ( auto * pc = Cast< AGBFPlayerController >( widget->GetOwningPlayer() ) )
    {
        if ( auto * input_switcher = pc->GetPlatformInputSwitcherComponent() )
        {
            success = true;
            return input_switcher;
        }
    }

    success = false;
    return nullptr;
}