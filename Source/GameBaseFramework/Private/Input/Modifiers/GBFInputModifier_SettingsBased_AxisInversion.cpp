#include "Input/Modifiers/GBFInputModifier_SettingsBased_AxisInversion.h"

#include "Engine/GBFLocalPlayer.h"
#include "Input/Modifiers/GBFInputModifierHelpers.h"

#include <EnhancedPlayerInput.h>

FInputActionValue UGBFInputModifier_SettingsBased_AxisInversion::ModifyRaw_Implementation( const UEnhancedPlayerInput * player_input, const FInputActionValue current_value, float delta_time )
{
    const auto * local_player = GBFInputModifiersHelpers::GetLocalPlayer( player_input );
    if ( local_player == nullptr )
    {
        return current_value;
    }

    const auto * settings = local_player->GetSharedSettings();
    ensure( settings );

    auto new_value = current_value.Get< FVector >();

    if ( settings->GetInvertVerticalAxis() )
    {
        new_value.Y *= -1.0f;
    }

    if ( settings->GetInvertHorizontalAxis() )
    {
        new_value.X *= -1.0f;
    }

    return new_value;
}
