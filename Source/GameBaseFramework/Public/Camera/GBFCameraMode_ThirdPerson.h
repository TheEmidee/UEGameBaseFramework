#pragma once

#include "Camera/GBFCameraMode.h"

#include <CoreMinimal.h>
#include <Curves/CurveFloat.h>

#include "GBFCameraMode_ThirdPerson.generated.h"

class UCurveVector;
/**
 * Struct defining a feeler ray used for camera penetration avoidance.
 */
USTRUCT()
struct FGBFCameraPenetrationAvoidanceFeeler
{
    GENERATED_BODY()

    /** FRotator describing deviance from main ray */
    UPROPERTY( EditAnywhere, Category = PenetrationAvoidanceFeeler )
    FRotator AdjustmentRot;

    /** how much this feeler affects the final position if it hits the world */
    UPROPERTY( EditAnywhere, Category = PenetrationAvoidanceFeeler )
    float WorldWeight;

    /** how much this feeler affects the final position if it hits a APawn (setting to 0 will not attempt to collide with pawns at all) */
    UPROPERTY( EditAnywhere, Category = PenetrationAvoidanceFeeler )
    float PawnWeight;

    /** extent to use for collision when tracing this feeler */
    UPROPERTY( EditAnywhere, Category = PenetrationAvoidanceFeeler )
    float Extent;

    /** minimum frame interval between traces with this feeler if nothing was hit last frame */
    UPROPERTY( EditAnywhere, Category = PenetrationAvoidanceFeeler )
    int32 TraceInterval;

    /** number of frames since this feeler was used */
    UPROPERTY( transient )
    int32 FramesUntilNextTrace;

    FGBFCameraPenetrationAvoidanceFeeler() :
        AdjustmentRot( ForceInit ),
        WorldWeight( 0 ),
        PawnWeight( 0 ),
        Extent( 0 ),
        TraceInterval( 0 ),
        FramesUntilNextTrace( 0 )
    {
    }

    FGBFCameraPenetrationAvoidanceFeeler( const FRotator & InAdjustmentRot,
        const float & InWorldWeight,
        const float & InPawnWeight,
        const float & InExtent,
        const int32 & InTraceInterval = 0,
        const int32 & InFramesUntilNextTrace = 0 ) :
        AdjustmentRot( InAdjustmentRot ),
        WorldWeight( InWorldWeight ),
        PawnWeight( InPawnWeight ),
        Extent( InExtent ),
        TraceInterval( InTraceInterval ),
        FramesUntilNextTrace( InFramesUntilNextTrace )
    {
    }
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFCameraMode_ThirdPerson : public UGBFCameraMode
{
    GENERATED_BODY()

public:
    UGBFCameraMode_ThirdPerson();

protected:
    void UpdateView( float delta_time ) override;

    void UpdateForTarget( float delta_time );
    void UpdatePreventPenetration( float delta_time );
    void PreventCameraPenetration( FVector & camera_location, float & distance_blocked_pct, class AActor const & view_target, FVector const & safe_location, float const & delta_time, bool single_ray_only );

    void DrawDebug( UCanvas * canvas ) const override;

private:
    void SetTargetCrouchOffset( const FVector & new_target_offset );
    void UpdateCrouchOffset( float delta_time );

    // Curve that defines local-space offsets from the target using the view pitch to evaluate the curve.
    UPROPERTY( EditDefaultsOnly, Category = "Third Person", Meta = ( EditCondition = "!bUseRuntimeFloatCurves" ) )
    TObjectPtr< const UCurveVector > TargetOffsetCurve;

    // UE-103986: Live editing of RuntimeFloatCurves during PIE does not work (unlike curve assets).
    // Once that is resolved this will become the default and TargetOffsetCurve will be removed.
    UPROPERTY( EditDefaultsOnly, Category = "Third Person" )
    bool bUseRuntimeFloatCurves;

    UPROPERTY( EditDefaultsOnly, Category = "Third Person", Meta = ( EditCondition = "bUseRuntimeFloatCurves" ) )
    FRuntimeFloatCurve TargetOffsetX;

    UPROPERTY( EditDefaultsOnly, Category = "Third Person", Meta = ( EditCondition = "bUseRuntimeFloatCurves" ) )
    FRuntimeFloatCurve TargetOffsetY;

    UPROPERTY( EditDefaultsOnly, Category = "Third Person", Meta = ( EditCondition = "bUseRuntimeFloatCurves" ) )
    FRuntimeFloatCurve TargetOffsetZ;

    // Alters the speed that a crouch offset is blended in or out
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Third Person", meta = ( AllowPrivateAccess = true ) )
    float CrouchOffsetBlendMultiplier = 5.0f;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = ( AllowPrivateAccess = true ) )
    float PenetrationBlendInTime = 0.1f;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = ( AllowPrivateAccess = true ) )
    float PenetrationBlendOutTime = 0.15f;

    /** If true, does collision checks to keep the camera out of the world. */
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = ( AllowPrivateAccess = true ) )
    bool bPreventPenetration = true;

    /** If true, try to detect nearby walls and move the camera in anticipation.  Helps prevent popping. */
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = ( AllowPrivateAccess = true ) )
    bool bDoPredictiveAvoidance = true;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = ( AllowPrivateAccess = true ) )
    float CollisionPushOutDistance = 2.f;

    /** When the camera's distance is pushed into this percentage of its full distance due to penetration */
    UPROPERTY( EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = ( AllowPrivateAccess = true ) )
    float ReportPenetrationPercent = 0.f;

    /**
     * These are the feeler rays that are used to find where to place the camera.
     * Index: 0  : This is the normal feeler we use to prevent collisions.
     * Index: 1+ : These feelers are used if you bDoPredictiveAvoidance=true, to scan for potential impacts if the player
     *             were to rotate towards that direction and primitively collide the camera so that it pulls in before
     *             impacting the occluder.
     */
    UPROPERTY( EditDefaultsOnly, Category = "Collision" )
    TArray< FGBFCameraPenetrationAvoidanceFeeler > PenetrationAvoidanceFeelers;

    UPROPERTY( Transient )
    float AimLineToDesiredPosBlockedPct;

    UPROPERTY( Transient )
    TArray< TObjectPtr< const AActor > > DebugActorsHitDuringCameraPenetration;

    mutable float LastDrawDebugTime = -MAX_FLT;

    FVector InitialCrouchOffset = FVector::ZeroVector;
    FVector TargetCrouchOffset = FVector::ZeroVector;
    float CrouchOffsetBlendPct = 1.0f;
    FVector CurrentCrouchOffset = FVector::ZeroVector;
};
