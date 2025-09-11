#include "GameFramework/GBFPlayerController.h"

#include "CommonInputSubsystem.h"
#include "GameBaseFrameworkDeveloperSettings.h"
#include "GBFLog.h"
#include "TimerManager.h"
#include "Engine/GBFLocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/GBFPlayerState.h"

AGBFPlayerController::AGBFPlayerController() :
    LastSeenPlayerState(nullptr)
{
}

UGBFLocalPlayer* AGBFPlayerController::GetGBFLocalPlayer() const
{
    return Cast<UGBFLocalPlayer>(GetLocalPlayer());
}

void AGBFPlayerController::EnableInput(class APlayerController* player_controller)
{
    if (GetWorldTimerManager().IsTimerActive(ReEnableInputTimerHandle))
    {
        return;
    }

    Super::EnableInput(player_controller);
}

void AGBFPlayerController::DisableInput(class APlayerController* player_controller)
{
    GetWorldTimerManager().ClearTimer(ReEnableInputTimerHandle);
    Super::DisableInput(player_controller);
}

void AGBFPlayerController::ForceEnableInput(class APlayerController* player_controller)
{
    GetWorldTimerManager().ClearTimer(ReEnableInputTimerHandle);
    Super::EnableInput(player_controller);
}

void AGBFPlayerController::DisableInputForDuration(const float duration)
{
    DisableInput(nullptr);

    auto new_duration = duration;

    if (ensureMsgf(duration > 0.0f, TEXT( "DisableInputForDuration must be called with a valid duration" )))
    {
        new_duration = 1.0f;
    }

    if (!ReEnableInputTimerHandle.IsValid() || GetWorldTimerManager().GetTimerRemaining(ReEnableInputTimerHandle) < new_duration)
    {
        auto enable_input = [ this ]()
        {
            GetWorldTimerManager().ClearTimer(ReEnableInputTimerHandle);
            EnableInput(nullptr);
        };

        GetWorldTimerManager().SetTimer(ReEnableInputTimerHandle, enable_input, new_duration, false);
    }
}

void AGBFPlayerController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    BroadcastOnPlayerStateChanged();
}

void AGBFPlayerController::InitPlayerState()
{
    Super::InitPlayerState();
    BroadcastOnPlayerStateChanged();
}

void AGBFPlayerController::CleanupPlayerState()
{
    Super::CleanupPlayerState();
    BroadcastOnPlayerStateChanged();
}

void AGBFPlayerController::ServerCheat_Implementation(const FString& message)
{
#if USING_CHEAT_MANAGER
    if (CheatManager != nullptr)
    {
        UE_LOG(LogGBF, Warning, TEXT( "ServerCheat: %s" ), *message);
        ClientMessage(ConsoleCommand(message));
    }
#endif
}

bool AGBFPlayerController::ServerCheat_Validate(const FString& /*message*/)
{
    return true;
}

void AGBFPlayerController::ServerCheatAll_Implementation(const FString& message)
{
#if USING_CHEAT_MANAGER
    if (CheatManager)
    {
        UE_LOG(LogGBF, Warning, TEXT( "ServerCheatAll: %s" ), *message);
        for (auto iterator = GetWorld()->GetPlayerControllerIterator(); iterator; ++iterator)
        {
            if (auto* pc = Cast<AGBFPlayerController>(*iterator))
            {
                pc->ClientMessage(pc->ConsoleCommand(message));
            }
        }
    }
#endif // #if USING_CHEAT_MANAGER
}

bool AGBFPlayerController::ServerCheatAll_Validate(const FString& /*message*/)
{
    return true;
}

void AGBFPlayerController::AddCheats(bool force)
{
#if USING_CHEAT_MANAGER
    Super::AddCheats(true);
#else  // #if USING_CHEAT_MANAGER
    Super::AddCheats(force);
#endif //
}

void AGBFPlayerController::OnPossess(APawn* pawn)
{
    Super::OnPossess(pawn);

#if WITH_SERVER_CODE && WITH_EDITOR
    if (GIsEditor && (pawn != nullptr) && (GetPawn() == pawn))
    {
        for (const auto& cheat_row : GetDefault<UGameBaseFrameworkDeveloperSettings>()->CheatsToRun)
        {
            if (cheat_row.Phase == EGBFCheatExecutionTime::OnPlayerPawnPossession)
            {
                ConsoleCommand(cheat_row.Cheat, /*bWriteToLog=*/true);
            }
        }
    }
#endif
}

void AGBFPlayerController::OnPlayerStateChanged()
{
}

void AGBFPlayerController::BroadcastOnPlayerStateChanged()
{
    OnPlayerStateChanged();

    LastSeenPlayerState = PlayerState;
}
