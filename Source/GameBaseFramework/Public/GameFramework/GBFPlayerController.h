#pragma once

#include "CommonPlayerController.h"
#include "Input/GBFInputTypes.h"

#include "GBFPlayerController.generated.h"

class UGBFLocalPlayer;
class UGBFAbilitySystemComponent;

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFPlayerController : public ACommonPlayerController
{
    GENERATED_BODY()

public:
    AGBFPlayerController();

    UFUNCTION( BlueprintPure )
    UGBFLocalPlayer * GetGBFLocalPlayer() const;

    UFUNCTION( BlueprintPure )
    AActor * GetStartSpot() const;

    void EnableInput( APlayerController * player_controller ) override;
    void DisableInput( APlayerController * player_controller ) override;

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

    UGBFAbilitySystemComponent * GetAbilitySystemComponent() const;

protected:
    void OnPossess( APawn * pawn ) override;

    // Called when the player state is set or cleared
    virtual void OnPlayerStateChanged();

private:
    void BroadcastOnPlayerStateChanged();

    UPROPERTY()
    APlayerState * LastSeenPlayerState;

    FTimerHandle ReEnableInputTimerHandle;
};

FORCEINLINE AActor * AGBFPlayerController::GetStartSpot() const
{
    return StartSpot.Get();
}
