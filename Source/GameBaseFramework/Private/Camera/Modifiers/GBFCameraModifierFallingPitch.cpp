#include "Camera/Modifiers/GBFCameraModifierFallingPitch.h"

#include "Camera/Modifiers/GBFCameraModifierUtils.h"

#include <Engine/Canvas.h>
#include <GameFramework/Character.h>
#include <GameFramework/CharacterMovementComponent.h>

UGBFCameraModifierFallingPitch::UGBFCameraModifierFallingPitch() :
    VelocityZThreshold( -600.0f ),
    bRestorePitchWhenLanded( true ),
    LandingTransitionTime( 0.5f ),
    LandingInterpolationSpeed( 5.0f ),
    State( EState::WaitingFall ),
    CurrentVelocityZ( 0.0f ),
    CurrentInterpolationSpeed( 0.0f ),
    CurrentPitch( 0.0f ),
    TargetPitch( 0.0f ),
    FallingInitialPitch( 0.0f ),
    LandingTransitionRemainingTime( 0 )
{
}

bool UGBFCameraModifierFallingPitch::ProcessViewRotation( AActor * view_target, float delta_time, FRotator & view_rotation, FRotator & delta_rotation )
{
    const auto * vt = GetViewTarget();

    if ( vt == nullptr )
    {
        return false;
    }

    const auto velocity = vt->GetVelocity();
    CurrentVelocityZ = velocity.Z;

    const auto * character = Cast< ACharacter >( GetViewTarget() );

    if ( character == nullptr )
    {
        return false;
    }

    const auto current_movement_mode = character->GetCharacterMovement()->MovementMode;

    switch ( State )
    {
        case EState::WaitingFall:
        {
            if ( current_movement_mode == MOVE_Falling && CurrentVelocityZ < VelocityZThreshold )
            {
                State = EState::Falling;
                FallingInitialPitch = FGBFCameraModifierUtilsLibrary::ClampAngle( view_rotation.Pitch );
                CurrentPitch = FallingInitialPitch;
            }
        }
        break;
        case EState::Falling:
        {
            if ( current_movement_mode == MOVE_Walking )
            {
                if ( bRestorePitchWhenLanded )
                {
                    State = EState::Landing;
                    LandingTransitionRemainingTime = LandingTransitionTime;
                    TargetPitch = FallingInitialPitch;
                    CurrentInterpolationSpeed = LandingInterpolationSpeed;
                }
                else
                {
                    State = EState::WaitingFall;
                }
            }
            else
            {
                CurrentInterpolationSpeed = VelocityZToInterpolationSpeedCurve.GetRichCurveConst()->Eval( CurrentVelocityZ );
                TargetPitch = FGBFCameraModifierUtilsLibrary::ClampAngle( VelocityZToPitchCurve.GetRichCurveConst()->Eval( CurrentVelocityZ ) );

                if ( CurrentPitch > TargetPitch )
                {
                    CurrentPitch = FMath::FInterpTo( CurrentPitch, TargetPitch, delta_time, CurrentInterpolationSpeed );
                    view_rotation.Pitch = CurrentPitch;
                }
            }
        }
        break;
        case EState::Landing:
        {
            LandingTransitionRemainingTime -= delta_time;

            if ( LandingTransitionRemainingTime <= 0.0f )
            {
                State = EState::WaitingFall;
            }
            else
            {
                CurrentPitch = FMath::FInterpTo( CurrentPitch, TargetPitch, delta_time, CurrentInterpolationSpeed );
                view_rotation.Pitch = CurrentPitch;
            }
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

void UGBFCameraModifierFallingPitch::DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const
{
    auto & display_debug_manager = canvas->DisplayDebugManager;

    switch ( State )
    {
        case EState::WaitingFall:
        {
            display_debug_manager.DrawString( TEXT( "State: Waiting Free Fall" ) );
            display_debug_manager.DrawString( FString::Printf( TEXT( "VelocityZThreshold: %s" ), *FString::SanitizeFloat( VelocityZThreshold ) ) );
            display_debug_manager.DrawString( FString::Printf( TEXT( "CurrentVelocityZ: %s" ), *FString::SanitizeFloat( CurrentVelocityZ ) ) );
            display_debug_manager.DrawString( FString::Printf( TEXT( "FallingInitialPitch: %s" ), *FString::SanitizeFloat( FallingInitialPitch ) ) );
        }
        break;
        case EState::Falling:
        {
            display_debug_manager.DrawString( TEXT( "State: Falling" ) );
            display_debug_manager.DrawString( FString::Printf( TEXT( "CurrentVelocityZ: %s" ), *FString::SanitizeFloat( CurrentVelocityZ ) ) );
            display_debug_manager.DrawString( FString::Printf( TEXT( "CurrentInterpolationSpeed: %s" ), *FString::SanitizeFloat( CurrentInterpolationSpeed ) ) );
            display_debug_manager.DrawString( FString::Printf( TEXT( "TargetPitch: %s" ), *FString::SanitizeFloat( TargetPitch ) ) );
            display_debug_manager.DrawString( FString::Printf( TEXT( "CurrentPitch: %s" ), *FString::SanitizeFloat( CurrentPitch ) ) );
        }
        break;
        case EState::Landing:
        {
            display_debug_manager.DrawString( TEXT( "State: Landing" ) );
        }
        break;
        default:
        {
            checkNoEntry();
        }
        break;
    }
}