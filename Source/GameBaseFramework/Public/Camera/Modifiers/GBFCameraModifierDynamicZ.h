#pragma once

#include "Camera/Modifiers/GBFCameraModifier.h"

#include <CoreMinimal.h>
#include <Curves/CurveFloat.h>

#include "GBFCameraModifierDynamicZ.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFCameraModifierDynamicZ final : public UGBFCameraModifier
{
    GENERATED_BODY()

public:
    UGBFCameraModifierDynamicZ();

    bool IsDisabled() const override;

protected:
    void ModifyCamera( float delta_time, FVector view_location, FRotator view_rotation, float fov, FVector & new_view_location, FRotator & new_view_rotation, float & new_fov ) override;

public:
    bool ProcessViewRotation( AActor * ViewTarget, float DeltaTime, FRotator & OutViewRotation, FRotator & OutDeltaRot ) override;

protected:
    void DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const override;

private:
    UPROPERTY( EditAnywhere )
    FRuntimeFloatCurve DistanceFromLastGroundedLocationInterpolationSpeedCurve;

    UPROPERTY( EditAnywhere, meta = ( InlineEditConditionToggle ) )
    uint8 bCorrectPitchWithVelocityZ : 1;

    UPROPERTY( EditAnywhere, meta = ( EditCondition = "bCorrectPitchWithVelocityZ" ) )
    FRuntimeFloatCurve VelocityZToPitchCurve;

    float LastGroundedPositionZ;
    float CurrentPositionZ;
    float CurrentViewLocationZ;
    EMovementMode CurrentMovementMode;
    float CurrentInterpolationSpeed;
    float DeltaLastGroundedPositionZ;
    float VelocityZ;
    float PitchFromVelocityZ;
    float PitchWhenGrounded;
    bool bPitchWasManuallyChanged;
    float CurrentPitch;
    float TimeSinceLanded;
    bool bJustLanded;
};
