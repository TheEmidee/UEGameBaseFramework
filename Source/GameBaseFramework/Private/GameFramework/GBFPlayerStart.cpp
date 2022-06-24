#include "GameFramework/GBFPlayerStart.h"

#include <Engine/World.h>
#include <GameFramework/GameModeBase.h>
#include <TimerManager.h>

EGBFPlayerStartLocationOccupancy AGBFPlayerStart::GetLocationOccupancy( AController * controller_pawn_to_fit ) const
{
    if ( auto * const world = GetWorld() )
    {
        if ( HasAuthority() )
        {
            if ( auto * auth_game_mode = world->GetAuthGameMode() )
            {
                const TSubclassOf< APawn > pawn_class = auth_game_mode->GetDefaultPawnClassForController( controller_pawn_to_fit );
                const auto pawn_to_fit = pawn_class ? GetDefault< APawn >( pawn_class ) : nullptr;

                auto actor_location = GetActorLocation();
                const auto actor_rotation = GetActorRotation();

                if ( !world->EncroachingBlockingGeometry( pawn_to_fit, actor_location, actor_rotation, nullptr ) )
                {
                    return EGBFPlayerStartLocationOccupancy::Empty;
                }
                if ( world->FindTeleportSpot( pawn_to_fit, actor_location, actor_rotation ) )
                {
                    return EGBFPlayerStartLocationOccupancy::Partial;
                }
            }
        }
    }

    return EGBFPlayerStartLocationOccupancy::Full;
}

bool AGBFPlayerStart::TryClaim( AController * occupying_controller )
{
    if ( occupying_controller != nullptr && !IsClaimed() )
    {
        ClaimingController = occupying_controller;
        if ( const auto * world = GetWorld() )
        {
            world->GetTimerManager().SetTimer( ExpirationTimerHandle, FTimerDelegate::CreateUObject( this, &AGBFPlayerStart::CheckUnclaimed ), ExpirationCheckInterval, true );
        }
        return true;
    }
    return false;
}

void AGBFPlayerStart::CheckUnclaimed()
{
    if ( ClaimingController != nullptr && ClaimingController->GetPawn() != nullptr && GetLocationOccupancy( ClaimingController ) == EGBFPlayerStartLocationOccupancy::Empty )
    {
        ClaimingController = nullptr;
        if ( const auto * world = GetWorld() )
        {
            world->GetTimerManager().ClearTimer( ExpirationTimerHandle );
        }
    }
}
