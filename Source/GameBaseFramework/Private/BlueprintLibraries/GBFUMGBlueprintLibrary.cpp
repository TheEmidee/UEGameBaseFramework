#include "GBFUMGBlueprintLibrary.h"

#include "Blueprint/UserWidget.h"

#include "Components/GBFPlatformInputSwitcherComponent.h"
#include "Engine/GBFLocalPlayer.h"
#include "GameFramework/GBFPlayerController.h"
#include "GameFramework/GBFSaveGame.h"

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

UGBFSaveGame * UGBFUMGBlueprintLibrary::GetSaveGameFromOwningPlayer( bool & success, UUserWidget * widget )
{
    if ( auto * pc = Cast< AGBFPlayerController >( widget->GetOwningPlayer() ) )
    {
        if ( auto * lc = pc->GetGBFLocalPlayer() )
        {
            success = true;
            return lc->GetSaveGame();
        }
    }

    success = false;
    return nullptr;
}