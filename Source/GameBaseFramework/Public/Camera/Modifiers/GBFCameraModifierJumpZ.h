#pragma once

#include "GBFCameraModifierSpringArmBased.h"
#include "Camera/Modifiers/GBFCameraModifier.h"

#include <CoreMinimal.h>

#include "GBFCameraModifierJumpZ.generated.h"

/**
 * This camera modifier will update the Z location of the camera dynamically when the player jumps
 */
UCLASS( DisplayName = "Jump Z" )
class GAMEBASEFRAMEWORK_API UGBFCameraModifierJumpZ final : public UGBFCameraModifierSpringArmBased
{
    GENERATED_BODY()

public:
    UGBFCameraModifierJumpZ();

    bool IsDisabled() const override;

protected:
    void ModifyCamera( float delta_time, FVector view_location, FRotator view_rotation, float fov, FVector & new_view_location, FRotator & new_view_rotation, float & new_fov ) override;
    void DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const override;

private:
    enum class EState : uint8
    {
        WaitingForJump,
        Jumping,
        Landing
    };

    UPROPERTY( EditAnywhere )
    float LandingTransitionTime;

    // Tolerance when comparing the character location on Z when he lands versus when he jumped
    UPROPERTY( EditAnywhere )
    float LandOnSameHeightCheckTolerance;

    // When falling after jumping, when the character goes below his last grounded position minus this property, we stop the modifier
    UPROPERTY( EditAnywhere )
    float DistanceFromLastGroundedPositionToResetModifier;

    EState CurrentState;
    float LastGroundedCameraZPosition;
    float LastGroundedCharacterZPosition;
    float CurrentCharacterZPosition;
    float CurrentCameraZPosition;
    float LerpStartCameraZPosition;
    float LerpEndCameraZPosition;
    float LandingTransitionRemainingTime;
};
