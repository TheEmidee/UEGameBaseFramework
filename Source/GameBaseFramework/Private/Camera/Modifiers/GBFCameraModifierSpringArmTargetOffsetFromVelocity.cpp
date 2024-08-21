#include "Camera/Modifiers/GBFCameraModifierSpringArmTargetOffsetFromVelocity.h"

#include "Camera/GBFPlayerCameraManager.h"
#include "Camera/Modifiers/GBFCameraModifierUtils.h"

#include <DrawDebugHelpers.h>
#include <Engine/Canvas.h>
#include <GameFramework/SpringArmComponent.h>

UGBFCameraModifierSpringArmTargetOffsetFromVelocity::UGBFCameraModifierSpringArmTargetOffsetFromVelocity() :
    OffsetScale( 1.0f, 1.0f, 0.0f ),
    Operation( EGBFCameraModifierAttributeOperation::Override ),
    bUseOffsetCurve( false ),
    OffsetInterpolationSpeed( 2.0f ),
    bUseInterpolationSpeedCurve( false ),
    bUseYawDifferenceCurve( 0 ),
    ViewTargetVelocity( 0.0f ),
    OffsetCurveValue( 0.0f ),
    InterpolationSpeed( 0.0f ),
    RotationDifference( 0.0f ),
    RotationDifferenceMultiplier( 1.0f )
{
}

void UGBFCameraModifierSpringArmTargetOffsetFromVelocity::ModifyCamera( float delta_time, FVector view_location, FRotator view_rotation, float fov, FVector & new_view_location, FRotator & new_view_rotation, float & new_fov )
{
    auto * view_target = GetViewTarget();

    if ( view_target == nullptr )
    {
        return;
    }

    TargetOffset = view_target->GetVelocity();

    const auto direction = TargetOffset.GetSafeNormal();

    ViewTargetVelocity = TargetOffset.Size();

    if ( bUseOffsetCurve )
    {
        OffsetCurveValue = direction * OffsetCurve.GetRichCurveConst()->Eval( ViewTargetVelocity );

        TargetOffset = FGBFCameraModifierUtilsLibrary::GetAttributeOperationResult( InitialTargetOffset, OffsetCurveValue, Operation );
    }

    if ( bUseYawDifferenceCurve )
    {
        const auto actor_yaw = view_target->GetActorRotation().Yaw;
        const auto view_yaw = view_rotation.Yaw;

        RotationDifference = FMath::Abs( FGBFCameraModifierUtilsLibrary::ClampAngle( actor_yaw - view_yaw ) );
        RotationDifferenceMultiplier = YawDifferenceMultiplierCurve.GetRichCurveConst()->Eval( RotationDifference );
        TargetOffset *= RotationDifferenceMultiplier;
    }

    TargetOffset *= OffsetScale;
    InterpolationSpeed = OffsetInterpolationSpeed;

    if ( bUseInterpolationSpeedCurve )
    {
        InterpolationSpeed *= InterpolationSpeedCurve.GetRichCurveConst()->Eval( TargetOffset.Size() );
    }

    CurrentOffset = FMath::VInterpTo( CurrentOffset, TargetOffset, delta_time, InterpolationSpeed );
    SpringArmComponent->TargetOffset = CurrentOffset;
}

void UGBFCameraModifierSpringArmTargetOffsetFromVelocity::DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const
{
    auto & display_debug_manager = canvas->DisplayDebugManager;

    display_debug_manager.DrawString( FString::Printf( TEXT( "Initial Target Offset: %s" ), *InitialTargetOffset.ToCompactString() ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "Offset Scale: %s" ), *OffsetScale.ToCompactString() ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "Operation: %s" ), *UEnum::GetValueAsString( Operation ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "ViewTarget Velocity : %s" ), *FString::SanitizeFloat( ViewTargetVelocity ) ) );

    if ( bUseOffsetCurve )
    {
        display_debug_manager.DrawString( FString::Printf( TEXT( "Curve Offset Value: %s" ), *OffsetCurveValue.ToCompactString() ) );
    }

    if ( bUseInterpolationSpeedCurve )
    {
        display_debug_manager.DrawString( FString::Printf( TEXT( "Interpolation Speed: %s" ), *FString::SanitizeFloat( InterpolationSpeed ) ) );
    }

    if ( bUseYawDifferenceCurve )
    {
        display_debug_manager.DrawString( FString::Printf( TEXT( "Yaw Difference: %s" ), *FString::SanitizeFloat( RotationDifference ) ) );
        display_debug_manager.DrawString( FString::Printf( TEXT( "Yaw Difference Multiplier: %s" ), *FString::SanitizeFloat( RotationDifferenceMultiplier ) ) );
    }

    display_debug_manager.DrawString( FString::Printf( TEXT( "Target Offset: %s" ), *TargetOffset.ToCompactString() ) );
    DrawDebugSphere( CameraOwner->GetWorld(), SpringArmComponent->GetComponentLocation() + TargetOffset, 15.0f, 12, FColor::Cyan );

    display_debug_manager.DrawString( FString::Printf( TEXT( "Current Offset: %s" ), *CurrentOffset.ToCompactString() ) );
    DrawDebugSphere( CameraOwner->GetWorld(), SpringArmComponent->GetComponentLocation() + CurrentOffset, 15.0f, 12, FColor::Blue );
}

void UGBFCameraModifierSpringArmTargetOffsetFromVelocity::OnSpringArmComponentSet( USpringArmComponent * spring_arm_component )
{
    InitialTargetOffset = spring_arm_component->TargetOffset;
}