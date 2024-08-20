#include "Camera/Modifiers/GBFCameraModifierUtils.h"

float FGBFCameraModifierUtilsLibrary::ClampAngle( float angle )
{
    while ( angle < -180.0f )
    {
        angle += 360.0f;
    }

    while ( angle > 180.0f )
    {
        angle -= 360.0f;
    }

    return angle;
}