#include "GameFramework/GBFGameMode.h"

#include "Characters/Components/GBFPawnExtensionComponent.h"
#include "Engine/GBFAssetManager.h"
#include "GBFLog.h"
#include "AI/GBFAIController.h"
#include "GameFramework/Components/GBFPlayerSpawningManagerComponent.h"
#include "GameFramework/GBFPlayerState.h"

#include <Engine/World.h>
#include <TimerManager.h>

const UGBFPawnData * AGBFGameMode::GetPawnDataForController( const AController * controller ) const
{
    // See if pawn data is already set on the player state
    if ( controller != nullptr )
    {
        if ( const auto * player_state = controller->GetPlayerState< AGBFPlayerState >() )
        {
            if ( const auto * pawn_data = player_state->GetPawnData< UGBFPawnData >() )
            {
                return pawn_data;
            }
        }
    }

    // If not, fall back to the the default for the current experience
    check( GameState );
    // :TODO: Experiences
    // ULyraExperienceManagerComponent * ExperienceComponent = GameState->FindComponentByClass< ULyraExperienceManagerComponent >();
    // check( ExperienceComponent );

    // if ( ExperienceComponent->IsExperienceLoaded() )
    //{
    //     const ULyraExperienceDefinition * Experience = ExperienceComponent->GetCurrentExperienceChecked();
    //     if ( Experience->DefaultPawnData != nullptr )
    //     {
    //         return Experience->DefaultPawnData;
    //     }

    //    // Experience is loaded and there's still no pawn data, fall back to the default for now
    //    return ULyraAssetManager::Get().GetDefaultPawnData();
    //}

    // Experience not loaded yet, so there is no pawn data to be had
    return UGBFAssetManager::Get().GetDefaultPawnData();
}

APawn * AGBFGameMode::SpawnDefaultPawnAtTransform_Implementation( AController * new_player, const FTransform & spawn_transform )
{
    FActorSpawnParameters SpawnInfo;
    SpawnInfo.Instigator = GetInstigator();
    SpawnInfo.ObjectFlags |= RF_Transient; // Never save the default player pawns into a map.
    SpawnInfo.bDeferConstruction = true;

    if ( auto * pawn_class = GetDefaultPawnClassForController( new_player ) )
    {
        if ( auto * spawned_pawn = GetWorld()->SpawnActor< APawn >( pawn_class, spawn_transform, SpawnInfo ) )
        {
            if ( auto * pawn_ext_comp = UGBFPawnExtensionComponent::FindPawnExtensionComponent( spawned_pawn ) )
            {
                if ( const auto * pawn_data = GetPawnDataForController( new_player ) )
                {
                    pawn_ext_comp->SetPawnData( pawn_data );
                }
                else
                {
                    UE_LOG( LogGBF, Error, TEXT( "Game mode was unable to set PawnData on the spawned pawn [%s]." ), *GetNameSafe( spawned_pawn ) );
                }
            }

            spawned_pawn->FinishSpawning( spawn_transform );

            return spawned_pawn;
        }

        UE_LOG( LogGBF, Error, TEXT( "Game mode was unable to spawn Pawn of class [%s] at [%s]." ), *GetNameSafe( pawn_class ), *spawn_transform.ToHumanReadableString() );
    }
    else
    {
        UE_LOG( LogGBF, Error, TEXT( "Game mode was unable to spawn Pawn due to NULL pawn class." ) );
    }

    return nullptr;
}

UClass * AGBFGameMode::GetDefaultPawnClassForController_Implementation( AController * controller )
{
    if ( const auto * pawn_data = GetPawnDataForController( controller ) )
    {
        if ( pawn_data->PawnClass )
        {
            return pawn_data->PawnClass;
        }
    }

    return Super::GetDefaultPawnClassForController_Implementation( controller );
}

bool AGBFGameMode::PlayerCanRestart_Implementation( APlayerController * player )
{
    return ControllerCanRestart( player );
}

bool AGBFGameMode::ControllerCanRestart( AController * controller )
{
    if ( auto * player_controller = Cast< APlayerController >( controller ) )
    {
        if ( !Super::PlayerCanRestart_Implementation( player_controller ) )
        {
            return false;
        }
    }
    else
    {
        // Bot version of Super::PlayerCanRestart_Implementation
        if ( controller == nullptr || controller->IsPendingKillPending() )
        {
            return false;
        }
    }

    if ( const auto * player_spawning_component = GameState->FindComponentByClass< UGBFPlayerSpawningManagerComponent >() )
    {
        return player_spawning_component->ControllerCanRestart( controller );
    }

    return true;
}

void AGBFGameMode::RequestPlayerRestartNextFrame( AController * controller, bool force_reset )
{
    if ( force_reset && controller != nullptr )
    {
        controller->Reset();
    }

    if ( auto * player_controller = Cast< APlayerController >( controller ) )
    {
        GetWorldTimerManager().SetTimerForNextTick( player_controller, &APlayerController::ServerRestartPlayer_Implementation );
    }
    else if ( auto * BotController = Cast< AGBFAIController >( controller ) )
    {
        GetWorldTimerManager().SetTimerForNextTick( BotController, &AGBFAIController::ServerRestartController );
    }
}

AActor * AGBFGameMode::ChoosePlayerStart_Implementation( AController * player )
{
    if ( auto * player_spawning_component = GameState->FindComponentByClass< UGBFPlayerSpawningManagerComponent >() )
    {
        return player_spawning_component->ChoosePlayerStart( player );
    }

    return Super::ChoosePlayerStart_Implementation( player );
}

void AGBFGameMode::FinishRestartPlayer( AController * new_player, const FRotator & start_rotation )
{
    if ( auto * player_spawning_component = GameState->FindComponentByClass< UGBFPlayerSpawningManagerComponent >() )
    {
        player_spawning_component->FinishRestartPlayer( new_player, start_rotation );
    }

    Super::FinishRestartPlayer( new_player, start_rotation );
}

// bool AGBFGameMode::UpdatePlayerStartSpot( AController * /*player*/, const FString & /*portal*/, FString & /*out_error_message*/ )
//{
//     // Do nothing, we'll wait until PostLogin when we try to spawn the player for real.
//     // Doing anything right now is no good, systems like team assignment haven't even occurred yet.
//     return true;
// }

// void AGBFGameMode::OnPostLogin( AController * new_player )
//{
//     Super::OnPostLogin( new_player );
//
//     OnGameModeCombinedPostLoginDelegate.Broadcast( this, new_player );
// }

// void AGBFGameMode::FailedToRestartPlayer( AController * new_player )
//{
//     Super::FailedToRestartPlayer( new_player );
//
//     // If we tried to spawn a pawn and it failed, lets try again *note* check if there's actually a pawn class
//     // before we try this forever.
//     if ( auto * pawn_class = GetDefaultPawnClassForController( new_player ) )
//     {
//         if ( auto * new_pc = Cast< APlayerController >( new_player ) )
//         {
//             // If it's a player don't loop forever, maybe something changed and they can no longer restart if so stop trying.
//             if ( PlayerCanRestart( new_pc ) )
//             {
//                 RequestPlayerRestartNextFrame( new_player, false );
//             }
//             else
//             {
//                 UE_LOG( LogGBF, Verbose, TEXT( "FailedToRestartPlayer(%s) and PlayerCanRestart returned false, so we're not going to try again." ), *GetPathNameSafe( new_player ) );
//             }
//         }
//         else
//         {
//             RequestPlayerRestartNextFrame( new_player, false );
//         }
//     }
//     else
//     {
//         UE_LOG( LogGBF, Verbose, TEXT( "FailedToRestartPlayer(%s) but there's no pawn class so giving up." ), *GetPathNameSafe( new_player ) );
//     }
// }