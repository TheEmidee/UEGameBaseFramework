#include "Camera/Modifiers/GBFCameraModifierSlopeWalkingPitch.h"

#include "Camera/Modifiers/GBFCameraModifierUtils.h"

#include <Engine/Canvas.h>
#include <GameFramework/Character.h>
#include <GameFramework/CharacterMovementComponent.h>

UGBFCameraModifierSlopeWalkingPitch::UGBFCameraModifierSlopeWalkingPitch() :
    MinSlopeAngle( 20.0f ),
    SlopeDetectionStartLocationOffset( 0.0f, 0.0f, 50.0f ),
    SlopeDetectionLineTraceLength( 1000.0f ),
    SlopeDetectionCollisionChannel( ECC_WorldStatic ),
    bUseSlopeAngleToPitchCurve( false ),
    InterpolationSpeed( 2.0f ),
    LeavingSlopeInterpolationSpeed( 2.0f ),
    bUseManualRotationCooldown( true ),
    ManualRotationCooldownBeforeSlope( 2.50f ),
    ManualRotationCooldownWhileOnSlope( 10.0f ),
    State( EModifierState::WaitingForSlope ),
    PlayerToSlopeState( EPlayerToSlopeState::Traversing ),
    CurrentSlopeAngle( 0.0f ),
    CameraToSlopeState( ECameraToSlopeState::Facing ),
    TargetPitch( 0.0f ),
    CurrentPitch( 0.0f ),
    PitchBeforeAdjustment( 0.0f ),
    ManualRotationCooldownRemainingTime( 0.0f )
{
}

bool UGBFCameraModifierSlopeWalkingPitch::ProcessViewRotation( AActor * view_target, float const delta_time, FRotator & view_rotation, FRotator & delta_rotation )
{
    auto * vt = GetViewTarget();
    if ( vt == nullptr )
    {
        return false;
    }

    const auto * character = Cast< ACharacter >( vt );
    if ( character == nullptr )
    {
        return false;
    }

    const auto * cmc = character->GetCharacterMovement();

    if ( !cmc->CurrentFloor.bBlockingHit )
    {
        return false;
    }

    const auto floor_impact_normal = cmc->CurrentFloor.HitResult.ImpactNormal;

    CurrentSlopeAngle = FMath::RadiansToDegrees( FMath::Acos( floor_impact_normal.Z ) );

    if ( bUseManualRotationCooldown )
    {
        if ( !FMath::IsNearlyZero( delta_rotation.Pitch ) )
        {
            ManualRotationCooldownRemainingTime = IsOnSlopeSteepEnough() ? ManualRotationCooldownWhileOnSlope : ManualRotationCooldownBeforeSlope;
            PitchBeforeAdjustment = FGBFCameraModifierUtilsLibrary::ClampAngle( view_rotation.Pitch );
            CurrentPitch = PitchBeforeAdjustment;
            return false;
        }

        if ( ManualRotationCooldownRemainingTime >= 0.0f )
        {
            ManualRotationCooldownRemainingTime -= delta_time;
            return false;
        }
    }

    switch ( State )
    {
        case EModifierState::WaitingForSlope:
        {
            HandleStateWaitingForSlope( view_rotation.Pitch );
        }
        break;
        case EModifierState::OnASlope:
        {
            HandleStateOnASlope( view_rotation, { delta_time, floor_impact_normal, vt } );
        }
        break;
        case EModifierState::LeavingSlope:
        {
            HandleStateLeavingSlope( view_rotation, delta_time );
        }
        break;
        default:
        {
            checkNoEntry();
        }
        break;
    }

    return false;
}

void UGBFCameraModifierSlopeWalkingPitch::DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const
{
    auto & display_debug_manager = canvas->DisplayDebugManager;

    static const FString PlayerToSlopeStateString[] = {
        TEXT( "Ascending" ),
        TEXT( "Descending" ),
        TEXT( "Traversing" )
    };

    static const FString CameraToSlopeStateString[] = {
        TEXT( "Facing" ),
        TEXT( "Opposing" ),
        TEXT( "Traversing" )
    };

    display_debug_manager.DrawString( FString::Printf( TEXT( "MinSlopeAngle: %s" ), *FString::SanitizeFloat( MinSlopeAngle ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "CurrentSlopeAngle: %s" ), *FString::SanitizeFloat( CurrentSlopeAngle ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "CurrentPitch: %s" ), *FString::SanitizeFloat( CurrentPitch ) ) );

    if ( bUseManualRotationCooldown && ManualRotationCooldownRemainingTime > 0.0f )
    {
        display_debug_manager.DrawString( TEXT( "UseManualRotationCooldown" ) );
        display_debug_manager.DrawString( FString::Printf( TEXT( "ManualRotationCooldownRemainingTime: %s" ), *FString::SanitizeFloat( ManualRotationCooldownRemainingTime ) ) );
        return;
    }

    switch ( State )
    {
        case EModifierState::WaitingForSlope:
        {
            display_debug_manager.DrawString( TEXT( "State : Waiting for slope" ) );
        }
        break;
        case EModifierState::OnASlope:
        {
            display_debug_manager.DrawString( TEXT( "State : On a slope" ) );
            display_debug_manager.DrawString( FString::Printf( TEXT( "PlayerToSlopeState: %s" ), *PlayerToSlopeStateString[ static_cast< uint8 >( PlayerToSlopeState ) ] ) );
            display_debug_manager.DrawString( FString::Printf( TEXT( "CameraToSlopeState: %s" ), *CameraToSlopeStateString[ static_cast< uint8 >( CameraToSlopeState ) ] ) );
            display_debug_manager.DrawString( FString::Printf( TEXT( "TargetPitch: %s" ), *FString::SanitizeFloat( TargetPitch ) ) );
        }
        break;
        case EModifierState::LeavingSlope:
        {
            display_debug_manager.DrawString( TEXT( "State : Leaving slope" ) );
            display_debug_manager.DrawString( FString::Printf( TEXT( "TargetPitch: %s" ), *FString::SanitizeFloat( TargetPitch ) ) );
        }
        break;
        default:
        {
            checkNoEntry();
        }
        break;
    }
}

bool UGBFCameraModifierSlopeWalkingPitch::IsOnSlopeSteepEnough() const
{
    return CurrentSlopeAngle >= MinSlopeAngle;
}

void UGBFCameraModifierSlopeWalkingPitch::HandleStateWaitingForSlope( const float view_rotation_pitch )
{
    if ( !IsOnSlopeSteepEnough() )
    {
        PitchBeforeAdjustment = FGBFCameraModifierUtilsLibrary::ClampAngle( view_rotation_pitch );
        CurrentPitch = PitchBeforeAdjustment;
    }
    else
    {
        State = EModifierState::OnASlope;
    }
}

void UGBFCameraModifierSlopeWalkingPitch::HandleStateOnASlope( FRotator & view_rotation, const FOnASlopeParameters & parameters )
{
    if ( !IsOnSlopeSteepEnough() )
    {
        State = EModifierState::LeavingSlope;
        TargetPitch = PitchBeforeAdjustment;
        return;
    }

    FHitResult hit_result;
    const auto character_forward_vector = parameters.ViewTarget->GetActorForwardVector();
    const auto line_trace_start = parameters.ViewTarget->GetActorLocation() + SlopeDetectionStartLocationOffset;
    const auto line_trace_end = line_trace_start + character_forward_vector * SlopeDetectionLineTraceLength;

    if ( !FGBFCameraModifierUtilsLibrary::LineTraceSingleByChannel( parameters.ViewTarget.Get(), hit_result, line_trace_start, line_trace_end, SlopeDetectionCollisionChannel, bDebug ) )
    {
        const auto reverse_line_trace_end = line_trace_start + character_forward_vector * -SlopeDetectionLineTraceLength;
        FGBFCameraModifierUtilsLibrary::LineTraceSingleByChannel( parameters.ViewTarget.Get(), hit_result, line_trace_start, reverse_line_trace_end, SlopeDetectionCollisionChannel, bDebug );

        if ( !hit_result.bBlockingHit )
        {
            const auto dot = FVector::DotProduct( character_forward_vector, parameters.FloorImpactNormal );

            if ( FMath::Abs( dot ) < 0.1f )
            {
                PlayerToSlopeState = EPlayerToSlopeState::Traversing;
                return;
            }

            // We are on a slope, but none of the two traces hit something. Chances are we starting to descend the hill
            const auto is_player_descending_slope = dot > 0.0f;

            if ( !is_player_descending_slope )
            {
                // Ascending a slope, but traces did not hit. We are near the top of the hill
                PlayerToSlopeState = EPlayerToSlopeState::Ascending;
                return;
            }
        }

        PlayerToSlopeState = EPlayerToSlopeState::Descending;
    }
    else
    {
        PlayerToSlopeState = EPlayerToSlopeState::Ascending;
    }

    const auto camera_dot = FVector::DotProduct( view_rotation.Vector().GetSafeNormal2D(), parameters.FloorImpactNormal.GetSafeNormal2D() );

    if ( FMath::Abs( camera_dot ) < 0.1f )
    {
        CameraToSlopeState = ECameraToSlopeState::Traversing;
    }
    else if ( camera_dot > 0.0f )
    {
        CameraToSlopeState = ECameraToSlopeState::Opposing;
    }
    else
    {
        CameraToSlopeState = ECameraToSlopeState::Facing;
    }

    TargetPitch = bUseSlopeAngleToPitchCurve
                      ? SlopeAngleToPitchCurve.GetRichCurveConst()->Eval( CurrentSlopeAngle )
                      : CurrentSlopeAngle;

    switch ( CameraToSlopeState )
    {
        case ECameraToSlopeState::Facing:
        {
        }
        break;
        case ECameraToSlopeState::Opposing:
        {
            TargetPitch *= -1.0f;
        }
        break;
        case ECameraToSlopeState::Traversing:
        {
            TargetPitch = PitchBeforeAdjustment;
        }
        break;
        default:
        {
            checkNoEntry();
        }
        break;
    }

    CurrentPitch = FMath::FInterpTo( CurrentPitch, TargetPitch, parameters.DeltaTime, InterpolationSpeed );
    view_rotation.Pitch = CurrentPitch;
}

void UGBFCameraModifierSlopeWalkingPitch::HandleStateLeavingSlope( FRotator & view_rotation, const float delta_time )
{
    CurrentPitch = FMath::FInterpTo( CurrentPitch, TargetPitch, delta_time, LeavingSlopeInterpolationSpeed );
    view_rotation.Pitch = CurrentPitch;

    if ( FMath::IsNearlyEqual( CurrentPitch, TargetPitch, 1.0f ) )
    {
        State = EModifierState::WaitingForSlope;
    }

    if ( IsOnSlopeSteepEnough() )
    {
        State = EModifierState::OnASlope;
    }
}