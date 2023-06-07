#pragma once

#include "CoreMinimal.h"

class UEnhancedPlayerInput;

namespace GBFInputModifiersHelpers
{
    /** Returns the owning local player of an Enhanced Player Input pointer */
    static UGBFLocalPlayer * GetLocalPlayer( const UEnhancedPlayerInput * player_input );
}
