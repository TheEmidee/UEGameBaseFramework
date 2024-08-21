#include "Camera/Modifiers/GBFCameraModifierDynamicZ.h"

#include <Engine/Canvas.h>
#include <GameFramework/Character.h>
#include <GameFramework/CharacterMovementComponent.h>

UGBFCameraModifierDynamicZ::UGBFCameraModifierDynamicZ() :
    bCorrectPitchWithVelocityZ( false ),
    LastGroundedPositionZ( 0.0f ),
    CurrentPositionZ( 0.0f ),
    CurrentViewLocationZ( 0.0f ),
    CurrentMovementMode( MOVE_None ),
    CurrentInterpolationSpeed( 0.0f ),
    DeltaLastGroundedPositionZ( 0.0f ),
    VelocityZ( 0.0f ),
    PitchFromVelocityZ( 0.0f ),
    PitchWhenGrounded( 0.0f ),
    bPitchWasManuallyChanged( false ),
    CurrentPitch( 0.0f ),
    TimeSinceLanded( 0.0f ),
    bJustLanded( false )
{
}

bool UGBFCameraModifierDynamicZ::IsDisabled() const
{
    if ( Super::IsDisabled() )
    {
        return true;
    }

    return Cast< ACharacter >( GetViewTarget() ) == nullptr;
}

void UGBFCameraModifierDynamicZ::ModifyCamera( float delta_time, FVector view_location, FRotator view_rotation, float fov, FVector & new_view_location, FRotator & new_view_rotation, float & new_fov )
{
    const auto * character = Cast< ACharacter >( GetViewTarget() );

    const auto cmc_movement_mode = character->GetCharacterMovement()->MovementMode;

    if ( CurrentMovementMode != cmc_movement_mode )
    {
        if ( cmc_movement_mode == MOVE_Walking && CurrentMovementMode == MOVE_Falling )
        {
            TimeSinceLanded = 0.0f;
            bJustLanded = true;
        }

        CurrentMovementMode = cmc_movement_mode;
    }

    const auto character_z = character->GetActorLocation().Z;
    VelocityZ = character->GetCharacterMovement()->Velocity.Z;

    if ( CurrentMovementMode == MOVE_Walking )
    {
        LastGroundedPositionZ = character_z;
        CurrentViewLocationZ = view_location.Z;
        PitchWhenGrounded = view_rotation.Pitch;
    }
    else
    {
        DeltaLastGroundedPositionZ = FMath::Abs( character_z - LastGroundedPositionZ );
        CurrentInterpolationSpeed = FMath::Abs( DistanceFromLastGroundedLocationInterpolationSpeedCurve.GetRichCurveConst()->Eval( VelocityZ ) );
    }

    const auto delta_current_view_z = view_location.Z - CurrentViewLocationZ;
    const auto delta_sign = FMath::Sign( delta_current_view_z );
    const auto velocity_sign = FMath::Sign( VelocityZ );

    if ( delta_sign == velocity_sign )
    {
        CurrentViewLocationZ = FMath::FInterpTo( CurrentViewLocationZ, view_location.Z, delta_time, CurrentInterpolationSpeed );
    }

    new_view_location.Z = CurrentViewLocationZ;

    if ( bCorrectPitchWithVelocityZ )
    {
        bool should_update_pitch = false;

        if ( VelocityZ < -600.0f )
        {
            PitchFromVelocityZ = VelocityZToPitchCurve.GetRichCurveConst()->Eval( VelocityZ );
            should_update_pitch = true;
        }
        else if ( bJustLanded )
        {
            TimeSinceLanded += delta_time;

            if ( TimeSinceLanded > 1.0f )
            {
                bJustLanded = false;
            }
            else
            {
                PitchFromVelocityZ = PitchWhenGrounded;
                should_update_pitch = true;
            }
        }
        else
        {
            PitchFromVelocityZ = view_rotation.Pitch;
        }

        if ( should_update_pitch )
        {
            CurrentPitch = FMath::FInterpTo( CurrentPitch, PitchFromVelocityZ, delta_time, 2.0f );
            new_view_rotation.Pitch = CurrentPitch;
        }
    }
}

bool UGBFCameraModifierDynamicZ::ProcessViewRotation( AActor * ViewTarget, float DeltaTime, FRotator & OutViewRotation, FRotator & OutDeltaRot )
{
    return false;
}

void UGBFCameraModifierDynamicZ::DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const
{
    auto & display_debug_manager = canvas->DisplayDebugManager;

    display_debug_manager.DrawString( FString::Printf( TEXT( "CurrentMovementMode: %s" ), *UEnum::GetValueAsString( CurrentMovementMode ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "LastGroundedPositionZ: %s" ), *FString::SanitizeFloat( LastGroundedPositionZ ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "DeltaLastGroundedPositionZ: %s" ), *FString::SanitizeFloat( DeltaLastGroundedPositionZ ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "VelocityZ: %s" ), *FString::SanitizeFloat( VelocityZ ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "CurrentInterpolationSpeed: %s" ), *FString::SanitizeFloat( CurrentInterpolationSpeed ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "CurrentViewLocationZ: %s" ), *FString::SanitizeFloat( CurrentViewLocationZ ) ) );

    if ( bCorrectPitchWithVelocityZ )
    {
        display_debug_manager.DrawString( FString::Printf( TEXT( "PitchFromVelocityZ: %s" ), *FString::SanitizeFloat( PitchFromVelocityZ ) ) );
        display_debug_manager.DrawString( FString::Printf( TEXT( "CurrentPitch: %s" ), *FString::SanitizeFloat( PitchFromVelocityZ ) ) );
    }
}