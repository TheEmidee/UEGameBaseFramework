#include "Camera/Modifiers/GBFCameraModifierDynamicZ.h"

#include <Engine/Canvas.h>
#include <GameFramework/Character.h>
#include <GameFramework/CharacterMovementComponent.h>

UGBFCameraModifierDynamicZ::UGBFCameraModifierDynamicZ() :
    LastGroundedPositionZ( 0.0f ),
    CurrentViewLocationZ( 0.0f ),
    TargetViewLocationZ( 0.0f ),
    CurrentInterpolationSpeed( 0.0f ),
    DeltaLastGroundedPositionZ( 0.0f ),
    VelocityZ( 0.0f )
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

    const auto current_movement_mode = character->GetCharacterMovement()->MovementMode;

    const auto character_z = character->GetActorLocation().Z;
    VelocityZ = character->GetCharacterMovement()->Velocity.Z;

    if ( current_movement_mode == MOVE_Walking )
    {
        LastGroundedPositionZ = character_z;
        CurrentViewLocationZ = view_location.Z;
    }
    else
    {
        DeltaLastGroundedPositionZ = character_z - LastGroundedPositionZ;
        CurrentInterpolationSpeed = FMath::Abs( DistanceFromLastGroundedLocationInterpolationSpeedCurve.GetRichCurveConst()->Eval( DeltaLastGroundedPositionZ ) );
    }

    if ( current_movement_mode == MOVE_Falling )
    {
        TargetViewLocationZ = view_location.Z;
        CurrentViewLocationZ = FMath::FInterpTo( CurrentViewLocationZ, TargetViewLocationZ, delta_time, CurrentInterpolationSpeed );
        new_view_location.Z = CurrentViewLocationZ;
    }
}

void UGBFCameraModifierDynamicZ::DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const
{
    auto & display_debug_manager = canvas->DisplayDebugManager;

    display_debug_manager.DrawString( FString::Printf( TEXT( "LastGroundedPositionZ: %s" ), *FString::SanitizeFloat( LastGroundedPositionZ ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "DeltaLastGroundedPositionZ: %s" ), *FString::SanitizeFloat( DeltaLastGroundedPositionZ ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "VelocityZ: %s" ), *FString::SanitizeFloat( VelocityZ ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "CurrentInterpolationSpeed: %s" ), *FString::SanitizeFloat( CurrentInterpolationSpeed ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "CurrentViewLocationZ: %s" ), *FString::SanitizeFloat( CurrentViewLocationZ ) ) );
    display_debug_manager.DrawString( FString::Printf( TEXT( "TargetViewLocationZ: %s" ), *FString::SanitizeFloat( TargetViewLocationZ ) ) );
}