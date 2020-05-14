#pragma once

#include "GBFGameInstanceSubsystemBase.h"
#include "OnlineSessionSettings.h"

#include <CoreMinimal.h>
#include <Interfaces/OnlineSessionInterface.h>

#include "GBFGameInstanceSessionSubsystem.generated.h"

class UGBFGameInstance;

UENUM()
enum class EGBFOnlineMode : uint8
{
    Offline,
    LAN,
    Online
};

class FGBFPendingInvite
{
public:
    FGBFPendingInvite() :
        ControllerId( -1 ),
        UserId( nullptr ),
        PrivilegesCheckedAndAllowed( false )
    {}

    int32 ControllerId;
    TSharedPtr< const FUniqueNetId > UserId;
    FOnlineSessionSearchResult InviteResult;
    uint8 PrivilegesCheckedAndAllowed : 1;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameInstanceSessionSubsystem final : public UGBFGameInstanceSubsystemBase
{
    GENERATED_BODY()

public:
    void Initialize( FSubsystemCollectionBase & collection ) override;

private:

    void HandleSessionFailure( const FUniqueNetId & unique_net_id, ESessionFailure::Type failure_type );
    void OnEndSessionComplete( FName session_name, bool was_successful );

    FOnEndSessionCompleteDelegate OnEndSessionCompleteDelegate;
    EGBFOnlineMode OnlineMode;
};
