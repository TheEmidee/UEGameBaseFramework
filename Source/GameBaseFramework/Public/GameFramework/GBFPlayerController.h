#pragma once

#include "GameFramework/PlayerController.h"
#include "Input/GBFInputTypes.h"
#include "ModularPlayerController.h"

#include "GBFPlayerController.generated.h"

class UGBFPlatformInputSwitcherComponent;
class UGBFUIDialogManagerComponent;
class UGBFLocalPlayer;

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

private:
    UFUNCTION()
    void OnPlatformInputTypeUpdatedEvent( EGBFPlatformInputType input_type );

    void UpdateInputRelatedFlags();

    UPROPERTY( VisibleAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = "true" ) )
    UGBFPlatformInputSwitcherComponent * PlatformInputSwitcherComponent;

    UPROPERTY( VisibleAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = "true" ) )
    UGBFUIDialogManagerComponent * UIDialogManagerComponent;

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
