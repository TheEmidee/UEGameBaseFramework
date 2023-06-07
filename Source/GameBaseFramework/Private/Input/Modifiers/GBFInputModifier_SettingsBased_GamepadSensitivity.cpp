#include "Input/Modifiers/GBFInputModifier_SettingsBased_GamepadSensitivity.h"

#include "Engine/GBFLocalPlayer.h"
#include "Input/GBFInputAimSensitivityData.h"
#include "Input/Modifiers/GBFInputModifierHelpers.h"

FInputActionValue UGBFInputModifier_SettingsBased_GamepadSensitivity::ModifyRaw_Implementation( const UEnhancedPlayerInput * PlayerInput, FInputActionValue CurrentValue, float DeltaTime )
{
    // You can't scale a boolean action type
    const auto * local_player = GBFInputModifiersHelpers::GetLocalPlayer( PlayerInput );
    if ( CurrentValue.GetValueType() == EInputActionValueType::Boolean || !local_player || !SensitivityLevelTable )
    {
        return CurrentValue;
    }

    const auto * settings = local_player->GetSharedSettings();
    ensure( settings != nullptr );

    const auto sensitivity = settings->GetGamepadSensitivityPreset( TargetingTypeTag );
    //== ELyraTargetingType::Normal ) ? settings->GetGamepadLookSensitivityPreset() : settings->GetGamepadTargetingSensitivityPreset();

    const float Scalar = SensitivityLevelTable->SensitivtyEnumToFloat( sensitivity );

    return CurrentValue.Get< FVector >() * Scalar;
}
