#pragma once

#include "GBFGameInstanceSubsystemBase.h"
#include "Online/GBFGameSession.h"

#include <CoreMinimal.h>
#include <Engine/Engine.h>
#include <Interfaces/OnlineIdentityInterface.h>
#include <Interfaces/OnlineSessionInterface.h>
#include <OnlineSessionSettings.h>

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

struct FGBFSessionPlayTogetherInfo
{
    FGBFSessionPlayTogetherInfo() :
        UserIndex( -1 )
    {}

    FGBFSessionPlayTogetherInfo( const int32 user_index, const TArray< TSharedPtr< const FUniqueNetId > > & user_id_list ) :
        UserIndex( user_index )
    {
        UserIdList.Append( user_id_list );
    }

    int32 UserIndex;
    TArray< TSharedPtr< const FUniqueNetId > > UserIdList;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameInstanceSessionSubsystem final : public UGBFGameInstanceSubsystemBase
{
    GENERATED_BODY()

public:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnSessionPrivilegeTaskStartedDelegate, ULocalPlayer *, local_player );
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnSessionPrivilegeTaskEndedDelegate, ULocalPlayer *, local_player );
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnSessionPrivilegeTaskFailedDelegate, ULocalPlayer *, local_player, FText, error_text );
    DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnPlayTogetherEventReceivedDelegate );

    EGBFOnlineMode GetOnlineMode() const;

    void Initialize( FSubsystemCollectionBase & collection ) override;
    AGBFGameSession * GetGameSession() const;
    void SetPendingInvite( const FGBFSessionPendingInvite & session_pending_invite );
    bool JoinSession( ULocalPlayer * local_player, int32 session_index_in_search_results );
    bool JoinSession( ULocalPlayer * local_player, const FOnlineSessionSearchResult & search_result );
    bool FindSessions( ULocalPlayer * player_owner, bool is_dedicated_server, bool is_lan_match );
    void HandlePendingSessionInvite();
    void TravelToSession( FName session_name );
    void OnPlayTogetherEventReceived( int32 user_index, const TArray< TSharedPtr< const FUniqueNetId > > & user_id_list );
    bool HostQuickSession( ULocalPlayer & local_player, const FOnlineSessionSettings & session_settings );
    bool HostGame( ULocalPlayer * local_player, const FString & game_type, const FString & travel_url );

protected:
    FString GetMapNameFromTravelURL() const;

private:
    void SetOnlineMode( EGBFOnlineMode online_mode );
    void UpdateUsingMultiPlayerFeatures( bool is_using_multi_player_features ) const;
    void HandleSessionFailure( const FUniqueNetId & unique_net_id, ESessionFailure::Type failure_type );
    void CleanupSessionOnReturnToMenu();
    void OnEndSessionComplete( FName session_name, bool was_successful );
    void BroadcastOnSessionPrivilegeTaskEnded( const FUniqueNetId & user_id ) const;
    void BroadcastOnSessionPrivilegeTaskFailed( const FUniqueNetId & user_id, EUserPrivileges::Type privilege, uint32 privilege_results ) const;

    /** Delegate function executed after checking privileges for starting quick match */
    void OnUserCanPlayInvite( const FUniqueNetId & user_id, EUserPrivileges::Type privilege, uint32 privilege_results );
    void OnUserCanPlayTogether( const FUniqueNetId & user_id, EUserPrivileges::Type privilege, uint32 privilege_results ) const;

    /** Called when there is an error trying to travel to a local session */
    void TravelLocalSessionFailure( UWorld * world, ETravelFailure::Type failure_type, const FString & reason ) const;

    void StartOnlinePrivilegeTask( const IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate & delegate, EUserPrivileges::Type privilege, TSharedPtr< const FUniqueNetId > user_id ) const;
    void AddNetworkFailureHandlers();
    void RemoveNetworkFailureHandlers() const;
    void OnJoinSessionComplete( EOnJoinSessionCompleteResult::Type result );
    void OnRegisterJoiningLocalPlayerComplete( const FUniqueNetId & player_id, EOnJoinSessionCompleteResult::Type result ) const;
    void FinishJoinSession( EOnJoinSessionCompleteResult::Type result ) const;
    void InternalTravelToSession( FName session_name ) const;
    void OnRegisterLocalPlayerComplete( const FUniqueNetId & player_id, EOnJoinSessionCompleteResult::Type result );
    void SendPlayTogetherInvites();
    void FinishSessionCreation( EOnJoinSessionCompleteResult::Type result );

    /** Callback which is intended to be called upon session creation */
    void OnCreatePresenceSessionComplete( FName session_name, bool was_successful );

    /** Callback which is intended to be called upon finding sessions */
    void OnSearchSessionsComplete( bool was_successful );

    UPROPERTY( BlueprintAssignable )
    FOnSessionPrivilegeTaskStartedDelegate OnSessionPrivilegeTaskStartedDelegate;

    UPROPERTY( BlueprintAssignable )
    FOnSessionPrivilegeTaskEndedDelegate OnSessionPrivilegeTaskEndedDelegate;

    UPROPERTY( BlueprintAssignable )
    FOnSessionPrivilegeTaskFailedDelegate OnSessionPrivilegeTaskFailedDelegate;

    UPROPERTY( BlueprintAssignable )
    FOnPlayTogetherEventReceivedDelegate OnPlayTogetherEventReceivedDelegate;

    FOnEndSessionCompleteDelegate OnEndSessionCompleteDelegate;
    EGBFOnlineMode OnlineMode;
    FDelegateHandle OnStartSessionCompleteDelegateHandle;
    FDelegateHandle OnEndSessionCompleteDelegateHandle;
    FDelegateHandle OnDestroySessionCompleteDelegateHandle;
    FDelegateHandle TravelLocalSessionFailureDelegateHandle;
    FDelegateHandle OnCreatePresenceSessionCompleteDelegateHandle;
    FDelegateHandle OnSearchSessionsCompleteDelegateHandle;
    FDelegateHandle OnJoinSessionCompleteDelegateHandle;

    FGBFSessionPendingInvite PendingInvite;
    /** URL to travel to after pending network operations */
    FString TravelURL;
    FGBFSessionPlayTogetherInfo PlayTogetherInfo;
};

FORCEINLINE EGBFOnlineMode UGBFGameInstanceSessionSubsystem::GetOnlineMode() const
{
    return OnlineMode;
}