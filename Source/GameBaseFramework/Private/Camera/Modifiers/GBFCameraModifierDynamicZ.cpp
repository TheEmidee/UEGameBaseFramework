#include "Camera/Modifiers/GBFCameraModifierDynamicZ.h"

#include "Kismet/KismetStringLibrary.h"

#include <Engine/Canvas.h>
#include <GameFramework/Character.h>
#include <GameFramework/CharacterMovementComponent.h>

UGBFCameraModifierDynamicZ::UGBFCameraModifierDynamicZ() :
    bUpdateFromDistanceFromLastGroundedLocation( true ),
    bUpdateCameraZOnLanding( false ),
    LandingInterpolationTime( 0.5f ),
    LastGroundedCharacterPositionZ( 0.0f ),
    LastGroundedCameraPositionZ( 0.0f ),
    CurrentViewLocationZ( 0.0f ),
    TargetViewLocationZ( 0.0f ),
    CurrentInterpolationSpeed( 0.0f ),
    DeltaLastGroundedPositionZ( 0.0f ),
    VelocityZ( 0.0f ),
    PreviousMovementMode(),
    bInterpolateLanding( false ),
    LandingInterpolationRemainingTime( 0.0f )
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

void UGBFCameraModifierDynamicZ::ModifyCamera( const float delta_time, const FVector view_location, FRotator /* view_rotation */, float /* fov */, FVector & new_view_location, FRotator & /* new_view_rotation */, float & /* new_fov */ )
{
    const auto * character = Cast< ACharacter >( GetViewTarget() );

    if ( character == nullptr )
    {
        return;
    }

    const auto current_movement_mode = character->GetCharacterMovement()->MovementMode;

    const auto character_z = character->GetActorLocation().Z;
    VelocityZ = character->GetCharacterMovement()->Velocity.Z;

    ON_SCOPE_EXIT
    {
        PreviousMovementMode = current_movement_mode;
    };

    // if ( bUpdateFromDistanceFromLastGroundedLocation )
    {
        if ( current_movement_mode == MOVE_Walking )
        {
            if ( PreviousMovementMode == MOVE_Falling && bUpdateCameraZOnLanding && FMath::IsNearlyZero( LandingInterpolationRemainingTime ) )
            {
                LandingInterpolationRemainingTime = LandingInterpolationTime;
            }

            if ( LandingInterpolationRemainingTime > 0.0f )
            {
                LandingInterpolationRemainingTime -= delta_time;

                if ( LandingInterpolationRemainingTime <= 0.0f )
                {
                    LandingInterpolationRemainingTime = 0.0f;
                }
                else
                {
                    if ( FMath::Abs( character_z - LastGroundedCharacterPositionZ ) < 1.0f )
                    {
                        TargetViewLocationZ = LastGroundedCameraPositionZ;
                    }
                    else
                    {
                        TargetViewLocationZ = view_location.Z;
                    }

                    CurrentViewLocationZ = FMath::Lerp( CurrentViewLocationZ, TargetViewLocationZ, 1.0f - ( LandingInterpolationRemainingTime / LandingInterpolationTime ) );
                    new_view_location.Z = CurrentViewLocationZ;
                    return;
                }
            }

            LastGroundedCharacterPositionZ = character_z;
            LastGroundedCameraPositionZ = view_location.Z;
            CurrentViewLocationZ = LastGroundedCameraPositionZ;
        }
        else if ( current_movement_mode == MOVE_Falling )
        {
            LandingInterpolationRemainingTime = 0.0f;

            DeltaLastGroundedPositionZ = character_z - LastGroundedCharacterPositionZ;
            CurrentInterpolationSpeed = FMath::Abs( DistanceFromLastGroundedLocationInterpolationSpeedCurve.GetRichCurveConst()->Eval( DeltaLastGroundedPositionZ ) );

            if ( bUpdateCameraZOnLanding )
            {
                if ( VelocityZ > 0.0f || character_z > ( LastGroundedCharacterPositionZ - 10.0f ) )
                {
                    new_view_location.Z = LastGroundedCameraPositionZ;
                    return;
                }
            }

            TargetViewLocationZ = view_location.Z;
            CurrentViewLocationZ = FMath::FInterpTo( CurrentViewLocationZ, TargetViewLocationZ, delta_time, CurrentInterpolationSpeed );
            new_view_location.Z = CurrentViewLocationZ;
        }
    }
}

void UGBFCameraModifierDynamicZ::DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const
{
    auto & display_debug_manager = canvas->DisplayDebugManager;

    display_debug_manager.DrawString( FString::Printf( TEXT( "LastGroundedCharacterPositionZ: %s" ), *FString::SanitizeFloat( LastGroundedCharacterPositionZ ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "LastGroundedCameraPositionZ: %s" ), *FString::SanitizeFloat( LastGroundedCameraPositionZ ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "DeltaLastGroundedPositionZ: %s" ), *FString::SanitizeFloat( DeltaLastGroundedPositionZ ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "VelocityZ: %s" ), *FString::SanitizeFloat( VelocityZ ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "CurrentInterpolationSpeed: %s" ), *FString::SanitizeFloat( CurrentInterpolationSpeed ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "UpdateCameraZOnLandingnLanding?: %s" ), *UKismetStringLibrary::Conv_BoolToString( bUpdateCameraZOnLanding ) ) );

    if ( bUpdateCameraZOnLanding )
    {
        display_debug_manager.DrawString( FString::Printf( TEXT( "InterpolateLanding: %s" ), *UKismetStringLibrary::Conv_BoolToString( bInterpolateLanding ) ) );
    }

    display_debug_manager.DrawString( FString::Printf( TEXT( "CurrentViewLocationZ: %s" ), *FString::SanitizeFloat( CurrentViewLocationZ ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "TargetViewLocationZ: %s" ), *FString::SanitizeFloat( TargetViewLocationZ ) ) );
}