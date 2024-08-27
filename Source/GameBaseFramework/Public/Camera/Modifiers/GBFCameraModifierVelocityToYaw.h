#pragma once

#include "Camera/Modifiers/GBFCameraModifier.h"

#include <CoreMinimal.h>
#include <Curves/CurveFloat.h>

#include "GBFCameraModifierVelocityToYaw.generated.h"

UCLASS( DisplayName = "Update Yaw From Velocity" )
class GAMEBASEFRAMEWORK_API UGBFCameraModifierVelocityToYaw final : public UGBFCameraModifier
{
    GENERATED_BODY()

public:
    UGBFCameraModifierVelocityToYaw();

    bool ProcessViewRotation( AActor * view_target, float delta_time, FRotator & view_rotation, FRotator & delta_rotation ) override;

protected:
    void DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const override;

private:
    UPROPERTY( EditAnywhere )
    float InterpolationSpeed;

    UPROPERTY( EditAnywhere, meta = ( InlineEditConditionToggle ) )
    uint8 bUseInterpolationSpeedMultiplierFromTime : 1;

    /* Curve to multiply the interpolation speed, based on the time the modifier has been running
     Y should be between 0 and 1
     */
    UPROPERTY( EditAnywhere, meta = ( EditCondition = "bUseInterpolationSpeedMultiplierFromTime" ) )
    FRuntimeFloatCurve InterpolationSpeedMultiplierFromTimeCurve;

    /*UPROPERTY( EditAnywhere, meta = ( InlineEditConditionToggle ) )
    uint8 bUseInterpolationSpeedMultiplierFromDistance : 1;

    UPROPERTY( EditAnywhere, meta = ( EditCondition = "bUseInterpolationSpeedMultiplierFromDistance" ) )
    FRuntimeFloatCurve InterpolationSpeedMultiplierFromDistanceCurve;*/

    UPROPERTY( EditAnywhere, meta = ( InlineEditConditionToggle ) )
    uint8 bUseManualRotationCooldown : 1;

    /* The amount of time to skip correcting the camera Yaw after the player manually moved the camera */
    UPROPERTY( EditAnywhere, meta = ( EditCondition = "bUseManualRotationCooldown" ) )
    float ManualRotationCooldown;

    /* Allows to filter out some axis of the velocity to compute the correction. Typically, Z should be 0 */
    UPROPERTY( EditAnywhere )
    FVector VelocityScale;

    UPROPERTY( EditAnywhere, meta = ( InlineEditConditionToggle ) )
    uint8 bUseYawCorrectionMultiplierFromVelocity : 1;

    /* Curve to multiply the correction, based on distance.
     This allows to reduce the correction at low speed, and increase it at high speed
     Y should be between 0 and 1 */
    UPROPERTY( EditAnywhere, meta = ( EditCondition = "bUseYawCorrectionMultiplierFromVelocity" ) )
    FRuntimeFloatCurve YawCorrectionMultiplierFromVelocityCurve;

    UPROPERTY( EditAnywhere, meta = ( InlineEditConditionToggle ) )
    uint8 bCanRotateToFaceViewTarget : 1;

    /* When the angle difference between the view target and the camera is greater than this value then
     the modifier will move the camera to face the view target, instead of moving behind
     */
    UPROPERTY( EditAnywhere, meta = ( EditCondition = "bCanRotateToFaceViewTarget" ) )
    float FaceViewTargetAngleThreshold;

    float InterpolationSpeedMultiplierTime;
    float InterpolationSpeedMultiplierFromTime;
    float FinalInterpolationSpeed;
    float ManualRotationCooldownRemainingTime;
    float ViewTargetVelocity;
    float YawCorrectionMultiplierFromVelocity;
    float RotationDifference;
    float Dot;
    float CurrentYaw;
    float TargetYaw;
};
