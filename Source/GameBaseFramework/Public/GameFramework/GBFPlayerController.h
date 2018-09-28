#pragma once

#include "GameFramework/PlayerController.h"

#include "Input/GBFInputTypes.h"

#include "GBFPlayerController.generated.h"

class UGBFPlatformInputSwitcherComponent;
class UGBFUIDialogManagerComponent;
class UGBFLocalPlayer;

UCLASS()

class GAMEBASEFRAMEWORK_API AGBFPlayerController : public APlayerController
{
    GENERATED_BODY()

public:

    AGBFPlayerController();

#if PLATFORM_DESKTOP
    FORCEINLINE UGBFPlatformInputSwitcherComponent * GetPlatformInputSwitcherComponent() const;
#endif

    FORCEINLINE UGBFUIDialogManagerComponent * GetUIDialogManagerComponent() const;

    void BeginPlay() override;

    UFUNCTION( BlueprintPure )
    UGBFLocalPlayer * GetGBFLocalPlayer() const;

private:

    UFUNCTION()
    void OnPlatformInputTypeUpdatedEvent( EGBFPlatformInputType input_type );

    void UpdateInputRelatedFlags();

    UPROPERTY( VisibleAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = "true" ) )
    UGBFPlatformInputSwitcherComponent * PlatformInputSwitcherComponent;

    UPROPERTY( VisibleAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = "true" ) )
    UGBFUIDialogManagerComponent * UIDialogManagerComponent;
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
