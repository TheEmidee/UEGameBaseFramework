#include "Camera/Modifiers/GBFCameraModifierVelocityToYaw.h"

#include "Camera/Modifiers/GBFCameraModifierUtils.h"

#include <Camera/PlayerCameraManager.h>
#include <Engine/Canvas.h>
#include <Engine/World.h>
#include <Kismet/KismetStringLibrary.h>
#include <Kismet/KismetSystemLibrary.h>

UGBFCameraModifierVelocityToYaw::UGBFCameraModifierVelocityToYaw() :
    InterpolationSpeed( 1.0f ),
    bUseInterpolationSpeedMultiplierFromTime( false ),
    bUseManualRotationCooldown( false ),
    ManualRotationCooldown( 2.0f ),
    VelocityScale( 1.0f, 1.0f, 0.0f ),
    bUseYawCorrectionMultiplierFromVelocity( false ),
    bCanRotateToFaceViewTarget( 1 ),
    FaceViewTargetAngleThreshold( 145.0f ),
    InterpolationSpeedMultiplierTime( 0.0f ),
    InterpolationSpeedMultiplierFromTime( 0.0f ),
    FinalInterpolationSpeed( 0.0f ),
    ManualRotationCooldownRemainingTime( 0.0f ),
    ViewTargetVelocity( 0.0f ),
    YawCorrectionMultiplierFromVelocity( 0.0f ),
    RotationDifference( 0 ),
    Dot( 0.0f ),
    CurrentYaw( 0.0f ),
    TargetYaw( 0.0f )
{
}

bool UGBFCameraModifierVelocityToYaw::ProcessViewRotation( AActor * view_target, float delta_time, FRotator & view_rotation, FRotator & delta_rotation )
{
    const auto * vt = GetViewTarget();

    if ( vt == nullptr )
    {
        return false;
    }

    CurrentYaw = view_rotation.Yaw;
    const auto vt_velocity = vt->GetVelocity() * VelocityScale;

    const auto actor_yaw = view_target->GetActorRotation().Yaw;
    const auto view_yaw = view_rotation.Yaw;

    TargetYaw = actor_yaw;
    RotationDifference = FMath::Abs( FGBFCameraModifierUtilsLibrary::ClampAngle( actor_yaw - view_yaw ) );
    Dot = FVector::DotProduct( vt->GetVelocity().GetSafeNormal(), view_rotation.Vector().GetSafeNormal() );

    if ( bCanRotateToFaceViewTarget && Dot < 0.0f )
    {
        TargetYaw = FGBFCameraModifierUtilsLibrary::ClampAngle( TargetYaw + 180.0f );
    }

    ViewTargetVelocity = vt_velocity.Size();

    if ( bUseManualRotationCooldown )
    {
        if ( !delta_rotation.IsZero() )
        {
            ManualRotationCooldownRemainingTime = ManualRotationCooldown;
            return false;
        }

        if ( ManualRotationCooldownRemainingTime >= 0.0f )
        {
            ManualRotationCooldownRemainingTime -= delta_time;
            InterpolationSpeedMultiplierTime = 0.0f;
            return false;
        }
    }

    FinalInterpolationSpeed = InterpolationSpeed;

    if ( bUseInterpolationSpeedMultiplierFromTime )
    {
        InterpolationSpeedMultiplierTime += delta_time;
        InterpolationSpeedMultiplierFromTime = InterpolationSpeedMultiplierFromTimeCurve.GetRichCurveConst()->Eval( InterpolationSpeedMultiplierTime );
        FinalInterpolationSpeed *= InterpolationSpeedMultiplierFromTime;
    }

    if ( bUseYawCorrectionMultiplierFromVelocity )
    {
        YawCorrectionMultiplierFromVelocity = YawCorrectionMultiplierFromVelocityCurve.GetRichCurveConst()->Eval( ViewTargetVelocity );
        FinalInterpolationSpeed *= YawCorrectionMultiplierFromVelocity;
    }

    const auto rot = FMath::RInterpTo( FRotator( 0.0f, CurrentYaw, 0.0f ), FRotator( 0.0f, TargetYaw, 0.0f ), delta_time, FinalInterpolationSpeed );
    delta_rotation.Yaw = rot.Yaw - CurrentYaw;

    return false;
}

void UGBFCameraModifierVelocityToYaw::DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const
{
    auto & display_debug_manager = canvas->DisplayDebugManager;

    if ( bUseManualRotationCooldown )
    {
        display_debug_manager.DrawString( FString::Printf( TEXT( "ManualRotationCooldown: %s" ), *FString::SanitizeFloat( ManualRotationCooldown ) ) );
        display_debug_manager.DrawString( FString::Printf( TEXT( "ManualRotationCooldownRemainingTime: %s" ), *FString::SanitizeFloat( ManualRotationCooldownRemainingTime ) ) );
    }

    if ( ManualRotationCooldownRemainingTime > 0.0f )
    {
        display_debug_manager.SetDrawColor( FColor::White );
    }

    display_debug_manager.DrawString( FString::Printf( TEXT( "Initial InterpolationSpeed: %s" ), *FString::SanitizeFloat( InterpolationSpeed ) ) );

    if ( bUseInterpolationSpeedMultiplierFromTime )
    {
        display_debug_manager.DrawString( FString::Printf( TEXT( "InterpolationSpeedMultiplierTime: %s" ), *FString::SanitizeFloat( InterpolationSpeedMultiplierTime ) ) );
        display_debug_manager.DrawString( FString::Printf( TEXT( "InterpolationSpeedMultiplierFromTime: %s" ), *FString::SanitizeFloat( InterpolationSpeedMultiplierFromTime ) ) );
    }

    display_debug_manager.DrawString( FString::Printf( TEXT( "Final Interpolation Speed: %s" ), *FString::SanitizeFloat( FinalInterpolationSpeed ) ) );

    if ( bUseYawCorrectionMultiplierFromVelocity )
    {
        display_debug_manager.DrawString( FString::Printf( TEXT( "ViewTargetVelocity: %s" ), *FString::SanitizeFloat( ViewTargetVelocity ) ) );
        display_debug_manager.DrawString( FString::Printf( TEXT( "YawCorrectionMultiplierFromVelocity: %s" ), *FString::SanitizeFloat( YawCorrectionMultiplierFromVelocity ) ) );
    }

    display_debug_manager.DrawString( FString::Printf( TEXT( "Velocity Scale: %s" ), *VelocityScale.ToCompactString() ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "RotationDifference: %s" ), *FString::SanitizeFloat( RotationDifference ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "Can rotate to face ViewTarget: %s" ), *UKismetStringLibrary::Conv_BoolToString( bCanRotateToFaceViewTarget ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "Target Yaw: %s" ), *FString::SanitizeFloat( TargetYaw ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "Current Yaw: %s" ), *FString::SanitizeFloat( CurrentYaw ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "Dot : %s" ), *FString::SanitizeFloat( Dot ) ) );

    const auto * view_target = GetViewTarget();
    const auto vt_location = view_target->GetActorLocation();
    UKismetSystemLibrary::DrawDebugArrow( CameraOwner->GetWorld(), vt_location, vt_location + view_target->GetActorForwardVector() * 100.0f, 10.0f, FLinearColor::Red );
}