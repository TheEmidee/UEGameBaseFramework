#pragma once

#include <CoreMinimal.h>
#include <GameFramework/GameSession.h>
#include <OnlineSessionClient.h>

#include "GBFGameSession.generated.h"

class FGBFOnlineSessionSettings;
class FGBFOnlineSearchSettings;

struct FGBFGameSessionParams
{
    FGBFGameSessionParams() :
        SessionName( NAME_None ),
        IsLAN( false ),
        IsPresence( false ),
        BestSessionIdx( 0 )
    {
    }

    FName SessionName;
    bool IsLAN;
    bool IsPresence;
    TSharedPtr< const FUniqueNetId > UserId;
    int32 BestSessionIdx;
};

/**
Acts as a game-specific wrapper around the session interface. The game code makes calls to this when it needs to interact with the session interface.
A game session exists only the server, while running an online game.
*/
UCLASS()
// ReSharper disable once CppClassCanBeFinal
class GAMEBASEFRAMEWORK_API AGBFGameSession : public AGameSession
{
    GENERATED_UCLASS_BODY()

public:
    DECLARE_EVENT_TwoParams( AGBFGameSession, FOnCreatePresenceSessionCompleteEvent, FName /*SessionName*/, bool /*bWasSuccessful*/ );
    DECLARE_EVENT_OneParam( AGBFGameSession, FOnFindSessionsCompleteEvent, bool /*bWasSuccessful*/ );
    DECLARE_EVENT_OneParam( AGBFGameSession, FOnJoinSessionCompleteEvent, EOnJoinSessionCompleteResult::Type /*Result*/ );

    FOnCreatePresenceSessionCompleteEvent & OnCreatePresenceSessionComplete();
    FOnJoinSessionCompleteEvent & OnJoinSessionComplete();
    FOnFindSessionsCompleteEvent & OnFindSessionsComplete();

    void HandleMatchHasStarted() override;
    void HandleMatchHasEnded() override;

    EOnlineAsyncTaskState::Type GetSearchResultStatus( int32 & search_result_idx, int32 & num_search_results ) const;
    const TArray< FOnlineSessionSearchResult > & GetSearchResults() const;

    bool JoinSession( const TSharedPtr< const FUniqueNetId > & user_id, FName session_name, int32 session_index_in_search_results );
    bool JoinSession( const TSharedPtr< const FUniqueNetId > & user_id, FName session_name, const FOnlineSessionSearchResult & search_result );
    void FindSessions( const TSharedPtr< const FUniqueNetId > & user_id, FName session_name, bool is_lan, bool is_presence );
    bool HostSession( const TSharedPtr<const FUniqueNetId> & user_id, FName session_name, const FString & game_type, const FString & map_name, bool is_lan, bool is_presence, int32 max_num_players );
    bool HostSession( const TSharedPtr<const FUniqueNetId> & user_id, const FName session_name, const FOnlineSessionSettings & session_settings );

protected:
    virtual void OnCreateSessionComplete( FName session_name, bool was_successful );
    virtual void OnStartOnlineGameComplete( FName session_name, bool was_successful );
    virtual void OnFindSessionsComplete( bool was_successful );
    virtual void OnJoinSessionComplete( FName session_name, EOnJoinSessionCompleteResult::Type result );
    virtual void OnDestroySessionComplete( FName session_name, bool was_successful );
    virtual void RegisterServer();
    virtual void FillHostSettings( FGBFOnlineSessionSettings & host_settings );

    FOnCreatePresenceSessionCompleteEvent OnCreatePresenceSessionCompleteEvent;
    FOnFindSessionsCompleteEvent OnFindSessionsCompleteEvent;
    FOnJoinSessionCompleteEvent OnJoinSessionCompleteEvent;

    FOnCreateSessionCompleteDelegate OnCreateSessionCompleteDelegate;
    FOnStartSessionCompleteDelegate OnStartSessionCompleteDelegate;
    FOnDestroySessionCompleteDelegate OnDestroySessionCompleteDelegate;
    FOnFindSessionsCompleteDelegate OnFindSessionsCompleteDelegate;
    FOnJoinSessionCompleteDelegate OnJoinSessionCompleteDelegate;

    FGBFGameSessionParams CurrentSessionParams;
    TSharedPtr< FGBFOnlineSessionSettings > HostSettings;
    TSharedPtr< FGBFOnlineSearchSettings > SearchSettings;

    FDelegateHandle OnStartSessionCompleteDelegateHandle;
    FDelegateHandle OnCreateSessionCompleteDelegateHandle;
    FDelegateHandle OnDestroySessionCompleteDelegateHandle;
    FDelegateHandle OnFindSessionsCompleteDelegateHandle;
    FDelegateHandle OnJoinSessionCompleteDelegateHandle;
};

FORCEINLINE AGBFGameSession::FOnCreatePresenceSessionCompleteEvent & AGBFGameSession::OnCreatePresenceSessionComplete()
{
    return OnCreatePresenceSessionCompleteEvent;
}

FORCEINLINE AGBFGameSession::FOnJoinSessionCompleteEvent & AGBFGameSession::OnJoinSessionComplete()
{
    return OnJoinSessionCompleteEvent;
}

FORCEINLINE AGBFGameSession::FOnFindSessionsCompleteEvent & AGBFGameSession::OnFindSessionsComplete()
{
    return OnFindSessionsCompleteEvent;
}