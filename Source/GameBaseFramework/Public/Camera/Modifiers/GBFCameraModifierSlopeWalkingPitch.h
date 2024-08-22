#pragma once

#include "Camera/Modifiers/GBFCameraModifier.h"

#include <CoreMinimal.h>

#include "GBFCameraModifierSlopeWalkingPitch.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFCameraModifierSlopeWalkingPitch final : public UGBFCameraModifier
{
    GENERATED_BODY()

public:
    UGBFCameraModifierSlopeWalkingPitch();

    bool ProcessViewRotation( AActor * view_target, float delta_time, FRotator & view_rotation, FRotator & delta_rotation ) override;

#if WITH_EDITOR
    void PostEditChangeProperty( FPropertyChangedEvent & property_changed_event ) override;
#endif

protected:
    void DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const override;

private:
    void SetMinSlopeAngle( const float min_floor_angle );

    /** Minimum angle of the slope the character is standing on to activate this modifier */
    UPROPERTY( EditAnywhere )
    float MinSlopeAngle;

    /** Offset from the center of the character capsule to start the raycast from */
    UPROPERTY( EditAnywhere )
    FVector SlopeDetectionStartLocationOffset;

    UPROPERTY( EditAnywhere )
    float SlopeDetectionLineTraceLength;

    UPROPERTY( Category = Custom, EditDefaultsOnly )
    TEnumAsByte< ECollisionChannel > SlopeDetectionCollisionChannel;

    float MinSlopeZ;
    bool bIsOnASlope;
    bool bIsDescendingSlope;
};
