#pragma once

#include "GBFCameraModifierSpringArmBased.h"
#include "GBFCameraModifierTypes.h"

#include <CoreMinimal.h>
#include <Curves/CurveFloat.h>

#include "GBFCameraModifierSpringArmLengthFromPitch.generated.h"

UCLASS( DisplayName = "Update Spring Arm Length From Pitch" )
class GAMEBASEFRAMEWORK_API UGBFCameraModifierSpringArmLengthFromPitch final : public UGBFCameraModifierSpringArmBased
{
    GENERATED_BODY()

public:
    UGBFCameraModifierSpringArmLengthFromPitch();

    void ModifyCamera( float delta_time, FVector view_location, FRotator view_rotation, float fov, FVector & new_view_location, FRotator & new_view_rotation, float & new_fov ) override;

protected:
    void OnSpringArmComponentSet( USpringArmComponent * spring_arm_component ) override;

private:
    void DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const override;

    UPROPERTY( EditAnywhere )
    FRuntimeFloatCurve PitchToArmLengthCurve;

    UPROPERTY( EditAnywhere )
    EGBFCameraModifierAttributeOperation Operation;

    float InitialSpringArmTargetLength;
    float LastCameraPitch;
    float LastCurveValue;
};
