#include "Camera/Modifiers/GBFCameraModifierSlopeWalkingPitch.h"

#include "Camera/Modifiers/GBFCameraModifierUtils.h"

#include <Engine/Canvas.h>
#include <GameFramework/Character.h>
#include <GameFramework/CharacterMovementComponent.h>

UGBFCameraModifierSlopeWalkingPitch::UGBFCameraModifierSlopeWalkingPitch() :
    SlopeDetectionStartLocationOffset( 0.0f, 0.0f, 50.0f ),
    SlopeDetectionLineTraceLength( 1000.0f ),
    SlopeDetectionCollisionChannel( ECC_WorldStatic ),
    MinSlopeZ( 0.0f ),
    bIsOnASlope( false ),
    bIsDescendingSlope( false )
{
    SetMinSlopeAngle( 30.0f );
}

bool UGBFCameraModifierSlopeWalkingPitch::ProcessViewRotation( AActor * view_target, float delta_time, FRotator & view_rotation, FRotator & delta_rotation )
{
    auto * character = Cast< ACharacter >( GetViewTarget() );
    const auto * cmc = character->GetCharacterMovement();

    if ( character == nullptr )
    {
        return false;
    }

    const auto floor_impact_normal = cmc->CurrentFloor.HitResult.ImpactNormal;

    // Not walking on a slope steep enough. Reset camera pitch
    if ( floor_impact_normal.Z >= MinSlopeZ )
    {
        bIsOnASlope = false;
        return false;
    }

    FHitResult hit_result;
    const auto character_forward_vector = character->GetActorForwardVector();
    const auto line_trace_start = character->GetActorLocation() + SlopeDetectionStartLocationOffset;
    const auto line_trace_end = line_trace_start + character_forward_vector * SlopeDetectionLineTraceLength;

    bool is_climbing;

    if ( !FGBFCameraModifierUtilsLibrary::LineTraceSingleByChannel( character, hit_result, line_trace_start, line_trace_end, SlopeDetectionCollisionChannel, bDebug ) )
    {
        const auto reverse_line_trace_end = line_trace_start + character_forward_vector * -SlopeDetectionLineTraceLength;
        FGBFCameraModifierUtilsLibrary::LineTraceSingleByChannel( character, hit_result, line_trace_start, reverse_line_trace_end, SlopeDetectionCollisionChannel, bDebug );

        if ( !hit_result.bBlockingHit )
        {
            // We are on a slope, but none of the two traces hit something. Chances are we starting to descend the hill
            const auto is_player_descending_slope = FVector::DotProduct( character_forward_vector, floor_impact_normal ) > 0.0f;

            if ( !is_player_descending_slope )
            {
                // Climbing a slope, but traces did not hit. We are near the top of the hill
                return false;
            }
        }

        is_climbing = false;
    }
    else
    {
        bIsOnASlope = true;
        is_climbing = true;
    }

    const auto is_camera_facing_slope = FVector::DotProduct( out_view_rotation.Vector(), floor_impact_normal ) < 0.0f;

    const auto target_pitch = is_climbing
                                  ? ( is_camera_facing_slope ? ClimbSlopeCameraBehindPitch : ClimbSlopeCameraInFrontPitch )
                                  : ( is_camera_facing_slope ? DescendSlopeCameraInFrontPitch : DescendSlopeCameraBehindPitch );

    return false;
}

#if WITH_EDITOR
void UGBFCameraModifierSlopeWalkingPitch::PostEditChangeProperty( FPropertyChangedEvent & property_changed_event )
{
    Super::PostEditChangeProperty( property_changed_event );

    const auto * property_that_changed = property_changed_event.MemberProperty;

    if ( property_that_changed != nullptr &&
         property_that_changed->GetFName() == GET_MEMBER_NAME_CHECKED( UGBFCameraModifierSlopeWalkingPitch, MinSlopeAngle ) )
    {
        SetMinSlopeAngle( MinSlopeAngle );
    }
}
#endif

void UGBFCameraModifierSlopeWalkingPitch::DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const
{
    auto & display_debug_manager = canvas->DisplayDebugManager;

    display_debug_manager.DrawString( FString::Printf( TEXT( "MinSlopeAngle: %s" ), *FString::SanitizeFloat( MinSlopeAngle ) ) );
}

void UGBFCameraModifierSlopeWalkingPitch::SetMinSlopeAngle( const float min_floor_angle )
{
    MinSlopeAngle = FMath::Clamp( min_floor_angle, 0.f, 90.0f );
    MinSlopeZ = FMath::Cos( FMath::DegreesToRadians( MinSlopeAngle ) );
}