#pragma once

#include "Camera/Modifiers/GBFCameraModifier.h"
#include "GBFCameraModifierTypes.h"

#include <CoreMinimal.h>
#include <Curves/CurveFloat.h>

#include "GBFCameraModifierFOVFromVelocity.generated.h"

UCLASS( DisplayName = "Update FOV from Velocity" )
class GAMEBASEFRAMEWORK_API UGBFCameraModifierFOVFromVelocity final : public UGBFCameraModifier
{
    GENERATED_BODY()

public:
    UGBFCameraModifierFOVFromVelocity();

    void AddedToCamera( APlayerCameraManager * camera ) override;
    void ModifyCamera( float delta_time, FVector view_location, FRotator view_rotation, float fov, FVector & new_view_location, FRotator & new_view_rotation, float & new_fov ) override;

protected:
    void DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const override;

private:
    /* Allows to filter out some axis of the velocity to compute the correction. Typically, Z should be 0 */
    UPROPERTY( EditAnywhere )
    FVector VelocityScale;

    // The FOV to use from the velocity
    UPROPERTY( EditAnywhere )
    FRuntimeFloatCurve VelocityToFOVCurve;

    UPROPERTY( EditAnywhere, meta = ( InlineEditConditionToggle ) )
    uint8 bUseInterpolationSpeedCurve : 1;

    UPROPERTY( EditAnywhere, meta = ( EditCondition = "bUseInterpolationSpeedCurve" ) )
    FRuntimeFloatCurve InterpolationSpeedCurve;

    UPROPERTY( EditAnywhere )
    EGBFCameraModifierAttributeOperation Operation;

    UPROPERTY( EditAnywhere )
    float FOVInterpolationSpeed;

    float ViewTargetVelocity;
    float InitialFOV;
    float CurveFloatFOV;
    float FinalFOV;
    float InterpolationSpeed;
};
