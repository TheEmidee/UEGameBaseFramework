#include "Camera/Modifiers/GBFCameraModifierClearSight.h"

#include "Camera/Modifiers/GBFCameraModifierUtils.h"

#include <Camera/PlayerCameraManager.h>
#include <Engine/Canvas.h>
#include <Engine/HitResult.h>

UGBFCameraModifierClearSight::UGBFCameraModifierClearSight() :
    LineOfSightProbeRadius( 25.0f ),
    LineOfSightProbeChannel( ECC_Camera ),
    MaximumAngle( 90.0f ),
    StepSizeAngle( 15.0f ),
    InterpolationSpeed( 2.0f ),
    bUseVelocitySpeedInterpolationMultiplier( false ),
    bUseManualRotationCooldown( true ),
    ManualRotationCooldown( 5.0f ),
    CurrentYaw( 0.0f ),
    TargetYaw( 0.0f ),
    AngleCorrection( 0.0f ),
    ManualRotationCooldownRemainingTime( 0.0f ),
    CurrentInterpolationSpeed( 0.0f ),
    ViewTargetVelocity( 0.0f )
{
}

bool UGBFCameraModifierClearSight::ProcessViewRotation( AActor * view_target, float delta_time, FRotator & view_rotation, FRotator & delta_rotation )
{
    const auto view_location = CameraOwner->GetCameraLocation();
    auto * vt = GetViewTarget();
    if ( vt == nullptr )
    {
        return false;
    }

    if ( bUseManualRotationCooldown )
    {
        if ( !FMath::IsNearlyZero( delta_rotation.Pitch ) )
        {
            ManualRotationCooldownRemainingTime = ManualRotationCooldown;
            return false;
        }

        if ( ManualRotationCooldownRemainingTime >= 0.0f )
        {
            ManualRotationCooldownRemainingTime -= delta_time;
            return false;
        }
    }

    CurrentYaw = view_rotation.Yaw;

    AngleCorrection = 0.0f;
    auto check_angle = -StepSizeAngle;

    while ( check_angle >= -MaximumAngle )
    {
        if ( IsLineOfSightObstructed( { vt, view_location, view_rotation, check_angle } ) )
        {
            // We found an obstacle for this angle.
            // The smaller the absolute value of check_angle,
            // the closer the obstacle, and the further we need to swing away.
            AngleCorrection += check_angle + MaximumAngle;
            break;
        }

        check_angle -= StepSizeAngle;
    }

    check_angle = StepSizeAngle;

    while ( check_angle <= MaximumAngle )
    {
        if ( IsLineOfSightObstructed( { vt, view_location, view_rotation, check_angle } ) )
        {
            AngleCorrection += check_angle - MaximumAngle;
            break;
        }

        // Look further away.
        check_angle += StepSizeAngle;
    }

    if ( FMath::IsNearlyZero( AngleCorrection ) )
    {
        return false;
    }

    TargetYaw = AngleCorrection;

    CurrentInterpolationSpeed = InterpolationSpeed;

    if ( bUseVelocitySpeedInterpolationMultiplier )
    {
        ViewTargetVelocity = vt->GetVelocity().Size2D();
        VelocityInterpolationSpeedMultiplier = VelocitySpeedInterpolationMultiplierCurve.GetRichCurveConst()->Eval( ViewTargetVelocity );
        CurrentInterpolationSpeed *= VelocityInterpolationSpeedMultiplier;
    }

    const auto applied_angle = FMath::Clamp( AngleCorrection, -CurrentInterpolationSpeed * delta_time, CurrentInterpolationSpeed * delta_time );
    delta_rotation.Yaw += FMath::RadiansToDegrees( applied_angle );

    return false;
}

void UGBFCameraModifierClearSight::DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const
{
    auto & display_debug_manager = canvas->DisplayDebugManager;

    display_debug_manager.DrawString( FString::Printf( TEXT( "LineOfSightProbeRadius: %s" ), *FString::SanitizeFloat( LineOfSightProbeRadius ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "MaximumAngle: %s" ), *FString::SanitizeFloat( MaximumAngle ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "StepSizeAngle: %s" ), *FString::SanitizeFloat( StepSizeAngle ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "CurrentYaw: %s" ), *FString::SanitizeFloat( CurrentYaw ) ) );

    if ( ManualRotationCooldownRemainingTime > 0.0f )
    {
        display_debug_manager.DrawString( FString::Printf( TEXT( "ManualRotationCooldownRemainingTime: %s" ), *FString::SanitizeFloat( ManualRotationCooldownRemainingTime ) ) );
        return;
    }

    display_debug_manager.DrawString( FString::Printf( TEXT( "AngleCorrection: %s" ), *FString::SanitizeFloat( AngleCorrection ) ) );

    if ( bUseVelocitySpeedInterpolationMultiplier )
    {
        display_debug_manager.DrawString( FString::Printf( TEXT( "ViewTargetVelocity: %s" ), *FString::SanitizeFloat( ViewTargetVelocity ) ) );
        display_debug_manager.DrawString( FString::Printf( TEXT( "VelocityInterpolationSpeedMultiplier: %s" ), *FString::SanitizeFloat( VelocityInterpolationSpeedMultiplier ) ) );
    }

    display_debug_manager.DrawString( FString::Printf( TEXT( "CurrentInterpolationSpeed: %s" ), *FString::SanitizeFloat( CurrentInterpolationSpeed ) ) );

    if ( !FMath::IsNearlyZero( AngleCorrection ) )
    {
        display_debug_manager.DrawString( FString::Printf( TEXT( "TargetYaw: %s" ), *FString::SanitizeFloat( TargetYaw ) ) );
    }
}

bool UGBFCameraModifierClearSight::IsLineOfSightObstructed( const LineOfSightCheckParameters & parameters ) const
{
    FHitResult hit_result;

    const auto vt_location = parameters.ViewTarget->GetActorLocation();
    auto trace_end = parameters.CameraLocation;

    const auto angle_sin = FMath::Sin( FMath::DegreesToRadians( parameters.TraceAngle ) );
    const auto angle_cos = FMath::Cos( FMath::DegreesToRadians( parameters.TraceAngle ) );

    // Translate point to origin.
    trace_end.X -= vt_location.X;
    trace_end.Y -= vt_location.Y;

    // Rotate point.
    const auto rotated_x = trace_end.X * angle_cos - trace_end.Y * angle_sin;
    const auto rotated_y = trace_end.X * angle_sin + trace_end.Y * angle_cos;

    // Translate point back.
    trace_end.X = rotated_x + vt_location.X;
    trace_end.Y = rotated_y + vt_location.Y;
    trace_end.Z = vt_location.Z;

    if ( !FGBFCameraModifierUtilsLibrary::SphereTraceSingleByChannel( parameters.ViewTarget, hit_result, vt_location, trace_end, LineOfSightProbeRadius, LineOfSightProbeChannel, bDebug ) )
    {
        return false;
    }

    const auto dot = FVector::DotProduct( hit_result.Normal, FVector::UpVector );

    return FMath::IsNearlyZero( dot );
}