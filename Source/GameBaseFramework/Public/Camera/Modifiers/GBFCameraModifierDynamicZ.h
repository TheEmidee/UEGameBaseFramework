#pragma once

#include "GBFCameraModifier.h"

#include <CoreMinimal.h>
#include <Curves/CurveFloat.h>

#include "GBFCameraModifierDynamicZ.generated.h"

UCLASS( DisplayName = "Dynamic Z" )
class GAMEBASEFRAMEWORK_API UGBFCameraModifierDynamicZ final : public UGBFCameraModifier
{
    GENERATED_BODY()

public:
    UGBFCameraModifierDynamicZ();

    bool IsDisabled() const override;

protected:
    void ModifyCamera( float delta_time, FVector view_location, FRotator view_rotation, float fov, FVector & new_view_location, FRotator & new_view_rotation, float & new_fov ) override;
    void DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const override;

private:
    UPROPERTY( EditAnywhere, meta = ( InlineEditConditionToggle ) )
    uint8 bUpdateFromDistanceFromLastGroundedLocation : 1;

    UPROPERTY( EditAnywhere, meta = ( EditCondition = "bUpdateFromDistanceFromLastGroundedLocation" ) )
    FRuntimeFloatCurve DistanceFromLastGroundedLocationInterpolationSpeedCurve;

    UPROPERTY( EditAnywhere, meta = ( InlineEditConditionToggle ) )
    uint8 bUpdateCameraZOnLanding : 1;

    UPROPERTY( EditAnywhere, meta = ( EditCondition = "bUpdateCameraZOnLanding" ) )
    float LandingInterpolationTime;

    float LastGroundedCharacterPositionZ;
    float LastGroundedCameraPositionZ;
    float CurrentViewLocationZ;
    float TargetViewLocationZ;
    float CurrentInterpolationSpeed;
    float DeltaLastGroundedPositionZ;
    float VelocityZ;
    EMovementMode PreviousMovementMode;
    bool bInterpolateLanding;
    float LandingInterpolationRemainingTime;
};
