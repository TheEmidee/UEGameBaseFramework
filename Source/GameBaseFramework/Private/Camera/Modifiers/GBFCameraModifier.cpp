#include "Camera/Modifiers/GBFCameraModifier.h"

#include "Camera/GBFPlayerCameraManager.h"

#include <Engine/Canvas.h>
#include <GameplayTagAssetInterface.h>

bool UGBFCameraModifier::IsDisabled()
{
    if ( TagRequirements.IsEmpty() )
    {
        return false;
    }

    if ( const auto * tag_owner = Cast< IGameplayTagAssetInterface >( GetViewTarget() ) )
    {
        FGameplayTagContainer character_tags;
        tag_owner->GetOwnedGameplayTags( character_tags );

        return !TagRequirements.RequirementsMet( character_tags );
    }

    return false;
}

bool UGBFCameraModifier::ProcessViewRotation( AActor * view_target, float delta_time, FRotator & view_rotation, FRotator & delta_rot )
{
    return false;
}

bool UGBFCameraModifier::ModifyCamera( float delta_time, FVector view_location, FRotator view_rotation, float fov, FVector & new_view_location, FRotator & new_view_rotation, float & new_fov )
{
    return false;
}

void UGBFCameraModifier::AddedToCameraManager( AGBFPlayerCameraManager & player_camera_manager )
{
    OwnerPlayerCameraManager = &player_camera_manager;
    ReceiveAddedToCameraManager( OwnerPlayerCameraManager );
}

void UGBFCameraModifier::OnViewTargetChanged( AActor * view_target )
{
}

void UGBFCameraModifier::RemovedFromCameraManager()
{
    ReceiveRemovedFromCameraManager();
    OwnerPlayerCameraManager = nullptr;
}

void UGBFCameraModifier::DisplayDebug( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos )
{
    auto & display_debug_manager = canvas->DisplayDebugManager;
    display_debug_manager.DrawString( GetName() );

    if ( IsDisabled() )
    {
        display_debug_manager.SetDrawColor( FColor::Red );
        display_debug_manager.DrawString( TEXT( "Disabled" ) );
        display_debug_manager.SetDrawColor( FColor::Cyan );
        return;
    }

    DisplayDebugInternal( canvas, debug_display, yl, y_pos );
}

AActor * UGBFCameraModifier::GetViewTarget() const
{
    return OwnerPlayerCameraManager != nullptr
               ? OwnerPlayerCameraManager->GetViewTarget()
               : nullptr;
}

void UGBFCameraModifier::DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const
{
}