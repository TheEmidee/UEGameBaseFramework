#include "Engine/SubSystems/GBFGameInstanceSessionSubsystem.h"

#include "Engine/GBFGameInstance.h"
#include "Engine/GBFGameState.h"
#include "Engine/SubSystems/GBFGameInstanceGameStateSystem.h"
#include "GameBaseFrameworkSettings.h"

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
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UGBFGameInstanceSessionSubsystem::HandleSessionFailure( const FUniqueNetId & unique_net_id, const ESessionFailure::Type failure_type )
{
    UE_LOG( LogOnlineGame, Warning, TEXT( "UShooterGameInstance::HandleSessionFailure: %u" ), ( uint32 ) failure_type );

#if 1 //GBF_CONSOLE_UI
    // If we are not currently at (or heading to) the welcome screen then display a message on consoles
    if ( OnlineMode != EGBFOnlineMode::Offline && !GetSubsystem< UGBFGameInstanceGameStateSystem >()->IsOnWelcomeScreenState() )
    {
        UE_LOG( LogOnlineGame, Log, TEXT( "UShooterGameInstance::HandleSessionFailure: Going to main menu" ) );

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

        GetGBFGameInstance()->ShowMessageThenGotoState(
            NSLOCTEXT( "GBF", "LocKey_NeedLicenseTitle", "NetworkFailures" ),
            return_reason,
            settings->MainMenuGameState.Get() );
    }
#endif
}

void UGBFGameInstanceSessionSubsystem::CleanupSessionOnReturnToMenu()
{
    auto pending_online_operation = false;

    // end online game and then destroy it
    auto * online_sub = Online::GetSubsystem( GetWorld() );
    auto sessions = ( online_sub != nullptr ) ? online_sub->GetSessionInterface() : nullptr;

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
        //GEngine->HandleDisconnect( GetWorld(), GetWorld()->GetNetDriver() );
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
