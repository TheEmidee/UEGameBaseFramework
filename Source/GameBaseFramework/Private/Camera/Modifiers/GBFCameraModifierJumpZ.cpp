#include "Camera/Modifiers/GBFCameraModifierJumpZ.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include <Engine/Canvas.h>
#include <GameFramework/Character.h>

UGBFCameraModifierJumpZ::UGBFCameraModifierJumpZ() :
    LandingTransitionTime( 0.5f ),
    LandOnSameHeightCheckTolerance( 1.0f ),
    DistanceFromLastGroundedPositionToResetModifier( 50.0f ),
    CurrentState( EState::WaitingForJump ),
    LastGroundedCameraZPosition( 0.0f ),
    LastGroundedCharacterZPosition( 0.0f ),
    CurrentCharacterZPosition( 0.0f ),
    CurrentCameraZPosition( 0.0f ),
    LerpStartCameraZPosition( 0.0f ),
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

void UGBFCameraModifierJumpZ::ModifyCamera( float delta_time, FVector view_location, FRotator view_rotation, float fov, FVector & new_view_location, FRotator & new_view_rotation, float & new_fov )
{
    auto * character = Cast< ACharacter >( GetViewTarget() );

    if ( character == nullptr )
    {
        return;
    }

    CurrentCharacterZPosition = character->GetActorLocation().Z;
    auto * cmc = character->GetCharacterMovement();

    switch ( CurrentState )
    {
        case EState::WaitingForJump:
        {
            if ( character->bWasJumping )
            {
                CurrentState = EState::Jumping;
            }
            else
            {
                LastGroundedCharacterZPosition = CurrentCharacterZPosition;
                LastGroundedCameraZPosition = view_location.Z;
            }
        }
        break;
        case EState::Jumping:
        {
            if ( cmc->MovementMode == MOVE_Walking )
            {
                CurrentState = EState::Landing;
                LandingTransitionRemainingTime = LandingTransitionTime;

                if ( FMath::IsNearlyEqual( CurrentCharacterZPosition, LastGroundedCharacterZPosition, LandOnSameHeightCheckTolerance ) )
                {
                    LerpStartCameraZPosition = LastGroundedCameraZPosition;
                    LerpEndCameraZPosition = LastGroundedCameraZPosition;

                    SpringArmComponent->bEnableCameraLag = false;
                }
                else
                {
                    LerpStartCameraZPosition = LastGroundedCameraZPosition;
                    LerpEndCameraZPosition = view_location.Z;
                }
            }

            if ( character->GetVelocity().Z <= 0.0f && CurrentCharacterZPosition < LastGroundedCharacterZPosition - DistanceFromLastGroundedPositionToResetModifier )
            {
                CurrentState = EState::WaitingForJump;
                return;
            }
            
            new_view_location.Z = LastGroundedCameraZPosition;
        }
        break;
        case EState::Landing:
        {
            if ( character->bWasJumping )
            {
                CurrentState = EState::Jumping;
                LastGroundedCameraZPosition = view_location.Z;
            }
            else
            {
                LandingTransitionRemainingTime -= delta_time;

                if ( LandingTransitionRemainingTime <= 0.0f )
                {
                    LandingTransitionRemainingTime = 0.0f;
                    CurrentState = EState::WaitingForJump;
                    SpringArmComponent->bEnableCameraLag = true;
                }
                else
                {
                    new_view_location.Z = FMath::Lerp( LerpStartCameraZPosition, LerpEndCameraZPosition, 1.0f - ( LandingTransitionRemainingTime / LandingTransitionTime ) );
                }
            }
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
            display_debug_manager.DrawString( FString::Printf( TEXT( "CurrentCameraZPosition: %s" ), *FString::SanitizeFloat( CurrentCameraZPosition ) ) );
        }
        break;
        case EState::Jumping:
        {
            display_debug_manager.DrawString( TEXT( "State : Jumping" ) );
            display_debug_manager.DrawString( FString::Printf( TEXT( "LastGroundedCameraZPosition: %s" ), *FString::SanitizeFloat( LastGroundedCameraZPosition ) ) );
            display_debug_manager.DrawString( FString::Printf( TEXT( "CurrentCameraZPosition: %s" ), *FString::SanitizeFloat( CurrentCameraZPosition ) ) );
        }
        break;
        case EState::Landing:
        {
            display_debug_manager.DrawString( TEXT( "State : Landing" ) );
            display_debug_manager.DrawString( FString::Printf( TEXT( "LandingTransitionRemainingTime: %s" ), *FString::SanitizeFloat( LandingTransitionRemainingTime ) ) );
            display_debug_manager.DrawString( FString::Printf( TEXT( "CurrentCameraZPosition: %s" ), *FString::SanitizeFloat( CurrentCameraZPosition ) ) );
        }
        break;
        default:
        {
            checkNoEntry();
        };
    }
}