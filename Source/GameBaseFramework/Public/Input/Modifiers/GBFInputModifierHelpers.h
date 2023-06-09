#pragma once

#include "Engine/GBFLocalPlayer.h"

#include <CoreMinimal.h>
#include <EnhancedPlayerInput.h>
#include <GameFramework/PlayerController.h>

class UEnhancedPlayerInput;

namespace GBFInputModifiersHelpers
{
    /** Returns the owning local player of an Enhanced Player Input pointer */
    static UGBFLocalPlayer * GetLocalPlayer( const UEnhancedPlayerInput * player_input )
    {
        if ( player_input )
        {
            if ( const auto * pc = Cast< APlayerController >( player_input->GetOuter() ) )
            {
                return Cast< UGBFLocalPlayer >( pc->GetLocalPlayer() );
            }
        }
        return nullptr;
    }
}
