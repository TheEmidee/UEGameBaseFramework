#include "Engine/SubSystems/GBFGameInstanceIdentitySubsystem.h"

#include "Engine/GBFGameInstance.h"
#include "Engine/SubSystems/GBFGameInstanceControllerSubsystem.h"
#include "Engine/SubSystems/GBFGameInstanceGameStateSystem.h"
#include "Log/GBFLog.h"

#include <Engine/GameInstance.h>
#include <Engine/LocalPlayer.h>

void UGBFGameInstanceIdentitySubsystem::Initialize( FSubsystemCollectionBase & collection )
{
    Super::Initialize( collection );

    LocalPlayerOnlineStatus.InsertDefaulted( 0, MAX_LOCAL_PLAYERS );

    const auto oss = IOnlineSubsystem::Get();
    check( oss != nullptr );

    const auto identity_interface = oss->GetIdentityInterface();
    check( identity_interface.IsValid() );

    for ( auto i = 0; i < MAX_LOCAL_PLAYERS; ++i )
    {
        identity_interface->AddOnLoginStatusChangedDelegate_Handle( i, FOnLoginStatusChangedDelegate::CreateUObject( this, &UGBFGameInstanceIdentitySubsystem::HandleUserLoginChanged ) );
    }

    GetSubsystem< UGBFGameInstanceGameStateSystem >()->OnStateChanged().AddDynamic( this, &UGBFGameInstanceIdentitySubsystem::OnGameStateChanged );
}

bool UGBFGameInstanceIdentitySubsystem::ProfileUISwap( const int controller_index )
{
    return ShowLoginUI( controller_index, FOnLoginUIClosedDelegate::CreateLambda( [this]( const TSharedPtr< const FUniqueNetId > unique_net_id, const int, const FOnlineError & ) {
        if ( unique_net_id->IsValid() )
        {
            GetOuterUGameInstance()->GetSubsystem< UGBFGameInstanceGameStateSystem >()->GoToWelcomeScreenState();
        }
    } ) );
}

bool UGBFGameInstanceIdentitySubsystem::ShowLoginUI( const int controller_index, const FOnLoginUIClosedDelegate & delegate )
{
    if ( ensure( !LoginUIClosedDelegate.IsBound() ) )
    {
        if ( const auto * oss = IOnlineSubsystem::Get() )
        {
            const auto external_ui_interface = oss->GetExternalUIInterface();

            if ( external_ui_interface.IsValid() )
            {
                LoginUIClosedDelegate = delegate;

                auto * controller_subsystem = GetOuterUGameInstance()->GetSubsystem< UGBFGameInstanceControllerSubsystem >();

                controller_subsystem->SetIgnorePairingChangeForControllerId( controller_index );

                if ( external_ui_interface->ShowLoginUI( controller_index, false, false, FOnLoginUIClosedDelegate::CreateUObject( this, &UGBFGameInstanceIdentitySubsystem::OnLoginUIClosed ) ) )
                {
                    return true;
                }
                controller_subsystem->SetIgnorePairingChangeForControllerId( -1 );
            }
        }
    }

    return false;
}

void UGBFGameInstanceIdentitySubsystem::HandleUserLoginChanged( const int32 game_user_index, ELoginStatus::Type /* previous_login_status */, const ELoginStatus::Type login_status, const FUniqueNetId & user_id )
{
    const auto is_downgraded = login_status == ELoginStatus::NotLoggedIn;

    UE_LOG( LogGBF_OSS, Verbose, TEXT( "HandleUserLoginChanged: bDownGraded: %i" ), ( int ) is_downgraded );

    LocalPlayerOnlineStatus[ game_user_index ] = login_status;

    if ( GetOuterUGameInstance()->FindLocalPlayerFromUniqueNetId( user_id ) )
    {
        if ( is_downgraded )
        {
            UE_LOG( LogGBF_OSS, Log, TEXT( "HandleUserLoginChanged: Player logged out: %s" ), *user_id.ToString() );

            GetGBFGameInstance()->HandleSignInChangeMessaging();
        }
    }
}

void UGBFGameInstanceIdentitySubsystem::OnLoginUIClosed( const TSharedPtr< const FUniqueNetId > unique_id, const int controller_index, const FOnlineError & error )
{
    auto * controller_subsystem = GetOuterUGameInstance()->GetSubsystem< UGBFGameInstanceControllerSubsystem >();
    controller_subsystem->SetIgnorePairingChangeForControllerId( -1 );

    // If the id is null, the user backed out

    if ( unique_id.IsValid() && unique_id->IsValid() && CurrentUniqueNetId.IsValid() && *CurrentUniqueNetId == *unique_id )
    {
        LoginUIClosedDelegate.Unbind();
        return;
    }

    if ( LoginUIClosedDelegate.IsBound() )
    {
        LoginUIClosedDelegate.Execute( unique_id, controller_index, error );
        LoginUIClosedDelegate.Unbind();
    }

    if ( unique_id.IsValid() && unique_id->IsValid() )
    {
        CurrentUniqueNetId = unique_id;
    }
}

void UGBFGameInstanceIdentitySubsystem::OnGameStateChanged( const UGBFGameState * /*new_state*/ )
{
    if ( !GetSubsystem< UGBFGameInstanceGameStateSystem >()->IsOnWelcomeScreenState() )
    {
        return;
    }

    if ( CurrentUniqueNetId.IsValid() )
    {
        if ( auto * local_player = GetOuterUGameInstance()->FindLocalPlayerFromUniqueNetId( *CurrentUniqueNetId ) )
        {
            local_player->SetCachedUniqueNetId( nullptr );
        }
    }

    CurrentUniqueNetId = nullptr;
}