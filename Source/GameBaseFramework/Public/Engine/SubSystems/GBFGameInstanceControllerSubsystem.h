#pragma once

#include "GBFGameInstanceSubsystemBase.h"

#include <CoreMinimal.h>

#include "GBFGameInstanceControllerSubsystem.generated.h"

class FUniqueNetId;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameInstanceControllerSubsystem final : public UGBFGameInstanceSubsystemBase
{
    GENERATED_BODY()

public:
    UGBFGameInstanceControllerSubsystem();

    void SetIgnorePairingChangeForControllerId( int ignore_pairing_change_for_controller_id );

    void Initialize( FSubsystemCollectionBase & collection ) override;

private:
    int IgnorePairingChangeForControllerId;
};

FORCEINLINE void UGBFGameInstanceControllerSubsystem::SetIgnorePairingChangeForControllerId( const int ignore_pairing_change_for_controller_id )
{
    IgnorePairingChangeForControllerId = ignore_pairing_change_for_controller_id;
}