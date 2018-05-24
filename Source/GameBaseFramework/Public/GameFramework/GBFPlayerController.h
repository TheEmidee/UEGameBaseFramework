#pragma once

#include "GameFramework/PlayerController.h"

#include "GBFPlayerController.generated.h"

class UGBFPlatformInputSwitcherComponent;

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFPlayerController : public APlayerController
{
    GENERATED_BODY()

public:

    AGBFPlayerController();

private:

    UPROPERTY( VisibleAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = "true" ) )
    UGBFPlatformInputSwitcherComponent * PlatformInputSwitcherComponent;
};