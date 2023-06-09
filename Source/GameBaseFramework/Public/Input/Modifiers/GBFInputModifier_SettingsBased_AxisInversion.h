#pragma once

#include <CoreMinimal.h>
#include <InputModifiers.h>

#include "GBFInputModifier_SettingsBased_AxisInversion.generated.h"

/*
 * Applies an inversion of axis values based on a setting in the shared game settings
 */
UCLASS( NotBlueprintable, MinimalAPI, meta = ( DisplayName = "Settings Based Aim Inversion" ) )
class UGBFInputModifier_SettingsBased_AxisInversion final : public UInputModifier
{
    GENERATED_BODY()

protected:
    FInputActionValue ModifyRaw_Implementation( const UEnhancedPlayerInput * player_input, FInputActionValue current_value, float delta_time ) override;
};
