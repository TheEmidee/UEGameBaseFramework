#include "Camera/Modifiers/GBFCameraModifierFOVFromVelocity.h"

#include "Camera/Modifiers/GBFCameraModifierUtils.h"

#include <Engine/Canvas.h>

UGBFCameraModifierFOVFromVelocity::UGBFCameraModifierFOVFromVelocity() :
    VelocityScale( 1.0f, 1.0f, 0.0f ),
    bUseInterpolationSpeedCurve( false ),
    Operation( EGBFCameraModifierAttributeOperation::Add ),
    FOVInterpolationSpeed( 2.0f ),
    ViewTargetVelocity( 0.0f ),
    InitialFOV( 0.0f ),
    CurveFloatFOV( 0.0f ),
    FinalFOV( 0.0f ),
    InterpolationSpeed( 0.0f )
{
}

void UGBFCameraModifierFOVFromVelocity::ModifyCamera( float delta_time, FVector view_location, FRotator view_rotation, float fov, FVector & new_view_location, FRotator & new_view_rotation, float & new_fov )
{
    const auto * vt = GetViewTarget();

    if ( vt == nullptr )
    {
        return;
    }

    const auto vt_velocity = vt->GetVelocity() * VelocityScale;
    ViewTargetVelocity = vt_velocity.Size();

    InitialFOV = fov;
    CurveFloatFOV = VelocityToFOVCurve.GetRichCurveConst()->Eval( ViewTargetVelocity );
    FinalFOV = FGBFCameraModifierUtilsLibrary::GetAttributeOperationResult( InitialFOV, CurveFloatFOV, Operation );

    InterpolationSpeed = FOVInterpolationSpeed;

    if ( bUseInterpolationSpeedCurve )
    {
        InterpolationSpeed *= InterpolationSpeedCurve.GetRichCurveConst()->Eval( FinalFOV );
    }

    FinalFOV = FMath::FInterpTo( fov, FinalFOV, delta_time, InterpolationSpeed );
    new_fov = FinalFOV;
}

void UGBFCameraModifierFOVFromVelocity::DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const
{
    auto & display_debug_manager = canvas->DisplayDebugManager;

    display_debug_manager.DrawString( FString::Printf( TEXT( "VelocityScale : %s" ), *VelocityScale.ToCompactString() ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "ViewTargetVelocity : %s" ), *FString::SanitizeFloat( ViewTargetVelocity ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "Initial FOV: %s" ), *FString::SanitizeFloat( InitialFOV ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "Operation: %s" ), *UEnum::GetValueAsString( Operation ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "Curve FOV: %s" ), *FString::SanitizeFloat( CurveFloatFOV ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "Final FOV: %s" ), *FString::SanitizeFloat( FinalFOV ) ) );
}