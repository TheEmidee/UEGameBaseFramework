#pragma once

#include "Input/GBFInputTypes.h"
#include "ModularPlayerController.h"

#include "GBFPlayerController.generated.h"

class UGBFPlatformInputSwitcherComponent;
class UGBFUIDialogManagerComponent;
class UGBFLocalPlayer;
class UGASExtAbilitySystemComponent;

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFPlayerController : public AModularPlayerController
{
    GENERATED_BODY()

public:
    AGBFPlayerController();

#if PLATFORM_DESKTOP
    FORCEINLINE UGBFPlatformInputSwitcherComponent * GetPlatformInputSwitcherComponent() const;
#endif

    FORCEINLINE UGBFUIDialogManagerComponent * GetUIDialogManagerComponent() const;

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
    void PostProcessInput( const float DeltaTime, const bool bGamePaused ) override;

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
    UFUNCTION()
    void OnPlatformInputTypeUpdatedEvent( EGBFPlatformInputType input_type );

    void UpdateInputRelatedFlags();
    void BroadcastOnPlayerStateChanged();

    UPROPERTY( VisibleAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = "true" ) )
    UGBFPlatformInputSwitcherComponent * PlatformInputSwitcherComponent;

    UPROPERTY( VisibleAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = "true" ) )
    UGBFUIDialogManagerComponent * UIDialogManagerComponent;

    UPROPERTY()
    APlayerState * LastSeenPlayerState;

    FTimerHandle ReEnableInputTimerHandle;
};

#if PLATFORM_DESKTOP
UGBFPlatformInputSwitcherComponent * AGBFPlayerController::GetPlatformInputSwitcherComponent() const
{
    return PlatformInputSwitcherComponent;
}
#endif

UGBFUIDialogManagerComponent * AGBFPlayerController::GetUIDialogManagerComponent() const
{
    return UIDialogManagerComponent;
}
