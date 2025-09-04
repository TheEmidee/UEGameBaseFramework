#pragma once

#include "ModularGameMode.h"
#include "CoreMinimal.h"

#include "GBFGameMode.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnGameModePlayerInitializedEventDelegate, AGameModeBase * /*GameMode*/, AController * /*NewPlayer*/);

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFGameMode : public AModularGameMode
{
    GENERATED_BODY()

public:
    FOnGameModePlayerInitializedEventDelegate& OnControllerPostLogin();
    FOnGameModePlayerInitializedEventDelegate& OnControllerLogout();

    AGBFGameMode();

    bool PlayerCanRestart_Implementation(APlayerController* player) override;
    virtual bool ControllerCanRestart(AController* controller);

    UFUNCTION(BlueprintCallable)
    void RequestPlayerRestartNextFrame(AController* controller, bool force_reset = false);

    AActor* ChoosePlayerStart_Implementation(AController* player) override;
    void Logout(AController* exiting_controller) override;
    void GenericPlayerInitialization(AController* new_player) override;

protected:
    FString InitNewPlayer(APlayerController* new_player_controller, const FUniqueNetIdRepl& unique_id, const FString& options, const FString& portal) override;
    bool ShouldSpawnAtStartSpot(AController* player) override;
    void FinishRestartPlayer(AController* new_player, const FRotator& start_rotation) override;
    bool UpdatePlayerStartSpot(AController* player, const FString& portal, FString& out_error_message) override;
    void FailedToRestartPlayer(AController* new_player) override;

private:
    FOnGameModePlayerInitializedEventDelegate OnPlayerInitializedDelegate;
    FOnGameModePlayerInitializedEventDelegate OnControllerLogoutDelegate;
};

FORCEINLINE FOnGameModePlayerInitializedEventDelegate& AGBFGameMode::OnControllerPostLogin()
{
    return OnPlayerInitializedDelegate;
}

FORCEINLINE FOnGameModePlayerInitializedEventDelegate& AGBFGameMode::OnControllerLogout()
{
    return OnControllerLogoutDelegate;
}
