#pragma once

#include "GBFCameraModifierSpringArmBased.h"
#include "GBFCameraModifierTypes.h"

#include <CoreMinimal.h>
#include <Curves/CurveFloat.h>

#include "GBFCameraModifierSpringArmTargetOffsetFromVelocity.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFCameraModifierSpringArmTargetOffsetFromVelocity final : public UGBFCameraModifierSpringArmBased
{
    GENERATED_BODY()

public:
    UGBFCameraModifierSpringArmTargetOffsetFromVelocity();

    void ModifyCamera( float delta_time, FVector view_location, FRotator view_rotation, float fov, FVector & new_view_location, FRotator & new_view_rotation, float & new_fov ) override;

protected:
    void DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const override;
    void OnSpringArmComponentSet( USpringArmComponent * spring_arm_component ) override;

private:
    /* On which axis should we update the target offset? */
    UPROPERTY( EditAnywhere )
    FVector OffsetScale;

    UPROPERTY( EditAnywhere )
    EGBFCameraModifierAttributeOperation Operation;

    UPROPERTY( EditAnywhere, meta = ( InlineEditConditionToggle ) )
    uint8 bUseOffsetCurve : 1;

    /* The offset to apply on the target offset based on the velocity of the view target */
    UPROPERTY( EditAnywhere, meta = ( EditCondition = "bUseOffsetCurve" ) )
    FRuntimeFloatCurve OffsetCurve;

    /* The speed at which we interpolate the offset */
    UPROPERTY( EditAnywhere )
    float OffsetInterpolationSpeed;

    UPROPERTY( EditAnywhere, meta = ( InlineEditConditionToggle ) )
    uint8 bUseInterpolationSpeedCurve : 1;

    /* A multiplier applied to OffsetInterpolationSpeed, based on the distance between the target and the original location of the component.
     * Should be between 0 and 1 on the Y axis. */
    UPROPERTY( EditAnywhere, meta = ( EditCondition = "bUseInterpolationSpeedCurve" ) )
    FRuntimeFloatCurve InterpolationSpeedCurve;

    UPROPERTY( EditAnywhere, meta = ( InlineEditConditionToggle ) )
    uint8 bUseYawDifferenceCurve : 1;

    /* A multiplier applied to the target offset, based on the difference between the view target rotation and the camera rotation.
     * Useful to limit the offset when the camera is in the same orientation as the view target */
    UPROPERTY( EditAnywhere, meta = ( EditCondition = "bUseYawDifferenceCurve" ) )
    FRuntimeFloatCurve YawDifferenceMultiplierCurve;

    FVector InitialTargetOffset;
    float ViewTargetVelocity;
    FVector OffsetCurveValue;
    float InterpolationSpeed;
    FVector TargetOffset;
    FVector CurrentOffset;
    float RotationDifference;
    float RotationDifferenceMultiplier;
};
