#pragma once

#include "CommonPlayerController.h"
#include "Input/GBFInputTypes.h"

#include "GBFPlayerController.generated.h"

class UGBFLocalPlayer;
class UGASExtAbilitySystemComponent;

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFPlayerController : public ACommonPlayerController
{
    GENERATED_BODY()

public:
    AGBFPlayerController();

    void BeginPlay() override;
    void EndPlay( const EEndPlayReason::Type reason ) override;

    UFUNCTION( BlueprintPure )
    UGBFLocalPlayer * GetGBFLocalPlayer() const;

    void EnableInput( class APlayerController * player_controller ) override;
    void DisableInput( class APlayerController * player_controller ) override;

    UFUNCTION( BlueprintCallable )
    void ForceEnableInput( class APlayerController * player_controller );

    void DisableInputForDuration( const float duration );
    void OnRep_PlayerState() override;
    void InitPlayerState() override;
    void CleanupPlayerState() override;
    void SetPlayer( UPlayer * player ) override;
    void PostProcessInput( const float delta_time, const bool game_paused ) override;
    void UpdateForceFeedback( IInputInterface * input_interface, const int32 controller_id ) override;

    UFUNCTION( Reliable, Server, WithValidation )
    void ServerCheat( const FString & message );

    UFUNCTION( Reliable, Server, WithValidation )
    void ServerCheatAll( const FString & message );

    void AddCheats( bool force ) override;

    UGASExtAbilitySystemComponent * GetAbilitySystemComponent() const;

protected:
    void OnPossess( APawn * pawn ) override;
    void OnUnPossess() override;

    // Called when the player state is set or cleared
    virtual void OnPlayerStateChanged();

private:
    void BroadcastOnPlayerStateChanged();

    UPROPERTY()
    APlayerState * LastSeenPlayerState;

    FTimerHandle ReEnableInputTimerHandle;
};
