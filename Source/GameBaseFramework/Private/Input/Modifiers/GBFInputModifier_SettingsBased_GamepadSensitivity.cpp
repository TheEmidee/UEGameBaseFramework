#include "Input/Modifiers/GBFInputModifier_SettingsBased_GamepadSensitivity.h"

#include "Engine/GBFLocalPlayer.h"
#include "Input/GBFInputAimSensitivityData.h"
#include "Input/Modifiers/GBFInputModifierHelpers.h"

FInputActionValue UGBFInputModifier_SettingsBased_GamepadSensitivity::ModifyRaw_Implementation( const UEnhancedPlayerInput * player_input, const FInputActionValue current_value, float delta_time )
{
    // You can't scale a boolean action type
    const auto * local_player = GBFInputModifiersHelpers::GetLocalPlayer( player_input );
    if ( current_value.GetValueType() == EInputActionValueType::Boolean || !local_player || !SensitivityLevelTable )
    {
        return current_value;
    }

    const auto * settings = local_player->GetSharedSettings();
    if ( !ensure( settings != nullptr ) )
    {
        return current_value;
    }

    const auto sensitivity = settings->GetGamepadSensitivityPreset( TargetingTypeTag );
    const auto scalar = SensitivityLevelTable->SensitivtyEnumToFloat( sensitivity );

    return current_value.Get< FVector >() * scalar;
}
