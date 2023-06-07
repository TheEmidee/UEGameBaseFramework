#include "Input/Modifiers/GBFInputModifierHelpers.h"

#include "Engine/GBFLocalPlayer.h"

#include <EnhancedPlayerInput.h>
#include <GameFramework/PlayerController.h>

namespace GBFInputModifiersHelpers
{
    UGBFLocalPlayer * GetLocalPlayer( const UEnhancedPlayerInput * player_input )
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