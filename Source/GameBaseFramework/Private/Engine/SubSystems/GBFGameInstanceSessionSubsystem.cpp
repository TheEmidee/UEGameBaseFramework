#include "Engine/SubSystems/GBFGameInstanceSessionSubsystem.h"

#include "Engine/GBFGameInstance.h"
#include "Engine/SubSystems/GBFGameInstanceIdentitySubsystem.h"
#include "GameBaseFrameworkSettings.h"

#include <Engine/LocalPlayer.h>
#include <GameFramework/GameModeBase.h>
#include <GameFramework/PlayerController.h>
#include <Kismet/GameplayStatics.h>
#include <OnlineSubsystemUtils.h>

void UGBFGameInstanceSessionSubsystem::Initialize( FSubsystemCollectionBase & collection )
{
    Super::Initialize( collection );

    auto * oss = Online::GetSubsystem( GetWorld() );
    check( oss != nullptr );

    const auto session_interface = oss->GetSessionInterface();
    check( session_interface.IsValid() );

    session_interface->AddOnSessionFailureDelegate_Handle( FOnSessionFailureDelegate::CreateUObject( this, &UGBFGameInstanceSessionSubsystem::HandleSessionFailure ) );
    OnEndSessionCompleteDelegate = FOnEndSessionCompleteDelegate::CreateUObject( this, &UGBFGameInstanceSessionSubsystem::OnEndSessionComplete );

    OnlineMode = EGBFOnlineMode::Offline;
}

AGBFGameSession * UGBFGameInstanceSessionSubsystem::GetGameSession() const
{
    if ( const auto * world = GetWorld() )
    {
        if ( const auto * game_mode = world->GetAuthGameMode() )
        {
            return Cast< AGBFGameSession >( game_mode->GameSession );
        }
    }

    return nullptr;
}

void UGBFGameInstanceSessionSubsystem::SetPendingInvite( const FGBFSessionPendingInvite & session_pending_invite )
{
    PendingInvite = session_pending_invite;

    //GetSubsystem< UGBFGameInstanceGameStateSystem >()->GoToMainMenuState();

    const auto delegate = IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate::CreateUObject( this, &UGBFGameInstanceSessionSubsystem::OnUserCanPlayInvite );
    StartOnlinePrivilegeTask( delegate, EUserPrivileges::CanPlayOnline, PendingInvite.UserId );
}

bool UGBFGameInstanceSessionSubsystem::JoinSession( const ULocalPlayer * local_player, const int32 session_index_in_search_results )
{
    UE_LOG( LogOnlineGame, Display, TEXT( __FUNCTION__ ) )

    if ( auto * game_session = GetGameSession() )
    {
        AddNetworkFailureHandlers();

        OnJoinSessionCompleteDelegateHandle = game_session->OnJoinSessionComplete().AddUObject( this, &UGBFGameInstanceSessionSubsystem::OnJoinSessionComplete );

        if ( game_session->JoinSession( local_player->GetPreferredUniqueNetId().GetUniqueNetId(), NAME_GameSession, session_index_in_search_results ) )
        {
            // :TODO: Keep? If any error occured in the above, pending state would be set
            // if ( ( PendingState == CurrentState ) || ( PendingState == ShooterGameInstanceState::None ) )
            {
                // Go ahead and go into loading state now
                // If we fail, the delegate will handle showing the proper messaging and move to the correct state
                // ShowLoadingScreen();
                //GetSubsystem< UGBFGameInstanceGameStateSystem >()->GoToInGameState();
                UE_LOG( LogOnlineGame, Display, TEXT( "%s - Result %d" ), TEXT( __FUNCTION__ ), true );
                return true;
            }
        }
    }

    UE_LOG( LogOnlineGame, Warning, TEXT( "%s - Result %d" ), TEXT( __FUNCTION__ ), false );
    return false;
}

bool UGBFGameInstanceSessionSubsystem::JoinSession( const ULocalPlayer * local_player, const FOnlineSessionSearchResult & search_result )
{
    UE_LOG( LogOnlineGame, Display, TEXT( __FUNCTION__ ) )

    if ( auto * game_session = GetGameSession() )
    {
        AddNetworkFailureHandlers();

        OnJoinSessionCompleteDelegateHandle = game_session->OnJoinSessionComplete().AddUObject( this, &UGBFGameInstanceSessionSubsystem::OnJoinSessionComplete );
        if ( game_session->JoinSession( local_player->GetPreferredUniqueNetId().GetUniqueNetId(), NAME_GameSession, search_result ) )
        {
            // :TODO: Keep? If any error occured in the above, pending state would be set
            // if ( ( PendingState == CurrentState ) || ( PendingState == ShooterGameInstanceState::None ) )
            {
                // Go ahead and go into loading state now
                // If we fail, the delegate will handle showing the proper messaging and move to the correct state
                // ShowLoadingScreen();
                //GetSubsystem< UGBFGameInstanceGameStateSystem >()->GoToInGameState();
                UE_LOG( LogOnlineGame, Display, TEXT( "%s - Result %d" ), TEXT( __FUNCTION__ ), true );
                return true;
            }
        }
    }

    UE_LOG( LogOnlineGame, Warning, TEXT( "%s - Result %d" ), TEXT( __FUNCTION__ ), false );
    return false;
}

bool UGBFGameInstanceSessionSubsystem::FindSessions( ULocalPlayer * player_owner, const bool is_dedicated_server, const bool is_lan_match )
{
    UE_LOG( LogOnlineGame, Display, TEXT( __FUNCTION__ ) )

    auto result = false;

    check( player_owner != nullptr );
    if ( player_owner )
    {
        if ( auto * game_session = GetGameSession() )
        {
            game_session->OnFindSessionsComplete().RemoveAll( this );
            OnSearchSessionsCompleteDelegateHandle = game_session->OnFindSessionsComplete().AddUObject( this, &UGBFGameInstanceSessionSubsystem::OnSearchSessionsComplete );

            game_session->FindSessions( player_owner->GetPreferredUniqueNetId().GetUniqueNetId(), NAME_GameSession, is_lan_match, !is_dedicated_server );

            result = true;
        }
    }

    return result;
}

void UGBFGameInstanceSessionSubsystem::HandlePendingSessionInvite()
{
    // :NOTE: In shooter game they also check the game state is PendingInvite. We don't have that state. Maybe we are missing a check here
    if ( !PendingInvite.UserId.IsValid() || !PendingInvite.PrivilegesCheckedAndAllowed )
    {
        return;
    }

    auto * oss = Online::GetSubsystem( GetWorld() );
    const auto sessions_interface_ptr = ( oss != nullptr ) ? oss->GetSessionInterface() : nullptr;

    if ( !sessions_interface_ptr.IsValid() )
    {
        return;
    }

    const auto session_state = sessions_interface_ptr->GetSessionState( NAME_GameSession );

    if ( session_state != EOnlineSessionState::NoSession )
    {
        return;
    }

    auto * new_player_owner = GetOuterUGameInstance()->GetFirstGamePlayer();

    if ( new_player_owner != nullptr )
    {
        new_player_owner->SetControllerId( PendingInvite.ControllerId );
        new_player_owner->SetCachedUniqueNetId( PendingInvite.UserId );
        SetOnlineMode( EGBFOnlineMode::Online );

        const auto is_local_player_host = PendingInvite.UserId.IsValid() && PendingInvite.InviteResult.Session.OwningUserId.IsValid() && *PendingInvite.UserId == *PendingInvite.InviteResult.Session.OwningUserId;
        if ( is_local_player_host )
        {
            HostQuickSession( *new_player_owner, PendingInvite.InviteResult.Session.SessionSettings );
        }
        else
        {
            JoinSession( new_player_owner, PendingInvite.InviteResult );
        }
    }

    PendingInvite.UserId.Reset();
}

void UGBFGameInstanceSessionSubsystem::TravelToSession( const FName session_name )
{
    AddNetworkFailureHandlers();
    //GetSubsystem< UGBFGameInstanceGameStateSystem >()->GoToInGameState();
    InternalTravelToSession( session_name );
}

bool UGBFGameInstanceSessionSubsystem::HostQuickSession( const ULocalPlayer & local_player, const FOnlineSessionSettings & session_settings )
{
    // This function is different from BeginHostingQuickMatch in that it creates a session and then starts a quick match,
    // while BeginHostingQuickMatch assumes a session already exists

    if ( auto * game_session = GetGameSession() )
    {
        // Add callback delegate for completion
        OnCreatePresenceSessionCompleteDelegateHandle = game_session->OnCreatePresenceSessionComplete().AddUObject( this, &UGBFGameInstanceSessionSubsystem::OnCreatePresenceSessionComplete );

        // :TODO:
        TravelURL = ""; // GetQuickMatchUrl();

        auto host_settings = session_settings;

        const auto game_type = UGameplayStatics::ParseOption( TravelURL, TEXT( "game" ) );
        const auto & map_name = GetMapNameFromTravelURL();

        host_settings.Set( SETTING_GAMEMODE, game_type, EOnlineDataAdvertisementType::ViaOnlineService );
        host_settings.Set( SETTING_MAPNAME, map_name, EOnlineDataAdvertisementType::ViaOnlineService );
        host_settings.NumPublicConnections = 16;

        if ( game_session->HostSession( local_player.GetPreferredUniqueNetId().GetUniqueNetId(), NAME_GameSession, session_settings ) )
        {
            // :TODO: Keep? If any error occurred in the above, pending state would be set
            // if ( PendingState == CurrentState || PendingState == ShooterGameInstanceState::None )
            // {
            //     // Go ahead and go into loading state now
            //     // If we fail, the delegate will handle showing the proper messaging and move to the correct state
            //     ShowLoadingScreen();
            //GetSubsystem< UGBFGameInstanceGameStateSystem >()->GoToInGameState();
            return true;
            //}
        }
    }

    return false;
}

bool UGBFGameInstanceSessionSubsystem::HostGame( const ULocalPlayer * local_player, const FString & game_type, const FString & travel_url )
{
    UE_LOG( LogOnlineGame, Display, TEXT( __FUNCTION__ ) );

    //auto * game_state_subsystem = GetSubsystem< UGBFGameInstanceGameStateSystem >();

    if ( GetOnlineMode() == EGBFOnlineMode::Offline )
    {
        //
        // Offline game, just go straight to map
        //

        // ShowLoadingScreen();
        //game_state_subsystem->GoToInGameState();

        // Travel to the specified match URL
        TravelURL = travel_url;
        GetWorld()->ServerTravel( TravelURL );
        return true;
    }

    //
    // Online game
    //

    if ( auto * game_session = GetGameSession() )
    {
        // add callback delegate for completion
        OnCreatePresenceSessionCompleteDelegateHandle = game_session->OnCreatePresenceSessionComplete().AddUObject( this, &UGBFGameInstanceSessionSubsystem::OnCreatePresenceSessionComplete );

        TravelURL = travel_url;
        const auto is_lan_match = travel_url.Contains( TEXT( "?bIsLanMatch" ) );
        const auto & map_name = GetMapNameFromTravelURL();

        // :TODO: Allow to define the max number of players dynamically
        if ( game_session->HostSession( local_player->GetPreferredUniqueNetId().GetUniqueNetId(), NAME_GameSession, game_type, map_name, is_lan_match, true, 4 ) )
        {
            // :TODO: Keep? If any error occurred in the above, pending state would be set
            // if ( ( PendingState == CurrentState ) || ( PendingState == ShooterGameInstanceState::None ) )
            // {
            //     // Go ahead and go into loading state now
            //     // If we fail, the delegate will handle showing the proper messaging and move to the correct state
            //     ShowLoadingScreen();
            //game_state_subsystem->GoToInGameState();
            return true;
            //}
        }
    }

    return false;
}

FString UGBFGameInstanceSessionSubsystem::GetMapNameFromTravelURL() const
{
    const FString & map_name_sub_str = "/Game/Maps/";
    const auto & chopped_map_name = TravelURL.RightChop( map_name_sub_str.Len() );
    const auto & map_name = chopped_map_name.LeftChop( chopped_map_name.Len() - chopped_map_name.Find( "?game" ) );

    return map_name;
}

void UGBFGameInstanceSessionSubsystem::SetOnlineMode( const EGBFOnlineMode online_mode )
{
    OnlineMode = online_mode;
    UpdateUsingMultiPlayerFeatures( online_mode == EGBFOnlineMode::Online );
}

void UGBFGameInstanceSessionSubsystem::UpdateUsingMultiPlayerFeatures( const bool is_using_multi_player_features ) const
{
    if ( auto * oss = Online::GetSubsystem( GetWorld() ) )
    {
        for ( auto * local_player : GetOuterUGameInstance()->GetLocalPlayers() )
        {
            auto player_unique_id = local_player->GetPreferredUniqueNetId();

            if ( player_unique_id.IsValid() )
            {
                oss->SetUsingMultiplayerFeatures( *player_unique_id, is_using_multi_player_features );
            }
        }
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UGBFGameInstanceSessionSubsystem::HandleSessionFailure( const FUniqueNetId & unique_net_id, const ESessionFailure::Type failure_type )
{
    UE_LOG( LogOnlineGame, Warning, TEXT( "UGBFGameInstanceSessionSubsystem::HandleSessionFailure: %u" ), ( uint32 ) failure_type );

#if 1 // GBF_CONSOLE_UI
    // If we are not currently at (or heading to) the welcome screen then display a message on consoles
    if ( OnlineMode != EGBFOnlineMode::Offline /*&& !GetSubsystem< UGBFGameInstanceGameStateSystem >()->IsOnWelcomeScreenState()*/ )
    {
        UE_LOG( LogOnlineGame, Log, TEXT( "UGBFGameInstanceSessionSubsystem::HandleSessionFailure: Going to main menu" ) );

        // Display message on consoles
#if PLATFORM_XBOXONE
        const auto return_reason = NSLOCTEXT( "NetworkFailures", "ServiceUnavailable", "Connection to Xbox LIVE has been lost." );
#elif PLATFORM_PS4
        const auto return_reason = NSLOCTEXT( "NetworkFailures", "ServiceUnavailable", "Connection to PSN has been lost." );
#else
        const auto return_reason = NSLOCTEXT( "NetworkFailures", "ServiceUnavailable", "Connection has been lost." );
#endif
        const auto ok_button = NSLOCTEXT( "DialogButtons", "OKAY", "OK" );

        auto * settings = GetDefault< UGameBaseFrameworkSettings >();

        /*GetGBFGameInstance()->ShowMessageThenGotoState(
            NSLOCTEXT( "GBF", "LocKey_NetworkFailure", "NetworkFailures" ),
            return_reason,
            UGBFGameState::MainMenuStateName );*/
    }
#endif
}

void UGBFGameInstanceSessionSubsystem::CleanupSessionOnReturnToMenu()
{
    auto pending_online_operation = false;

    // end online game and then destroy it
    auto * online_sub = Online::GetSubsystem( GetWorld() );
    const auto sessions = ( online_sub != nullptr ) ? online_sub->GetSessionInterface() : nullptr;

    if ( sessions.IsValid() )
    {
        const FName game_session( NAME_GameSession );
        const auto session_state = sessions->GetSessionState( NAME_GameSession );

        UE_LOG( LogOnline, Log, TEXT( "Session %s is '%s'" ), *game_session.ToString(), EOnlineSessionState::ToString( session_state ) );

        if ( EOnlineSessionState::InProgress == session_state )
        {
            UE_LOG( LogOnline, Log, TEXT( "Ending session %s on return to main menu" ), *game_session.ToString() );
            OnEndSessionCompleteDelegateHandle = sessions->AddOnEndSessionCompleteDelegate_Handle( OnEndSessionCompleteDelegate );
            sessions->EndSession( NAME_GameSession );
            pending_online_operation = true;
        }
        else if ( EOnlineSessionState::Ending == session_state )
        {
            UE_LOG( LogOnline, Log, TEXT( "Waiting for session %s to end on return to main menu" ), *game_session.ToString() );
            OnEndSessionCompleteDelegateHandle = sessions->AddOnEndSessionCompleteDelegate_Handle( OnEndSessionCompleteDelegate );
            pending_online_operation = true;
        }
        else if ( EOnlineSessionState::Ended == session_state || EOnlineSessionState::Pending == session_state )
        {
            UE_LOG( LogOnline, Log, TEXT( "Destroying session %s on return to main menu" ), *game_session.ToString() );
            OnDestroySessionCompleteDelegateHandle = sessions->AddOnDestroySessionCompleteDelegate_Handle( OnEndSessionCompleteDelegate );
            sessions->DestroySession( NAME_GameSession );
            pending_online_operation = true;
        }
        else if ( EOnlineSessionState::Starting == session_state || EOnlineSessionState::Creating == session_state )
        {
            UE_LOG( LogOnline, Log, TEXT( "Waiting for session %s to start, and then we will end it to return to main menu" ), *game_session.ToString() );
            OnStartSessionCompleteDelegateHandle = sessions->AddOnStartSessionCompleteDelegate_Handle( OnEndSessionCompleteDelegate );
            pending_online_operation = true;
        }
    }

    if ( !pending_online_operation )
    {
        // :TODO:
        // GEngine->HandleDisconnect( GetWorld(), GetWorld()->GetNetDriver() );
    }
}

void UGBFGameInstanceSessionSubsystem::OnEndSessionComplete( const FName session_name, const bool was_successful )
{
    UE_LOG( LogOnline, Log, TEXT( "UShooterGameInstance::OnEndSessionComplete: Session=%s bWasSuccessful=%s" ), *session_name.ToString(), was_successful ? TEXT( "true" ) : TEXT( "false" ) );

    if ( auto * online_sub = Online::GetSubsystem( GetWorld() ) )
    {
        auto sessions = online_sub->GetSessionInterface();
        if ( sessions.IsValid() )
        {
            sessions->ClearOnStartSessionCompleteDelegate_Handle( OnStartSessionCompleteDelegateHandle );
            sessions->ClearOnEndSessionCompleteDelegate_Handle( OnEndSessionCompleteDelegateHandle );
            sessions->ClearOnDestroySessionCompleteDelegate_Handle( OnDestroySessionCompleteDelegateHandle );
        }
    }

    CleanupSessionOnReturnToMenu();
}

void UGBFGameInstanceSessionSubsystem::BroadcastOnSessionPrivilegeTaskEnded( const FUniqueNetId & user_id ) const
{
    const auto owning_player = GetSubsystem< UGBFGameInstanceIdentitySubsystem >()->GetLocalPlayerFromUniqueNetId( user_id );
    OnSessionPrivilegeTaskEndedDelegate.Broadcast( owning_player );
}

void UGBFGameInstanceSessionSubsystem::BroadcastOnSessionPrivilegeTaskFailed( const FUniqueNetId & user_id, EUserPrivileges::Type /* privilege */, const uint32 privilege_results ) const
{
    if ( GEngine == nullptr )
    {
        return;
    }

    const auto owning_player = GetSubsystem< UGBFGameInstanceIdentitySubsystem >()->GetLocalPlayerFromUniqueNetId( user_id );

    if ( owning_player == nullptr )
    {
        return;
    }

    if ( ( privilege_results & static_cast< uint32 >( IOnlineIdentity::EPrivilegeResults::AccountTypeFailure ) ) != 0 )
    {
        auto external_ui = Online::GetExternalUIInterface( GetWorld() );
        if ( external_ui.IsValid() )
        {
            external_ui->ShowAccountUpgradeUI( user_id );
        }
        return;
    }

    FText error_text;

    if ( ( privilege_results & static_cast< uint32 >( IOnlineIdentity::EPrivilegeResults::RequiredSystemUpdate ) ) != 0 )
    {
        error_text = NSLOCTEXT( "OnlinePrivilegeResult", "RequiredSystemUpdate", "A required system update is available.  Please upgrade to access online features." );
    }
    else if ( ( privilege_results & static_cast< uint32 >( IOnlineIdentity::EPrivilegeResults::RequiredPatchAvailable ) ) != 0 )
    {
        error_text = NSLOCTEXT( "OnlinePrivilegeResult", "RequiredPatchAvailable", "A required game patch is available.  Please upgrade to access online features." );
    }
    else if ( ( privilege_results & static_cast< uint32 >( IOnlineIdentity::EPrivilegeResults::AgeRestrictionFailure ) ) != 0 )
    {
        error_text = NSLOCTEXT( "OnlinePrivilegeResult", "AgeRestrictionFailure", "Cannot play due to age restrictions!" );
    }
    else if ( ( privilege_results & static_cast< uint32 >( IOnlineIdentity::EPrivilegeResults::UserNotFound ) ) != 0 )
    {
        error_text = NSLOCTEXT( "OnlinePrivilegeResult", "UserNotFound", "Cannot play due invalid user!" );
    }
    else if ( ( privilege_results & static_cast< uint32 >( IOnlineIdentity::EPrivilegeResults::GenericFailure ) ) != 0 )
    {
        error_text = NSLOCTEXT( "OnlinePrivilegeResult", "GenericFailure", "Cannot play online.  Check your network connection." );
    }

    if ( !error_text.IsEmpty() )
    {
        OnSessionPrivilegeTaskFailedDelegate.Broadcast( owning_player, error_text );
    }
}

void UGBFGameInstanceSessionSubsystem::OnUserCanPlayInvite( const FUniqueNetId & user_id, const EUserPrivileges::Type privilege, const uint32 privilege_results )
{
    BroadcastOnSessionPrivilegeTaskEnded( user_id );

    if ( privilege_results == static_cast< uint32 >( IOnlineIdentity::EPrivilegeResults::NoFailures ) )
    {
        if ( user_id == *PendingInvite.UserId )
        {
            PendingInvite.PrivilegesCheckedAndAllowed = true;
        }
    }
    else
    {
        BroadcastOnSessionPrivilegeTaskFailed( user_id, privilege, privilege_results );
        //GetSubsystem< UGBFGameInstanceGameStateSystem >()->GoToWelcomeScreenState();
    }
}

void UGBFGameInstanceSessionSubsystem::OnUserCanPlayTogether( const FUniqueNetId & user_id, const EUserPrivileges::Type privilege, const uint32 privilege_results ) const
{
    BroadcastOnSessionPrivilegeTaskEnded( user_id );

    //auto * game_state_system = GetSubsystem< UGBFGameInstanceGameStateSystem >();

    if ( privilege_results == static_cast< uint32 >( IOnlineIdentity::EPrivilegeResults::NoFailures ) )
    {
        //if ( game_state_system->IsOnWelcomeScreenState() )
        //{
        //    // :NOTE: needs testing
        //    if ( auto * new_player_owner = GetOuterUGameInstance()->GetFirstGamePlayer() )
        //    {
        //        if ( PlayTogetherInfo.UserIndex != -1 )
        //        {
        //            new_player_owner->SetControllerId( PlayTogetherInfo.UserIndex );
        //            new_player_owner->SetCachedUniqueNetId( new_player_owner->GetUniqueNetIdFromCachedControllerId().GetUniqueNetId() );

        //            game_state_system->GoToMainMenuState();
        //        }
        //    }
        //}
    }
    else
    {
        BroadcastOnSessionPrivilegeTaskFailed( user_id, privilege, privilege_results );
        //game_state_system->GoToWelcomeScreenState();
    }
}

void UGBFGameInstanceSessionSubsystem::TravelLocalSessionFailure( UWorld * world, ETravelFailure::Type failure_type, const FString & reason ) const
{
    auto return_reason = NSLOCTEXT( "NetworkErrors", "JoinSessionFailed", "Join Session failed." );
    if ( !reason.IsEmpty() )
    {
        return_reason = FText::Format( NSLOCTEXT( "NetworkErrors", "JoinSessionFailedReasonFmt", "Join Session failed. {0}" ), FText::FromString( reason ) );
    }

    const auto title = NSLOCTEXT( "GBF", "LocKey_NeedLicenseTitle", "NetworkFailures" );
    GetGBFGameInstance()->ShowMessageThenGotoMainMenuState( title, return_reason );
}

void UGBFGameInstanceSessionSubsystem::StartOnlinePrivilegeTask( const IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate & delegate, const EUserPrivileges::Type privilege, const TSharedPtr< const FUniqueNetId > user_id ) const
{
    const auto owning_player = GetSubsystem< UGBFGameInstanceIdentitySubsystem >()->GetLocalPlayerFromUniqueNetId( *user_id.Get() );
    OnSessionPrivilegeTaskStartedDelegate.Broadcast( owning_player );

    auto identity = Online::GetIdentityInterface( GetWorld() );

    if ( identity.IsValid() && user_id.IsValid() )
    {
        identity->GetUserPrivilege( *user_id, privilege, delegate );
    }
    else
    {
        // Can only get away with faking the UniqueNetId here because the delegates don't use it
        delegate.ExecuteIfBound( FUniqueNetIdString(), privilege, static_cast< uint32 >( IOnlineIdentity::EPrivilegeResults::NoFailures ) );
    }
}

void UGBFGameInstanceSessionSubsystem::AddNetworkFailureHandlers()
{
    if ( GEngine->OnTravelFailure().IsBoundToObject( this ) == false )
    {
        TravelLocalSessionFailureDelegateHandle = GEngine->OnTravelFailure().AddUObject( this, &UGBFGameInstanceSessionSubsystem::TravelLocalSessionFailure );
    }
}

void UGBFGameInstanceSessionSubsystem::RemoveNetworkFailureHandlers() const
{
    if ( GEngine->OnTravelFailure().IsBoundToObject( this ) == true )
    {
        GEngine->OnTravelFailure().Remove( TravelLocalSessionFailureDelegateHandle );
    }
}

void UGBFGameInstanceSessionSubsystem::OnJoinSessionComplete( const EOnJoinSessionCompleteResult::Type result )
{
    if ( auto * game_session = GetGameSession() )
    {
        game_session->OnJoinSessionComplete().Remove( OnJoinSessionCompleteDelegateHandle );
    }

    const auto & local_players = GetOuterUGameInstance()->GetLocalPlayers();

    // :TODO: Add the split screen player if one exists
    /*if ( result == EOnJoinSessionCompleteResult::Success && local_players.Num() > 1 )
    {
        auto sessions = Online::GetSessionInterface( GetWorld() );
        const auto * local_player = local_players[ 1 ];
        const auto & local_player_unique_net_id = local_player->GetPreferredUniqueNetId();

        if ( sessions.IsValid() && local_player_unique_net_id.IsValid() )
        {
            sessions->RegisterLocalPlayer( *local_player_unique_net_id, NAME_GameSession, FOnRegisterLocalPlayerCompleteDelegate::CreateUObject( this, &UGBFGameInstanceSessionSubsystem::OnRegisterJoiningLocalPlayerComplete ) );
        }
    }
    else*/
    {
        // We either failed or there is only a single local user
        FinishJoinSession( result );
    }
}

void UGBFGameInstanceSessionSubsystem::OnRegisterJoiningLocalPlayerComplete( const FUniqueNetId & /* player_id */, const EOnJoinSessionCompleteResult::Type result ) const
{
    FinishJoinSession( result );
}

void UGBFGameInstanceSessionSubsystem::FinishJoinSession( const EOnJoinSessionCompleteResult::Type result ) const
{
    if ( result != EOnJoinSessionCompleteResult::Success )
    {
        FText return_reason;
        switch ( result )
        {
            case EOnJoinSessionCompleteResult::SessionIsFull:
            {
                return_reason = NSLOCTEXT( "NetworkErrors", "JoinSessionFailed", "Game is full." );
            }
            break;
            case EOnJoinSessionCompleteResult::SessionDoesNotExist:
            {
                return_reason = NSLOCTEXT( "NetworkErrors", "JoinSessionFailed", "Game no longer exists." );
            }
            break;
            default:
            {
                return_reason = NSLOCTEXT( "NetworkErrors", "JoinSessionFailed", "Join failed." );
            }
            break;
        }

        RemoveNetworkFailureHandlers();
        GetGBFGameInstance()->ShowMessageThenGotoMainMenuState( NSLOCTEXT( "NetworkErrors", "LocKey_JoinSessionFailedTitle", "Impossible to join the session" ), return_reason );
        return;
    }

    InternalTravelToSession( NAME_GameSession );
}

void UGBFGameInstanceSessionSubsystem::InternalTravelToSession( const FName session_name ) const
{
    auto * player_controller = GetGBFGameInstance()->GetFirstLocalPlayerController();

    FText error_reason;
    FString url;

    if ( player_controller == nullptr )
    {
        error_reason = NSLOCTEXT( "NetworkErrors", "InvalidPlayerController", "Invalid Player Controller" );
    }
    else
    {
        auto * online_sub = Online::GetSubsystem( GetWorld() );

        if ( online_sub == nullptr )
        {
            error_reason = NSLOCTEXT( "NetworkErrors", "OSSMissing", "OSS missing" );
        }
        else
        {
            auto sessions_interface_ptr = online_sub->GetSessionInterface();

            if ( !sessions_interface_ptr.IsValid() || !sessions_interface_ptr->GetResolvedConnectString( session_name, url ) )
            {
                error_reason = NSLOCTEXT( "NetworkErrors", "TravelSessionFailed", "Travel to Session failed." );
            }
        }
    }

    if ( !error_reason.IsEmpty() )
    {
        RemoveNetworkFailureHandlers();
        GetGBFGameInstance()->ShowMessageThenGotoMainMenuState( NSLOCTEXT( "NetworkErrors", "LocKey_JoinSessionFailedTitle", "Impossible to join the session" ), error_reason );
        return;
    }

    // :TODO:
    // Add debug encryption token if desired.
    // if ( CVarShooterGameTestEncryption->GetInt() != 0 )
    // {
    //     // This is just a value for testing/debugging, the server will use the same key regardless of the token value.
    //     // But the token could be a user ID and/or session ID that would be used to generate a unique key per user and/or session, if desired.
    //     URL += TEXT( "?EncryptionToken=1" );
    // }

    player_controller->ClientTravel( url, TRAVEL_Absolute );
}

void UGBFGameInstanceSessionSubsystem::OnRegisterLocalPlayerComplete( const FUniqueNetId & /* player_id */, const EOnJoinSessionCompleteResult::Type result )
{
    FinishSessionCreation( result );
}

void UGBFGameInstanceSessionSubsystem::SendPlayTogetherInvites()
{
    const auto * const oss = Online::GetSubsystem( GetWorld() );
    check( oss );

    const auto session_interface_ptr = oss->GetSessionInterface();
    check( session_interface_ptr.IsValid() );

    if ( PlayTogetherInfo.UserIndex != -1 )
    {
        for ( const ULocalPlayer * local_player : GetOuterUGameInstance()->GetLocalPlayers() )
        {
            if ( local_player->GetControllerId() == PlayTogetherInfo.UserIndex )
            {
                auto player_net_id = local_player->GetPreferredUniqueNetId();

                if ( player_net_id.IsValid() )
                {
                    // Automatically send invites to friends in the player's PS4 party to conform with Play Together requirements
                    for ( const auto & friend_id : PlayTogetherInfo.UserIdList )
                    {
                        session_interface_ptr->SendSessionInviteToFriend( *player_net_id, NAME_GameSession, *friend_id.ToSharedRef() );
                    }
                }
            }
        }

        PlayTogetherInfo = FGBFSessionPlayTogetherInfo();
    }
}

void UGBFGameInstanceSessionSubsystem::FinishSessionCreation( const EOnJoinSessionCompleteResult::Type result )
{
    if ( result == EOnJoinSessionCompleteResult::Success )
    {
        // This will send any Play Together invites if necessary, or do nothing.
        SendPlayTogetherInvites();

        // Travel to the specified match URL
        GetWorld()->ServerTravel( TravelURL );
    }
    else
    {
        GetGBFGameInstance()->ShowMessageThenGotoMainMenuState(
            NSLOCTEXT( "NetworkErrors", "CreateSessionFailed", "Failed to create session." ),
            NSLOCTEXT( "NetworkErrors", "CreateSessionFailed", "Failed to create session." ) );
    }
}

void UGBFGameInstanceSessionSubsystem::OnCreatePresenceSessionComplete( FName session_name, const bool was_successful )
{
    if ( auto * game_session = GetGameSession() )
    {
        game_session->OnCreatePresenceSessionComplete().Remove( OnCreatePresenceSessionCompleteDelegateHandle );

        // Add the split screen player if one exists

        const auto & local_players = GetOuterUGameInstance()->GetLocalPlayers();

        if ( was_successful && local_players.Num() > 1 )
        {
            auto sessions_interface_ptr = Online::GetSessionInterface( GetWorld() );
            const auto * local_player = local_players[ 1 ];
            const auto & local_player_unique_net_id = local_player->GetPreferredUniqueNetId();

            if ( sessions_interface_ptr.IsValid() && local_player_unique_net_id.IsValid() )
            {
                sessions_interface_ptr->RegisterLocalPlayer( *local_player_unique_net_id, NAME_GameSession, FOnRegisterLocalPlayerCompleteDelegate::CreateUObject( this, &UGBFGameInstanceSessionSubsystem::OnRegisterLocalPlayerComplete ) );
            }
        }
        else
        {
            // We either failed or there is only a single local user
            FinishSessionCreation( was_successful ? EOnJoinSessionCompleteResult::Success : EOnJoinSessionCompleteResult::UnknownError );
        }
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UGBFGameInstanceSessionSubsystem::OnSearchSessionsComplete( bool /* was_successful */ )
{
    if ( auto * game_session = GetGameSession() )
    {
        game_session->OnFindSessionsComplete().Remove( OnSearchSessionsCompleteDelegateHandle );
    }
}
