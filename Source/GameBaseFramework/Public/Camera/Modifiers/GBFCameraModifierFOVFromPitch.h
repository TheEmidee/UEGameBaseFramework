#pragma once

#include "Camera/Modifiers/GBFCameraModifier.h"
#include "GBFCameraModifierTypes.h"

#include <CoreMinimal.h>
#include <Curves/CurveFloat.h>

#include "GBFCameraModifierFOVFromPitch.generated.h"

UCLASS( DisplayName = "Update FOV From Pitch" )
class GAMEBASEFRAMEWORK_API UGBFCameraModifierFOVFromPitch final : public UGBFCameraModifier
{
    GENERATED_BODY()

public:
    UGBFCameraModifierFOVFromPitch();

protected:
    void ModifyCamera( float delta_time, FVector view_location, FRotator view_rotation, float fov, FVector & new_view_location, FRotator & new_view_rotation, float & new_fov ) override;
    void DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const override;

private:
    UPROPERTY( EditAnywhere )
    FRuntimeFloatCurve PitchToFOVCurve;

    UPROPERTY( EditAnywhere )
    EGBFCameraModifierAttributeOperation Operation;

    float Pitch;
    float InitialFOV;
    float CurveFloatFOV;
    float FinalFOV;
};
