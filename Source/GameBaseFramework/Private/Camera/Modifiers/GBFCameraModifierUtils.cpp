#include "Camera/Modifiers/GBFCameraModifierUtils.h"

#include <GameFramework/Actor.h>
#include <Kismet/KismetSystemLibrary.h>

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

bool FGBFCameraModifierUtilsLibrary::LineTraceSingleByChannel( AActor * view_target, FHitResult & hit_result, const FVector & start, const FVector & end, const TEnumAsByte< ECollisionChannel > collision_channel, bool draw_debug )
{
    const auto draw_debug_type = draw_debug ? EDrawDebugTrace::ForOneFrame : EDrawDebugTrace::None;
    const auto object_type_query = UCollisionProfile::Get()->ConvertToTraceType( collision_channel );
    return UKismetSystemLibrary::LineTraceSingle( view_target, start, end, object_type_query, false, { view_target }, draw_debug_type, hit_result, false );
}