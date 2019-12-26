#include "BlueprintLibraries/GBFUMGBlueprintLibrary.h"

#include "Components/GBFPlatformInputSwitcherComponent.h"
#include "Components/GBFUIDialogManagerComponent.h"
#include "Engine/GBFLocalPlayer.h"
#include "GameFramework/GBFPlayerController.h"
#include "GameFramework/GBFSaveGame.h"

#include <Blueprint/UserWidget.h>

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

UGBFUIDialogManagerComponent * UGBFUMGBlueprintLibrary::GetDialogManagerComponentFromOwningPlayer( bool & success, UUserWidget * widget )
{
    if ( auto * pc = Cast< AGBFPlayerController >( widget->GetOwningPlayer() ) )
    {
        if ( auto * dialog_manager = pc->GetUIDialogManagerComponent() )
        {
            success = true;
            return dialog_manager;
        }
    }

    success = false;
    return nullptr;
}