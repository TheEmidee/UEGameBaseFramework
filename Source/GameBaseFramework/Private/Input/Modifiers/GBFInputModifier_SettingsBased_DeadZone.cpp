#include "Input/Modifiers/GBFInputModifier_SettingsBased_DeadZone.h"

#include "Engine/GBFLocalPlayer.h"
#include "Input/Modifiers/GBFInputModifierHelpers.h"

FInputActionValue UGBFInputModifier_SettingsBased_DeadZone::ModifyRaw_Implementation( const UEnhancedPlayerInput * player_input, const FInputActionValue current_value, float delta_time )
{
    const auto value_type = current_value.GetValueType();
    const auto * local_player = GBFInputModifiersHelpers::GetLocalPlayer( player_input );
    if ( value_type == EInputActionValueType::Boolean || !local_player )
    {
        return current_value;
    }

    const auto * settings = local_player->GetSharedSettings();
    ensure( settings != nullptr );

    auto lower_threshold = DeadZoneStick == EDeadzoneStick::MoveStick
                               ? settings->GetGamepadMoveStickDeadZone()
                               : settings->GetGamepadLookStickDeadZone();

    lower_threshold = FMath::Clamp( lower_threshold, 0.0f, 1.0f );

    auto dead_zone_lambda = [ lower_threshold, this ]( const float axis_val ) {
        // We need to translate and scale the input to the +/- 1 range after removing the dead zone.
        return FMath::Min( 1.f, ( FMath::Max( 0.f, FMath::Abs( axis_val ) - lower_threshold ) / ( UpperThreshold - lower_threshold ) ) ) * FMath::Sign( axis_val );
    };

    FVector new_value = current_value.Get< FVector >();
    switch ( Type )
    {
        case EDeadZoneType::Axial:
        {
            new_value.X = dead_zone_lambda( new_value.X );
            new_value.Y = dead_zone_lambda( new_value.Y );
            new_value.Z = dead_zone_lambda( new_value.Z );
        }
        break;
        case EDeadZoneType::Radial:
        {
            if ( value_type == EInputActionValueType::Axis3D )
            {
                new_value = new_value.GetSafeNormal() * dead_zone_lambda( new_value.Size() );
            }
            else if ( value_type == EInputActionValueType::Axis2D )
            {
                new_value = new_value.GetSafeNormal2D() * dead_zone_lambda( new_value.Size2D() );
            }
            else
            {
                new_value.X = dead_zone_lambda( new_value.X );
            }
        }
        break;
    }

    return new_value;
}

FLinearColor UGBFInputModifier_SettingsBased_DeadZone::GetVisualizationColor_Implementation( FInputActionValue SampleValue, FInputActionValue FinalValue ) const
{
    // Taken from UInputModifierDeadZone::GetVisualizationColor_Implementation
    if ( FinalValue.GetValueType() == EInputActionValueType::Boolean || FinalValue.GetValueType() == EInputActionValueType::Axis1D )
    {
        return FLinearColor( FinalValue.Get< float >() == 0.f ? 1.f : 0.f, 0.f, 0.f );
    }
    return FLinearColor( ( FinalValue.Get< FVector2D >().X == 0.f ? 0.5f : 0.f ) + ( FinalValue.Get< FVector2D >().Y == 0.f ? 0.5f : 0.f ), 0.f, 0.f );
}
