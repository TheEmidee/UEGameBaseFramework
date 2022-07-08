#pragma once

#include "ModularGameMode.h"

#include <CoreMinimal.h>

#include "GBFGameMode.generated.h"

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

    UFUNCTION( BlueprintCallable )
    void RequestPlayerRestartNextFrame( AController * controller, bool force_reset = false );

    AActor * ChoosePlayerStart_Implementation( AController * player ) override;

protected:
    FString InitNewPlayer( APlayerController * new_player_controller, const FUniqueNetIdRepl & unique_id, const FString & options, const FString & portal ) override;
    void FinishRestartPlayer( AController * new_player, const FRotator & start_rotation ) override;

    // :TODO: UE5
    /*bool UpdatePlayerStartSpot( AController * player, const FString & portal, FString & out_error_message ) override;
    void OnPostLogin( AController * new_player ) override;
    void FailedToRestartPlayer( AController * new_player ) override;*/

private:
    FOnGameModeCombinedPostLogin OnGameModeCombinedPostLoginDelegate;
};
