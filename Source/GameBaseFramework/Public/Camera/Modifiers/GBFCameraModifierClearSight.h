#pragma once

#include "Camera/Modifiers/GBFCameraModifier.h"

#include <CoreMinimal.h>

#include "GBFCameraModifierClearSight.generated.h"

UCLASS( DisplayName = "Clear Sight" )
class GAMEBASEFRAMEWORK_API UGBFCameraModifierClearSight final : public UGBFCameraModifier
{
    GENERATED_BODY()

public:
    UGBFCameraModifierClearSight();

    bool ProcessViewRotation( AActor * view_target, float delta_time, FRotator & view_rotation, FRotator & delta_rotation ) override;

protected:
    void DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const override;

private:
    struct LineOfSightCheckParameters
    {
        LineOfSightCheckParameters( AActor * view_target, const FVector & camera_location, const FRotator & camera_rotation, float trace_angle ) :
            ViewTarget( view_target ),
            CameraLocation( camera_location ),
            CameraRotation( camera_rotation ),
            TraceAngle( trace_angle )
        {
        }

        AActor * ViewTarget;
        FVector CameraLocation;
        FRotator CameraRotation;
        float TraceAngle;
    };

    bool IsLineOfSightObstructed( const LineOfSightCheckParameters & parameters ) const;

    // The radius of the sphere we cast to find obstacles
    UPROPERTY( EditAnywhere )
    float LineOfSightProbeRadius;

    // The collision channel to use for sphere casts
    UPROPERTY( EditAnywhere )
    TEnumAsByte< ECollisionChannel > LineOfSightProbeChannel;

    /** How far away from the current camera angle we should look for potential obstacles. */
    UPROPERTY( EditAnywhere )
    float MaximumAngle;

    /** How much rotation to increment between each sphere cast, up to MaximumAngle */
    UPROPERTY( EditAnywhere )
    float StepSizeAngle;

    /** The speed to rotate the camera */
    UPROPERTY( EditAnywhere )
    float InterpolationSpeed;

    float CurrentYaw;
    float TargetYaw;
    float AngleCorrection;
};
