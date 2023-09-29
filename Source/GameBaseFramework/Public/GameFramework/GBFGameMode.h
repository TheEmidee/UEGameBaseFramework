#pragma once

#include "ModularGameMode.h"

#include <CoreMinimal.h>

#include "GBFGameMode.generated.h"

class UGBFExperienceImplementation;
class UGBFPawnData;
enum class ECommonSessionOnlineMode : uint8;

DECLARE_MULTICAST_DELEGATE_TwoParams( FOnGameModePlayerInitializedEventDelegate, AGameModeBase * /*GameMode*/, AController * /*NewPlayer*/ );

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFGameMode : public AModularGameMode
{
    GENERATED_BODY()

public:
    FOnGameModePlayerInitializedEventDelegate & OnControllerPostLogin();
    FOnGameModePlayerInitializedEventDelegate & OnControllerLogout();

    AGBFGameMode();

    const UGBFPawnData * GetPawnDataForController( const AController * controller ) const;
    APawn * SpawnDefaultPawnAtTransform_Implementation( AController * new_player, const FTransform & spawn_transform ) override;
    UClass * GetDefaultPawnClassForController_Implementation( AController * controller ) override;
    bool PlayerCanRestart_Implementation( APlayerController * player ) override;
    virtual bool ControllerCanRestart( AController * controller );
    void InitGame( const FString & map_name, const FString & options, FString & error_message ) override;
    void InitGameState() override;

    UFUNCTION( BlueprintCallable )
    void RequestPlayerRestartNextFrame( AController * controller, bool force_reset = false );

    AActor * ChoosePlayerStart_Implementation( AController * player ) override;
    void HandleStartingNewPlayer_Implementation( APlayerController * new_player ) override;
    void Logout( AController * exiting_controller ) override;
    bool ReadyToStartMatch_Implementation() override;

    virtual bool TryDedicatedServerLogin();

protected:
    void HandleMatchHasStarted() override;
    FString InitNewPlayer( APlayerController * new_player_controller, const FUniqueNetIdRepl & unique_id, const FString & options, const FString & portal ) override;
    bool ShouldSpawnAtStartSpot( AController * player ) override;
    void FinishRestartPlayer( AController * new_player, const FRotator & start_rotation ) override;
    bool UpdatePlayerStartSpot( AController * player, const FString & portal, FString & out_error_message ) override;
    void FailedToRestartPlayer( AController * new_player ) override;
    void GenericPlayerInitialization( AController * new_player ) override;
    virtual void HostDedicatedServerMatch( ECommonSessionOnlineMode online_mode );

private:
    void HandleMatchAssignmentIfNotExpectingOne();
    void OnExperienceDefined( FPrimaryAssetId experience_id, const FString & experience_id_source );
    void OnExperienceLoaded( const UGBFExperienceImplementation * current_experience );
    bool IsExperienceLoaded() const;

    UFUNCTION()
    void OnUserInitializedForDedicatedServer( const UCommonUserInfo * user_info, bool is_successful, FText error, ECommonUserPrivilege requested_privilege, ECommonUserOnlineContext online_context );

    FOnGameModePlayerInitializedEventDelegate OnPlayerInitializedDelegate;
    FOnGameModePlayerInitializedEventDelegate OnControllerLogoutDelegate;
};

FORCEINLINE FOnGameModePlayerInitializedEventDelegate & AGBFGameMode::OnControllerPostLogin()
{
    return OnPlayerInitializedDelegate;
}

FORCEINLINE FOnGameModePlayerInitializedEventDelegate & AGBFGameMode::OnControllerLogout()
{
    return OnControllerLogoutDelegate;
}