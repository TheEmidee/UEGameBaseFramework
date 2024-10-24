#include "Camera/Modifiers/GBFCameraModifierSpringArmLengthFromPitch.h"

#include <Engine/Canvas.h>
#include <GameFramework/SpringArmComponent.h>

UGBFCameraModifierSpringArmLengthFromPitch::UGBFCameraModifierSpringArmLengthFromPitch() :
    InterpolationSpeed( 2.0f ),
    InitialSpringArmTargetLength( 0.0f ),
    LastCameraPitch( 0.0f ),
    LastCurveValue( 0.0f )
{
}

void UGBFCameraModifierSpringArmLengthFromPitch::ModifyCamera( float delta_time, FVector /* view_location */, FRotator view_rotation, float /* fov */, FVector & /* new_view_location */, FRotator & /* new_view_rotation */, float & /* new_fov */ )
{
    LastCameraPitch = view_rotation.Pitch;

    if ( SpringArmComponent != nullptr )
    {
        LastCurveValue = PitchToArmLengthCurve.GetRichCurve()->Eval( LastCameraPitch );
        SpringArmComponent->TargetArmLength = FMath::FInterpTo( SpringArmComponent->TargetArmLength, LastCurveValue, delta_time, InterpolationSpeed );
    }
}

void UGBFCameraModifierSpringArmLengthFromPitch::OnSpringArmComponentSet( USpringArmComponent * spring_arm_component )
{
    InitialSpringArmTargetLength = SpringArmComponent->TargetArmLength;
}

void UGBFCameraModifierSpringArmLengthFromPitch::DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const
{
    auto & display_debug_manager = canvas->DisplayDebugManager;

    display_debug_manager.DrawString( FString::Printf( TEXT( "Arm initial length: %s" ), *FString::SanitizeFloat( InitialSpringArmTargetLength ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "Camera Pitch: %s" ), *FString::SanitizeFloat( LastCameraPitch ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "Curve Value: %s" ), *FString::SanitizeFloat( LastCurveValue ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "Arm final length: %s" ), *FString::SanitizeFloat( SpringArmComponent->TargetArmLength ) ) );
}