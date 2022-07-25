#pragma once

#include "ModularGameMode.h"

#include <CoreMinimal.h>

#include "GBFGameMode.generated.h"

class UGBFExperienceDefinition;
DECLARE_MULTICAST_DELEGATE_TwoParams( FOnGameModeCombinedPostLogin, AGameModeBase * /*GameMode*/, AController * /*NewPlayer*/ );

class UGBFPawnData;
UCLASS()
class GAMEBASEFRAMEWORK_API AGBFGameMode : public AModularGameMode
{
    GENERATED_BODY()

public:
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

protected:
    void HandleMatchHasStarted() override;
    FString InitNewPlayer( APlayerController * new_player_controller, const FUniqueNetIdRepl & unique_id, const FString & options, const FString & portal ) override;
    bool ShouldSpawnAtStartSpot( AController * player ) override;
    void FinishRestartPlayer( AController * new_player, const FRotator & start_rotation ) override;
    // :TODO: UE5
    //bool UpdatePlayerStartSpot( AController * player, const FString & portal, FString & out_error_message ) override;
    // void FailedToRestartPlayer( AController * new_player ) override;
    // :TODO: UE5 - Rename to OnPostLogin
    void PostLogin( APlayerController * new_player ) override;

private:
    void HandleMatchAssignmentIfNotExpectingOne();
    void OnExperienceDefined( FPrimaryAssetId experience_id, const FString & experience_id_source );
    void OnExperienceLoaded( const UGBFExperienceDefinition * current_experience );
    bool IsExperienceLoaded() const;

    FOnGameModeCombinedPostLogin OnGameModeCombinedPostLoginDelegate;
};
