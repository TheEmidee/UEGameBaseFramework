#include "Camera/Modifiers/GBFCameraModifier.h"

#include "Camera/GBFPlayerCameraManager.h"

#include <Engine/Canvas.h>
#include <GameplayTagAssetInterface.h>

bool UGBFCameraModifier::IsDisabled() const
{
    if ( Super::IsDisabled() )
    {
        return true;
    }

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

void UGBFCameraModifier::AddedToCamera( APlayerCameraManager * player_camera_manager )
{
    // :NOTE: Don't call Super, as this binds functions to when the manager is destroyed, because modifiers are statically added to it
    // Here we can dynamically add and remove modifiers
    CameraOwner = player_camera_manager;
    
    ReceiveAddedToCameraManager( CameraOwner );
}

void UGBFCameraModifier::OnViewTargetChanged( AActor * view_target )
{
}

void UGBFCameraModifier::RemovedFromCameraManager()
{
    ReceiveRemovedFromCameraManager();
    CameraOwner = nullptr;
}

void UGBFCameraModifier::DisplayDebug( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos )
{
    auto & display_debug_manager = canvas->DisplayDebugManager;

    if ( IsDisabled() )
    {
        display_debug_manager.SetDrawColor( FColor::Red );
        display_debug_manager.DrawString( TEXT( "Disabled" ) );
        display_debug_manager.SetDrawColor( FColor::Cyan );
        return;
    }

    if ( bDebug )
    {
        DisplayDebugInternal( canvas, debug_display, yl, y_pos );
    }
}

void UGBFCameraModifier::DisplayDebugInternal( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos ) const
{
}