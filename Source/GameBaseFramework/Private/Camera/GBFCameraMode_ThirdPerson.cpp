#include "Camera/GBFCameraMode_ThirdPerson.h"

#include "DrawDebugHelpers.h"

#include <Components/PrimitiveComponent.h>
#include <Curves/CurveVector.h>
#include <Engine/Canvas.h>
#include <Engine/World.h>
#include <GameFramework/CameraBlockingVolume.h>
#include <GameFramework/Character.h>

namespace Private
{
    static const FName NAME_IgnoreCameraCollision = TEXT( "IgnoreCameraCollision" );
}

UGBFCameraMode_ThirdPerson::UGBFCameraMode_ThirdPerson() :
    bUseRuntimeFloatCurves( false ),
    AimLineToDesiredPosBlockedPct( 0 )
{
    TargetOffsetCurve = nullptr;

    PenetrationAvoidanceFeelers.Add( FGBFCameraPenetrationAvoidanceFeeler( FRotator( +00.0f, +00.0f, 0.0f ), 1.00f, 1.00f, 14.f, 0 ) );
    PenetrationAvoidanceFeelers.Add( FGBFCameraPenetrationAvoidanceFeeler( FRotator( +00.0f, +16.0f, 0.0f ), 0.75f, 0.75f, 00.f, 3 ) );
    PenetrationAvoidanceFeelers.Add( FGBFCameraPenetrationAvoidanceFeeler( FRotator( +00.0f, -16.0f, 0.0f ), 0.75f, 0.75f, 00.f, 3 ) );
    PenetrationAvoidanceFeelers.Add( FGBFCameraPenetrationAvoidanceFeeler( FRotator( +00.0f, +32.0f, 0.0f ), 0.50f, 0.50f, 00.f, 5 ) );
    PenetrationAvoidanceFeelers.Add( FGBFCameraPenetrationAvoidanceFeeler( FRotator( +00.0f, -32.0f, 0.0f ), 0.50f, 0.50f, 00.f, 5 ) );
    PenetrationAvoidanceFeelers.Add( FGBFCameraPenetrationAvoidanceFeeler( FRotator( +20.0f, +00.0f, 0.0f ), 1.00f, 1.00f, 00.f, 4 ) );
    PenetrationAvoidanceFeelers.Add( FGBFCameraPenetrationAvoidanceFeeler( FRotator( -20.0f, +00.0f, 0.0f ), 0.50f, 0.50f, 00.f, 4 ) );
}

void UGBFCameraMode_ThirdPerson::UpdateView( const float delta_time )
{
    UpdateForTarget( delta_time );
    UpdateCrouchOffset( delta_time );

    const auto pivot_location = GetPivotLocation() + CurrentCrouchOffset;
    auto pivot_rotation = GetPivotRotation();

    pivot_rotation.Pitch = FMath::ClampAngle( pivot_rotation.Pitch, ViewPitchMin, ViewPitchMax );

    View.Location = pivot_location;
    View.Rotation = pivot_rotation;
    View.ControlRotation = View.Rotation;
    View.FieldOfView = FieldOfView;

    // Apply third person offset using pitch.
    if ( !bUseRuntimeFloatCurves )
    {
        if ( TargetOffsetCurve )
        {
            const auto target_offset = TargetOffsetCurve->GetVectorValue( pivot_rotation.Pitch );
            View.Location = pivot_location + pivot_rotation.RotateVector( target_offset );
        }
    }
    else
    {
        FVector target_offset( 0.0f );

        target_offset.X = TargetOffsetX.GetRichCurveConst()->Eval( pivot_rotation.Pitch );
        target_offset.Y = TargetOffsetY.GetRichCurveConst()->Eval( pivot_rotation.Pitch );
        target_offset.Z = TargetOffsetZ.GetRichCurveConst()->Eval( pivot_rotation.Pitch );

        View.Location = pivot_location + pivot_rotation.RotateVector( target_offset );
    }

    // Adjust final desired camera location to prevent any penetration
    UpdatePreventPenetration( delta_time );
}

void UGBFCameraMode_ThirdPerson::UpdateForTarget( float /*delta_time*/ )
{
    if ( const auto * target_character = Cast< ACharacter >( GetTargetActor() ) )
    {
        if ( target_character->bIsCrouched )
        {
            const auto * target_character_cdo = target_character->GetClass()->GetDefaultObject< ACharacter >();
            const auto crouched_height_adjustment = target_character_cdo->CrouchedEyeHeight - target_character_cdo->BaseEyeHeight;

            SetTargetCrouchOffset( FVector( 0.f, 0.f, crouched_height_adjustment ) );

            return;
        }
    }

    SetTargetCrouchOffset( FVector::ZeroVector );
}

void UGBFCameraMode_ThirdPerson::UpdatePreventPenetration( const float delta_time )
{
    if ( !bPreventPenetration )
    {
        return;
    }

    const auto * target_actor = GetTargetActor();

    if ( const auto * actor_root_component = Cast< UPrimitiveComponent >( target_actor->GetRootComponent() ) )
    {
        // Attempt at picking SafeLocation automatically, so we reduce camera translation when aiming.
        // Our camera is our reticle, so we want to preserve our aim and keep that as steady and smooth as possible.
        // Pick closest point on capsule to our aim line.
        FVector closest_point_on_line_to_capsule_center;
        auto safe_location = target_actor->GetActorLocation();
        FMath::PointDistToLine( safe_location, View.Rotation.Vector(), View.Location, closest_point_on_line_to_capsule_center );

        // Adjust Safe distance height to be same as aim line, but within capsule.
        const auto push_in_distance = PenetrationAvoidanceFeelers[ 0 ].Extent + CollisionPushOutDistance;
        const auto max_half_height = target_actor->GetSimpleCollisionHalfHeight() - push_in_distance;
        safe_location.Z = FMath::Clamp( closest_point_on_line_to_capsule_center.Z, safe_location.Z - max_half_height, safe_location.Z + max_half_height );

        float distance_sqr;
        actor_root_component->GetSquaredDistanceToCollision( closest_point_on_line_to_capsule_center, distance_sqr, safe_location );

        // Push back inside capsule to avoid initial penetration when doing line checks.
        if ( PenetrationAvoidanceFeelers.Num() > 0 )
        {
            safe_location += ( safe_location - closest_point_on_line_to_capsule_center ).GetSafeNormal() * push_in_distance;
        }

        // Then aim line to desired camera position
        const auto single_ray_penetration_check = !bDoPredictiveAvoidance;
        PreventCameraPenetration( View.Location, AimLineToDesiredPosBlockedPct, *target_actor, safe_location, delta_time, single_ray_penetration_check );
    }
}

void UGBFCameraMode_ThirdPerson::PreventCameraPenetration( FVector & camera_location, float & distance_blocked_pct, AActor const & view_target, FVector const & safe_location, float const & delta_time, bool single_ray_only )
{
#if ENABLE_DRAW_DEBUG
    DebugActorsHitDuringCameraPenetration.Reset();
#endif

    auto hard_blocked_pct = distance_blocked_pct;
    auto soft_blocked_pct = distance_blocked_pct;

    auto base_ray = camera_location - safe_location;
    FRotationMatrix base_ray_matrix( base_ray.Rotation() );
    FVector base_ray_local_up, base_ray_local_fwd, base_ray_local_right;

    base_ray_matrix.GetScaledAxes( base_ray_local_fwd, base_ray_local_right, base_ray_local_up );

    auto dist_blocked_pct_this_frame = 1.f;

    const auto num_rays_to_shoot = single_ray_only ? FMath::Min( 1, PenetrationAvoidanceFeelers.Num() ) : PenetrationAvoidanceFeelers.Num();
    FCollisionQueryParams sphere_params( SCENE_QUERY_STAT( CameraPen ), false, nullptr /*PlayerCamera*/ );

    sphere_params.AddIgnoredActor( &view_target );

    auto sphere_shape = FCollisionShape::MakeSphere( 0.f );
    const auto * world = GetWorld();

    for ( auto ray_idx = 0; ray_idx < num_rays_to_shoot; ++ray_idx )
    {
        if ( auto & feeler = PenetrationAvoidanceFeelers[ ray_idx ];
             feeler.FramesUntilNextTrace <= 0 )
        {
            // calc ray target
            FVector ray_target;
            {
                auto rotated_ray = base_ray.RotateAngleAxis( feeler.AdjustmentRot.Yaw, base_ray_local_up );
                rotated_ray = rotated_ray.RotateAngleAxis( feeler.AdjustmentRot.Pitch, base_ray_local_right );
                ray_target = safe_location + rotated_ray;
            }

            // cast for world and pawn hits separately.  this is so we can safely ignore the
            // camera's target pawn
            sphere_shape.Sphere.Radius = feeler.Extent;
            auto trace_channel = ECC_Camera; //(Feeler.PawnWeight > 0.f) ? ECC_Pawn : ECC_Camera;

            // do multi-line check to make sure the hits we throw out aren't
            // masking real hits behind (these are important rays).

            // MT-> passing camera as actor so that camerablockingvolumes know when it's the camera doing traces
            FHitResult hit_result;
            const bool has_hit = world->SweepSingleByChannel( hit_result, safe_location, ray_target, FQuat::Identity, trace_channel, sphere_shape, sphere_params );

#if ENABLE_DRAW_DEBUG
            if ( world->TimeSince( LastDrawDebugTime ) < 1.f )
            {
                DrawDebugSphere( world, safe_location, sphere_shape.Sphere.Radius, 8, FColor::Red );
                DrawDebugSphere( world, has_hit ? hit_result.Location : ray_target, sphere_shape.Sphere.Radius, 8, FColor::Red );
                DrawDebugLine( world, safe_location, has_hit ? hit_result.Location : ray_target, FColor::Red );
            }
#endif
            feeler.FramesUntilNextTrace = feeler.TraceInterval;

            if ( const auto * hit_actor = hit_result.GetActor();
                 has_hit && hit_actor != nullptr )
            {
                auto ignore_hit = false;

                if ( hit_actor->ActorHasTag( Private::NAME_IgnoreCameraCollision ) )
                {
                    ignore_hit = true;
                    sphere_params.AddIgnoredActor( hit_actor );
                }

                // Ignore CameraBlockingVolume hits that occur in front of the ViewTarget.
                if ( !ignore_hit && hit_actor->IsA< ACameraBlockingVolume >() )
                {
                    const auto view_target_forward_xy = view_target.GetActorForwardVector().GetSafeNormal2D();
                    const auto view_target_location = view_target.GetActorLocation();
                    const auto hit_offset = hit_result.Location - view_target_location;
                    const auto hit_direction_xy = hit_offset.GetSafeNormal2D();

                    if ( const auto dot_hit_direction = FVector::DotProduct( view_target_forward_xy, hit_direction_xy );
                         dot_hit_direction > 0.0f )
                    {
                        ignore_hit = true;
                        // Ignore this CameraBlockingVolume on the remaining sweeps.
                        sphere_params.AddIgnoredActor( hit_actor );
                    }
                    else
                    {
#if ENABLE_DRAW_DEBUG
                        DebugActorsHitDuringCameraPenetration.AddUnique( TObjectPtr< const AActor >( hit_actor ) );
#endif
                    }
                }

                if ( !ignore_hit )
                {
                    const auto weight = Cast< APawn >( hit_result.GetActor() ) ? feeler.PawnWeight : feeler.WorldWeight;
                    auto new_block_pct = hit_result.Time;
                    new_block_pct += ( 1.f - new_block_pct ) * ( 1.f - weight );

                    // Recompute blocked pct taking into account pushout distance.
                    new_block_pct = ( ( hit_result.Location - safe_location ).Size() - CollisionPushOutDistance ) / ( ray_target - safe_location ).Size();
                    dist_blocked_pct_this_frame = FMath::Min( new_block_pct, dist_blocked_pct_this_frame );

                    // This feeler got a hit, so do another trace next frame
                    feeler.FramesUntilNextTrace = 0;

#if ENABLE_DRAW_DEBUG
                    DebugActorsHitDuringCameraPenetration.AddUnique( TObjectPtr< const AActor >( hit_actor ) );
#endif
                }
            }

            if ( ray_idx == 0 )
            {
                // don't interpolate toward this one, snap to it
                // assumes ray 0 is the center/main ray
                hard_blocked_pct = dist_blocked_pct_this_frame;
            }
            else
            {
                soft_blocked_pct = dist_blocked_pct_this_frame;
            }
        }
        else
        {
            --feeler.FramesUntilNextTrace;
        }
    }

    if ( bResetInterpolation )
    {
        distance_blocked_pct = dist_blocked_pct_this_frame;
    }
    else if ( distance_blocked_pct < dist_blocked_pct_this_frame )
    {
        // interpolate smoothly out
        if ( PenetrationBlendOutTime > delta_time )
        {
            distance_blocked_pct = distance_blocked_pct + delta_time / PenetrationBlendOutTime * ( dist_blocked_pct_this_frame - distance_blocked_pct );
        }
        else
        {
            distance_blocked_pct = dist_blocked_pct_this_frame;
        }
    }
    else
    {
        if ( distance_blocked_pct > hard_blocked_pct )
        {
            distance_blocked_pct = hard_blocked_pct;
        }
        else if ( distance_blocked_pct > soft_blocked_pct )
        {
            // interpolate smoothly in
            if ( PenetrationBlendInTime > delta_time )
            {
                distance_blocked_pct = distance_blocked_pct - delta_time / PenetrationBlendInTime * ( distance_blocked_pct - soft_blocked_pct );
            }
            else
            {
                distance_blocked_pct = soft_blocked_pct;
            }
        }
    }

    distance_blocked_pct = FMath::Clamp< float >( distance_blocked_pct, 0.f, 1.f );
    if ( distance_blocked_pct < ( 1.f - ZERO_ANIMWEIGHT_THRESH ) )
    {
        camera_location = safe_location + ( camera_location - safe_location ) * distance_blocked_pct;
    }
}

void UGBFCameraMode_ThirdPerson::DrawDebug( UCanvas * canvas ) const
{
    Super::DrawDebug( canvas );

#if ENABLE_DRAW_DEBUG
    auto & display_debug_manager = canvas->DisplayDebugManager;
    for ( int index = 0; index < DebugActorsHitDuringCameraPenetration.Num(); index++ )
    {
        display_debug_manager.DrawString( FString::Printf( TEXT( "HitActorDuringPenetration[%d]: %s" ), index, *DebugActorsHitDuringCameraPenetration[ index ]->GetName() ) );
    }

    LastDrawDebugTime = GetWorld()->GetTimeSeconds();
#endif
}

void UGBFCameraMode_ThirdPerson::SetTargetCrouchOffset( const FVector & new_target_offset )
{
    CrouchOffsetBlendPct = 0.0f;
    InitialCrouchOffset = CurrentCrouchOffset;
    TargetCrouchOffset = new_target_offset;
}

void UGBFCameraMode_ThirdPerson::UpdateCrouchOffset( const float delta_time )
{
    if ( CrouchOffsetBlendPct < 1.0f )
    {
        CrouchOffsetBlendPct = FMath::Min( CrouchOffsetBlendPct + delta_time * CrouchOffsetBlendMultiplier, 1.0f );
        CurrentCrouchOffset = FMath::InterpEaseInOut( InitialCrouchOffset, TargetCrouchOffset, CrouchOffsetBlendPct, 1.0f );
    }
    else
    {
        CurrentCrouchOffset = TargetCrouchOffset;
        CrouchOffsetBlendPct = 1.0f;
    }
}
