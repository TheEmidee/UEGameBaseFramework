#include "Camera/GBFPlayerCameraManager.h"

#include "Camera/GBFCameraComponent.h"

#include <Engine/Canvas.h>
#include <Engine/Engine.h>
#include <GameFramework/Pawn.h>
#include <GameFramework/PlayerController.h>

void AGBFPlayerCameraManager::DisplayDebug( UCanvas * canvas, const FDebugDisplayInfo & debug_display, float & yl, float & y_pos )
{
    check( canvas != nullptr );

    auto & display_debug_manager = canvas->DisplayDebugManager;

    display_debug_manager.SetFont( GEngine->GetSmallFont() );
    display_debug_manager.SetDrawColor( FColor::Yellow );
    display_debug_manager.DrawString( FString::Printf( TEXT( "GBFPlayerCameraManager: %s" ), *GetNameSafe( this ) ) );

    Super::DisplayDebug( canvas, debug_display, yl, y_pos );

    const auto * pawn = PCOwner ? PCOwner->GetPawn() : nullptr;

    if ( const auto * camera_component = UGBFCameraComponent::FindCameraComponent( pawn ) )
    {
        camera_component->DrawDebug( canvas );
    }
}