#include "Input/GBFInputAimSensitivityData.h"

UGBFInputAimSensitivityData::UGBFInputAimSensitivityData( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    SensitivityMap = {
        { EGBFGamepadSensitivity::Slow, 0.5f },
        { EGBFGamepadSensitivity::SlowPlus, 0.75f },
        { EGBFGamepadSensitivity::SlowPlusPlus, 0.9f },
        { EGBFGamepadSensitivity::Normal, 1.0f },
        { EGBFGamepadSensitivity::NormalPlus, 1.1f },
        { EGBFGamepadSensitivity::NormalPlusPlus, 1.25f },
        { EGBFGamepadSensitivity::Fast, 1.5f },
        { EGBFGamepadSensitivity::FastPlus, 1.75f },
        { EGBFGamepadSensitivity::FastPlusPlus, 2.0f },
        { EGBFGamepadSensitivity::Insane, 2.5f },
    };
}

float UGBFInputAimSensitivityData::SensitivtyEnumToFloat( const EGBFGamepadSensitivity gamepad_sensitivity ) const
{
    if ( const auto * sensitivity = SensitivityMap.Find( gamepad_sensitivity ) )
    {
        return *sensitivity;
    }

    return 1.0f;
}
