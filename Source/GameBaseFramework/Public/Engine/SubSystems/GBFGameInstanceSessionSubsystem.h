#pragma once

#include "GBFGameInstanceSubsystemBase.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineIdentityInterface.h"
#include "Online/GBFGameSession.h"



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

class FGBFSessionPendingInvite
{
public:
    FGBFSessionPendingInvite() :
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
    DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnSessionPrivilegeTaskStartedDelegate )
    DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnSessionPrivilegeTaskEndedDelegate )
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnSessionPrivilegeTaskFailedDelegate, ULocalPlayer *, local_player, FText, error_text )

    void Initialize( FSubsystemCollectionBase & collection ) override;
    AGBFGameSession * GetGameSession() const;

private:

    void HandleSessionFailure( const FUniqueNetId & unique_net_id, ESessionFailure::Type failure_type );
    void CleanupSessionOnReturnToMenu();
    void OnEndSessionComplete( FName session_name, bool was_successful );
    void BroadcastOnSessionPrivilegeTaskEnded() const;
    void BroadcastOnSessionPrivilegeTaskFailed( const FUniqueNetId & user_id, EUserPrivileges::Type privilege, uint32 privilege_results );

    /** Delegate function executed after checking privileges for starting quick match */
    void OnUserCanPlayInvite( const FUniqueNetId& user_id, EUserPrivileges::Type privilege, uint32 privilege_results );

    UPROPERTY( BlueprintAssignable )
    FOnSessionPrivilegeTaskStartedDelegate OnSessionPrivilegeTaskStartedDelegate;

    UPROPERTY( BlueprintAssignable )
    FOnSessionPrivilegeTaskEndedDelegate OnSessionPrivilegeTaskEndedDelegate;

    UPROPERTY( BlueprintAssignable )
    FOnSessionPrivilegeTaskFailedDelegate OnSessionPrivilegeTaskFailedDelegate; 

    FOnEndSessionCompleteDelegate OnEndSessionCompleteDelegate;
    EGBFOnlineMode OnlineMode;
    FDelegateHandle OnStartSessionCompleteDelegateHandle;
    FDelegateHandle OnEndSessionCompleteDelegateHandle;
    FDelegateHandle OnDestroySessionCompleteDelegateHandle;

    FGBFSessionPendingInvite PendingInvite;
};
