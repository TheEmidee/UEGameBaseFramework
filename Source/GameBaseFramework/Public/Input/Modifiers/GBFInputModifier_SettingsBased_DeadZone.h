#pragma once

#include <CoreMinimal.h>
#include <InputModifiers.h>

#include "GBFInputModifier_SettingsBased_DeadZone.generated.h"

/** Represents which stick that this deadzone is for, either the move or the look stick */
UENUM()
enum class EDeadzoneStick : uint8
{
    /** Deadzone for the movement stick */
    MoveStick = 0,

    /** Deadzone for the looking stick */
    LookStick = 1,
};

/**
 * Deadzone input modifier that will have it's thresholds driven by what is in the shared game settings.
 * */
UCLASS( NotBlueprintable, MinimalAPI, meta = ( DisplayName = "Settings Based Dead Zone" ) )
class UGBFInputModifier_SettingsBased_DeadZone final : public UInputModifier
{
    GENERATED_BODY()

public:
    UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = Settings, Config )
    EDeadZoneType Type = EDeadZoneType::Radial;

    // Threshold above which input is clamped to 1
    UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = Settings, Config )
    float UpperThreshold = 1.0f;

    /** Which stick this deadzone is for. This controls which setting will be used when calculating the deadzone */
    UPROPERTY( EditInstanceOnly, BlueprintReadWrite, Category = Settings, Config )
    EDeadzoneStick DeadZoneStick = EDeadzoneStick::MoveStick;

protected:
    FInputActionValue ModifyRaw_Implementation( const UEnhancedPlayerInput * player_input, FInputActionValue current_value, float delta_time ) override;

    // Visualize as black when unmodified. Red when blocked (with differing intensities to indicate axes)
    // Mirrors visualization in https://www.gamasutra.com/blogs/JoshSutphin/20130416/190541/Doing_Thumbstick_Dead_Zones_Right.php.
    FLinearColor GetVisualizationColor_Implementation( FInputActionValue sample_value, FInputActionValue final_value ) const override;
};
