#pragma once

#include "Camera/Modifiers/GBFCameraModifier.h"

#include <CoreMinimal.h>
#include <Curves/CurveFloat.h>

#include "GBFCameraModifierFallingPitch.generated.h"

UCLASS( DisplayName = "Update Pitch When Falling" )
class GAMEBASEFRAMEWORK_API UGBFCameraModifierFallingPitch final : public UGBFCameraModifier
{
    GENERATED_BODY()

public:
    UGBFCameraModifierFallingPitch();

    bool ProcessViewRotation( AActor * view_target, float delta_time, FRotator & view_rotation, FRotator & delta_rotation ) override;

protected:
    void DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const override;

private:
    enum class EState : uint8
    {
        WaitingFall,
        Falling,
        Landing
    };

    // Curve to get the pitch to give to the camera based on the vertical velocity of the view target
    UPROPERTY( EditAnywhere )
    FRuntimeFloatCurve VelocityZToPitchCurve;

    // The minimal value the velocity must have before sampling the curve
    UPROPERTY( EditAnywhere )
    float VelocityZThreshold;

    // Curve to define the interpolation speed to change the pitch, based on the vertical velocity
    UPROPERTY( EditAnywhere )
    FRuntimeFloatCurve VelocityZToInterpolationSpeedCurve;

    UPROPERTY( EditAnywhere, meta = ( InlineEditConditionToggle ) )
    uint8 bRestorePitchWhenLanded : 1;

    // The amount of time to transition the camera to its regular pitch after landing
    UPROPERTY( EditAnywhere, meta = ( EditCondition = "bRestorePitchWhenLanded" ) )
    float LandingTransitionTime;

    // Interpolation speed to use to set the pitch back to its previous value
    UPROPERTY( EditAnywhere, meta = ( EditCondition = "bRestorePitchWhenLanded" ) )
    float LandingInterpolationSpeed;

    EState State;
    float CurrentVelocityZ;
    float CurrentInterpolationSpeed;
    float CurrentPitch;
    float TargetPitch;
    float FallingInitialPitch;
    float LandingTransitionRemainingTime;
};
