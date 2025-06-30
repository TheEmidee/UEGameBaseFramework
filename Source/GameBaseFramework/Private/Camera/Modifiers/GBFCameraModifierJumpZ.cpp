#include "Camera/Modifiers/GBFCameraModifierJumpZ.h"

#include <Engine/Canvas.h>
#include <GameFramework/Character.h>
#include <GameFramework/CharacterMovementComponent.h>

UGBFCameraModifierJumpZ::UGBFCameraModifierJumpZ() :
    LandingTransitionTime( 0.5f ),
    DistanceFromLastGroundedPositionToResetModifier( 50.0f ),
    DefaultInterpolationSpeed( 10.0f ),
    CurrentState( EState::WaitingForJump ),
    LastGroundedCameraZPosition( 0.0f ),
    LastGroundedCharacterZPosition( 0.0f ),
    CurrentCharacterZPosition( 0.0f ),
    LerpEndCameraZPosition( 0.0f ),
    LandingTransitionRemainingTime( 0.0f )
{
}

bool UGBFCameraModifierJumpZ::IsDisabled() const
{
    if ( Super::IsDisabled() )
    {
        return true;
    }

    return Cast< ACharacter >( GetViewTarget() ) == nullptr;
}

void UGBFCameraModifierJumpZ::ModifyCamera( const float delta_time, const FVector view_location, FRotator view_rotation, float fov, FVector & new_view_location, FRotator & new_view_rotation, float & new_fov )
{
    const auto * character = Cast< ACharacter >( GetViewTarget() );
    if ( character == nullptr )
    {
        return;
    }

    const auto * cmc = character->GetCharacterMovement();
    if ( cmc == nullptr )
    {
        return;
    }

    if ( !CurrentCameraZPosition.IsSet() )
    {
        CurrentCameraZPosition = view_location.Z;
    }

    CurrentCharacterZPosition = character->GetActorLocation().Z;

    switch ( CurrentState )
    {
        case EState::WaitingForJump:
        {
            if ( character->bWasJumping )
            {
                CurrentState = EState::Jumping;
                break;
            }
        }
        break;
        case EState::Jumping:
        {
            if ( cmc->MovementMode == MOVE_Walking )
            {
                LandingTransitionRemainingTime = LandingTransitionTime;

                CurrentState = EState::Landing;
                break;
            }

            if ( character->GetVelocity().Z <= 0.0f && CurrentCharacterZPosition < LastGroundedCharacterZPosition - DistanceFromLastGroundedPositionToResetModifier )
            {
                LandingTransitionRemainingTime = LandingTransitionTime;

                CurrentState = EState::Landing;
                break;
            }
        }
        break;
        case EState::Landing:
        {
            if ( character->bWasJumping &&
                 FMath::Abs( CurrentCharacterZPosition - LastGroundedCharacterZPosition ) < DistanceFromLastGroundedPositionToResetModifier )
            {
                LastGroundedCharacterZPosition = CurrentCharacterZPosition;
                LastGroundedCameraZPosition = CurrentCameraZPosition.GetValue();

                CurrentState = EState::Jumping;
                break;
            }

            if ( LandingTransitionRemainingTime <= 0.0f )
            {

                CurrentState = EState::WaitingForJump;
                break;
            }
        }
        break;
        default:
        {
            checkNoEntry();
        }
    }

    switch ( CurrentState )
    {
        case EState::WaitingForJump:
        {
            LastGroundedCameraZPosition = CurrentCameraZPosition.GetValue();
            LastGroundedCharacterZPosition = CurrentCharacterZPosition;

            new_view_location.Z = FMath::FInterpTo( CurrentCameraZPosition.GetValue(), view_location.Z, delta_time, DefaultInterpolationSpeed );
        }
        break;
        case EState::Jumping:
        {
            new_view_location.Z = LastGroundedCameraZPosition;
        }
        break;
        case EState::Landing:
        {
            LandingTransitionRemainingTime -= delta_time;

            if ( FMath::Abs( LastGroundedCharacterZPosition - CurrentCharacterZPosition ) <= DistanceFromLastGroundedPositionToResetModifier )
            {
                LerpEndCameraZPosition = LastGroundedCameraZPosition;
            }
            else
            {
                LerpEndCameraZPosition = view_location.Z;
            }

            new_view_location.Z = FMath::Lerp( LastGroundedCameraZPosition, LerpEndCameraZPosition, 1.0f - ( LandingTransitionRemainingTime / LandingTransitionTime ) );
        }
        break;
        default:
        {
            checkNoEntry();
        }
    }

    CurrentCameraZPosition = new_view_location.Z;
}

void UGBFCameraModifierJumpZ::DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const
{
    auto & display_debug_manager = canvas->DisplayDebugManager;

    switch ( CurrentState )
    {
        case EState::WaitingForJump:
        {
            display_debug_manager.DrawString( TEXT( "State : Waiting For Jump" ) );
            display_debug_manager.DrawString( FString::Printf( TEXT( "LastGroundedCameraZPosition: %s" ), *FString::SanitizeFloat( LastGroundedCameraZPosition ) ) );
            display_debug_manager.DrawString( FString::Printf( TEXT( "CurrentCameraZPosition: %s" ), *FString::SanitizeFloat( CurrentCameraZPosition.GetValue() ) ) );
        }
        break;
        case EState::Jumping:
        {
            display_debug_manager.DrawString( TEXT( "State : Jumping" ) );
            display_debug_manager.DrawString( FString::Printf( TEXT( "LastGroundedCameraZPosition: %s" ), *FString::SanitizeFloat( LastGroundedCameraZPosition ) ) );
            display_debug_manager.DrawString( FString::Printf( TEXT( "CurrentCameraZPosition: %s" ), *FString::SanitizeFloat( CurrentCameraZPosition.GetValue() ) ) );
        }
        break;
        case EState::Landing:
        {
            display_debug_manager.DrawString( TEXT( "State : Landing" ) );
            display_debug_manager.DrawString( FString::Printf( TEXT( "LandingTransitionRemainingTime: %s" ), *FString::SanitizeFloat( LandingTransitionRemainingTime ) ) );
            display_debug_manager.DrawString( FString::Printf( TEXT( "CurrentCameraZPosition: %s" ), *FString::SanitizeFloat( CurrentCameraZPosition.GetValue() ) ) );
            display_debug_manager.DrawString( FString::Printf( TEXT( "LerpEndCameraZPosition: %s" ), *FString::SanitizeFloat( LerpEndCameraZPosition ) ) );
        }
        break;
        default:
        {
            checkNoEntry();
        };
    }
}