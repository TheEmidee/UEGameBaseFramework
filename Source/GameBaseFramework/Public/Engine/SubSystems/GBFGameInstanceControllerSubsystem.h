#pragma once

#include "GBFGameInstanceSubsystemBase.h"

#include <CoreMinimal.h>
#include "Interfaces/OnlineIdentityInterface.h"

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
    void HandleControllerPairingChanged( int game_user_index, FControllerPairingChangedUserInfo previous_user, FControllerPairingChangedUserInfo new_user );
    void HandleControllerConnectionChange( bool is_connection, FPlatformUserId new_uer_id, int32 old_user_id );

    int IgnorePairingChangeForControllerId;
};

FORCEINLINE void UGBFGameInstanceControllerSubsystem::SetIgnorePairingChangeForControllerId( const int ignore_pairing_change_for_controller_id )
{
    IgnorePairingChangeForControllerId = ignore_pairing_change_for_controller_id;
}