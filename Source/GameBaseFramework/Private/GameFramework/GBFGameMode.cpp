#include "GameFramework/GBFGameMode.h"

#include "GBFLog.h"
#include "TimerManager.h"
#include "AI/GBFAIController.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/GBFAssetManager.h"
#include "Engine/GBFHUD.h"
#include "Engine/World.h"
#include "GameFramework/GBFGameState.h"
#include "GameFramework/GBFPlayerController.h"
#include "GameFramework/GBFPlayerState.h"
#include "GameFramework/Components/GBFPlayerSpawningManagerComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Online/GBFGameSession.h"
#include "Phases/GBFGamePhaseSubsystem.h"

AGBFGameMode::AGBFGameMode()
{
    GameStateClass = AGBFGameState::StaticClass();
    GameSessionClass = AGBFGameSession::StaticClass();
    PlayerControllerClass = AGBFPlayerController::StaticClass();
    PlayerStateClass = AGBFPlayerState::StaticClass();
    HUDClass = AGBFHUD::StaticClass();
}

bool AGBFGameMode::PlayerCanRestart_Implementation(APlayerController* player)
{
    return ControllerCanRestart(player);
}

bool AGBFGameMode::ControllerCanRestart(AController* controller)
{
    if (auto* player_controller = Cast<APlayerController>(controller))
    {
        if (!Super::PlayerCanRestart_Implementation(player_controller))
        {
            return false;
        }
    }
    else
    {
        // Bot version of Super::PlayerCanRestart_Implementation
        if (controller == nullptr || controller->IsPendingKillPending())
        {
            return false;
        }
    }

    if (const auto* player_spawning_component = GameState->FindComponentByClass<UGBFPlayerSpawningManagerComponent>())
    {
        return player_spawning_component->ControllerCanRestart(controller);
    }

    return true;
}

void AGBFGameMode::RequestPlayerRestartNextFrame(AController* controller, bool force_reset)
{
    if (force_reset && controller != nullptr)
    {
        controller->Reset();
    }

    if (auto* player_controller = Cast<APlayerController>(controller))
    {
        GetWorldTimerManager().SetTimerForNextTick(player_controller, &APlayerController::ServerRestartPlayer_Implementation);
    }
    else if (auto* bot_controller = Cast<AGBFAIController>(controller))
    {
        GetWorldTimerManager().SetTimerForNextTick(bot_controller, &AGBFAIController::ServerRestartController);
    }
}

AActor* AGBFGameMode::ChoosePlayerStart_Implementation(AController* player)
{
    if (auto* player_spawning_component = GameState->FindComponentByClass<UGBFPlayerSpawningManagerComponent>())
    {
        return player_spawning_component->ChoosePlayerStart(player);
    }

    return Super::ChoosePlayerStart_Implementation(player);
}

void AGBFGameMode::Logout(AController* exiting_controller)
{
    Super::Logout(exiting_controller);

    OnControllerLogoutDelegate.Broadcast(this, exiting_controller);
}

void AGBFGameMode::GenericPlayerInitialization(AController* new_player)
{
    Super::GenericPlayerInitialization(new_player);

    OnPlayerInitializedDelegate.Broadcast(this, new_player);
}

FString AGBFGameMode::InitNewPlayer(APlayerController* new_player_controller, const FUniqueNetIdRepl& unique_id, const FString& options, const FString& portal)
{
    const auto error_message = Super::InitNewPlayer(new_player_controller, unique_id, options, portal);

    if (!error_message.IsEmpty())
    {
        return error_message;
    }

    if (auto* player_state = new_player_controller->GetPlayerState<AGBFPlayerState>())
    {
        player_state->SetConnectionOptions(options);
    }
    else
    {
        return TEXT("The player state must inherit from AGBFPlayerState");
    }

    return FString();
}

bool AGBFGameMode::ShouldSpawnAtStartSpot(AController* /*player*/)
{
    // We never want to use the start spot, always use the spawn management component.
    return false;
}

void AGBFGameMode::FinishRestartPlayer(AController* new_player, const FRotator& start_rotation)
{
    if (auto* player_spawning_component = GameState->FindComponentByClass<UGBFPlayerSpawningManagerComponent>())
    {
        player_spawning_component->FinishRestartPlayer(new_player, start_rotation);
    }

    Super::FinishRestartPlayer(new_player, start_rotation);
}

bool AGBFGameMode::UpdatePlayerStartSpot(AController* /*player*/, const FString& /*portal*/, FString& /*out_error_message*/)
{
    // Do nothing, we'll wait until PostLogin when we try to spawn the player for real.
    // Doing anything right now is no good, systems like team assignment haven't even occurred yet.
    return true;
}

void AGBFGameMode::FailedToRestartPlayer(AController* new_player)
{
    Super::FailedToRestartPlayer(new_player);

    // If we tried to spawn a pawn and it failed, lets try again *note* check if there's actually a pawn class
    // before we try this forever.
    if (GetDefaultPawnClassForController(new_player) != nullptr)
    {
        if (auto* new_pc = Cast<APlayerController>(new_player))
        {
            // If it's a player don't loop forever, maybe something changed and they can no longer restart if so stop trying.
            if (PlayerCanRestart(new_pc))
            {
                RequestPlayerRestartNextFrame(new_player, false);
            }
            else
            {
                UE_LOG(LogGBF, Verbose, TEXT( "FailedToRestartPlayer(%s) and PlayerCanRestart returned false, so we're not going to try again." ), *GetPathNameSafe( new_player ));
            }
        }
        else
        {
            RequestPlayerRestartNextFrame(new_player, false);
        }
    }
    else
    {
        UE_LOG(LogGBF, Verbose, TEXT( "FailedToRestartPlayer(%s) but there's no pawn class so giving up." ), *GetPathNameSafe( new_player ));
    }
}
