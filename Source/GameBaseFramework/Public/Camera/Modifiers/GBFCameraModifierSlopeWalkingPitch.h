#pragma once

#include "Camera/Modifiers/GBFCameraModifier.h"

#include <CoreMinimal.h>

#include "GBFCameraModifierSlopeWalkingPitch.generated.h"

UCLASS( DisplayName = "Update Pitch When Walking On Slopes" )
class GAMEBASEFRAMEWORK_API UGBFCameraModifierSlopeWalkingPitch final : public UGBFCameraModifier
{
    GENERATED_BODY()

public:
    UGBFCameraModifierSlopeWalkingPitch();

    bool ProcessViewRotation( AActor * view_target, float delta_time, FRotator & view_rotation, FRotator & delta_rotation ) override;

protected:
    void DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const override;

private:
    enum class EModifierState : uint8
    {
        WaitingForSlope,
        OnASlope,
        LeavingSlope
    };

    enum class EPlayerToSlopeState : uint8
    {
        Ascending,
        Descending,
        Traversing
    };

    enum class ECameraToSlopeState : uint8
    {
        Facing,
        Opposing,
        Traversing
    };

    struct FOnASlopeParameters
    {
        FOnASlopeParameters( float delta_time, const FVector& floor_impact_normal, const TWeakObjectPtr<AActor>& view_target ) :
            DeltaTime( delta_time ),
            FloorImpactNormal( floor_impact_normal ),
            ViewTarget( view_target )
        {
        }

        float DeltaTime;
        FVector FloorImpactNormal;
        TWeakObjectPtr< AActor > ViewTarget;
    };

    bool IsOnSlopeSteepEnough() const;
    void HandleStateWaitingForSlope(float view_rotation_pitch);
    void HandleStateOnASlope( FRotator & view_rotation, const FOnASlopeParameters & parameters );
    void HandleStateLeavingSlope(FRotator& view_rotation, float delta_time);

    /** Minimum angle of the slope the character is standing on to activate this modifier */
    UPROPERTY( EditAnywhere )
    float MinSlopeAngle;

    /** Offset from the center of the character capsule to start the raycast from */
    UPROPERTY( EditAnywhere )
    FVector SlopeDetectionStartLocationOffset;

    UPROPERTY( EditAnywhere )
    float SlopeDetectionLineTraceLength;

    UPROPERTY( EditAnywhere )
    TEnumAsByte< ECollisionChannel > SlopeDetectionCollisionChannel;

    // The speed at which we interpolate the pitch when the view target is on a slope
    UPROPERTY( EditAnywhere )
    float InterpolationSpeed;

    // The speed at which we interpolate the pitch when the view target leaves a slope
    UPROPERTY( EditAnywhere )
    float LeavingSlopeInterpolationSpeed;

    UPROPERTY( EditAnywhere, meta = ( InlineEditConditionToggle ) )
    uint8 bUseManualRotationCooldown : 1;

    /* The amount of time to skip correcting the camera pitch after the player manually moved the camera BEFORE being on a slope */
    UPROPERTY( EditAnywhere, meta = ( EditCondition = "bUseManualRotationCooldown" ) )
    float ManualRotationCooldownBeforeSlope;

    /* The amount of time to skip correcting the camera pitch after the player manually moved the camera WHILE being on a slope */
    UPROPERTY( EditAnywhere, meta = ( EditCondition = "bUseManualRotationCooldown" ) )
    float ManualRotationCooldownWhileOnSlope;

    EModifierState State;
    EPlayerToSlopeState PlayerToSlopeState;
    float CurrentSlopeAngle;
    ECameraToSlopeState CameraToSlopeState;
    float TargetPitch;
    float CurrentPitch;
    float PitchBeforeAdjustment;
    float ManualRotationCooldownRemainingTime;
};
