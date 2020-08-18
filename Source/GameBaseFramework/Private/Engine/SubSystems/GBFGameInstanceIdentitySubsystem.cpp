#include "Engine/SubSystems/GBFGameInstanceIdentitySubsystem.h"

#include "Components/GBFUIDialogManagerComponent.h"
#include "Engine/GBFGameInstance.h"
#include "Engine/SubSystems/GBFGameInstanceControllerSubsystem.h"
#include "Engine/SubSystems/GBFGameInstanceGameStateSystem.h"
#include "Engine/SubSystems/GBFGameInstanceOnlineSubsystem.h"
#include "Engine/SubSystems/GBFGameInstanceSessionSubsystem.h"
#include "GameFramework/GBFPlayerController.h"
#include "Log/GBFLog.h"

#include <Engine/GameInstance.h>
#include <Engine/LocalPlayer.h>
#include <GameFramework/PlayerController.h>
#include <GameFramework/PlayerState.h>
#include <OnlineSubsystemUtils.h>

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

ULocalPlayer * UGBFGameInstanceIdentitySubsystem::GetLocalPlayerFromUniqueNetId( const FUniqueNetId & user_id ) const
{
    for ( auto local_player_iterator = GEngine->GetLocalPlayerIterator( GetWorld() ); local_player_iterator; ++local_player_iterator )
    {
        const auto other_id = ( *local_player_iterator )->GetPreferredUniqueNetId();
        if ( other_id.IsValid() )
        {
            if ( user_id == ( *other_id ) )
            {
                return *local_player_iterator;
            }
        }
    }

    return nullptr;
}

APlayerController * UGBFGameInstanceIdentitySubsystem::GetPlayerControllerFromUniqueNetId( const FUniqueNetId & user_id ) const
{
    if ( user_id.IsValid() )
    {
        for ( auto iterator = GetWorld()->GetPlayerControllerIterator(); iterator; ++iterator )
        {
            if ( auto * player_controller = iterator->Get() )
            {
                if ( player_controller->PlayerState != nullptr && player_controller->PlayerState->GetUniqueId().IsValid() )
                {
                    if ( *player_controller->PlayerState->GetUniqueId() == user_id )
                    {
                        return player_controller;
                    }
                }
            }
        }
    }

    return nullptr;
}

bool UGBFGameInstanceIdentitySubsystem::IsLocalPlayerOnline( ULocalPlayer * local_player ) const
{
    if ( local_player == nullptr )
    {
        return false;
    }

    if ( const IOnlineSubsystem * oss = Online::GetSubsystem( GetWorld() ) )
    {
        const auto identity_interface_ptr = oss->GetIdentityInterface();
        if ( identity_interface_ptr.IsValid() )
        {
            const auto unique_id = local_player->GetCachedUniqueNetId();

            if ( unique_id.IsValid() )
            {
                const auto login_status = identity_interface_ptr->GetLoginStatus( *unique_id );

                if ( login_status == ELoginStatus::LoggedIn )
                {
                    return true;
                }
            }
        }
    }

    return false;
}

bool UGBFGameInstanceIdentitySubsystem::IsLocalPlayerSignedIn( ULocalPlayer * local_player ) const
{
    if ( local_player == nullptr )
    {
        return false;
    }

    if ( const IOnlineSubsystem * oss = Online::GetSubsystem( GetWorld() ) )
    {
        const auto identity_interface_ptr = oss->GetIdentityInterface();
        if ( identity_interface_ptr.IsValid() )
        {
            const auto unique_id = local_player->GetCachedUniqueNetId();

            if ( unique_id.IsValid() )
            {
                return true;
            }
        }
    }

    return false;
}

bool UGBFGameInstanceIdentitySubsystem::ValidatePlayerForOnlinePlay( ULocalPlayer * local_player ) const
{
#if PLATFORM_XBOXONE
    if ( GetSubsystem< UGBFGameInstanceOnlineSubsystem >()->GetCurrentConnectionStatus() != EOnlineServerConnectionStatus::Connected )
    {
        if ( auto * pc = Cast< AGBFPlayerController >( local_player->GetPlayerController( GetWorld() ) )
        {
            pc->GetUIDialogManagerComponent()->ShowConfirmationPopup(
                NSLOCTEXT( "NetworkFailures", "ServiceDisconnected", "You must be connected to the Xbox LIVE service to play online." ),
                NSLOCTEXT( "NetworkFailures", "ServiceDisconnected", "You must be connected to the Xbox LIVE service to play online." ),
                EGBFUIDialogType::AdditiveOnTop );
        }

        return false;
    }
#endif

    if ( !IsLocalPlayerOnline( local_player ) )
    {
        if ( auto * pc = Cast< AGBFPlayerController >( local_player->GetPlayerController( GetWorld() ) ) )
        {
            pc->GetUIDialogManagerComponent()->ShowConfirmationPopup(
                NSLOCTEXT( "NetworkFailures", "MustBeSignedIn", "You must be signed in to play online" ),
                NSLOCTEXT( "NetworkFailures", "MustBeSignedIn", "You must be signed in to play online" ),
                EGBFUIDialogType::AdditiveOnTop );
        }

        return false;
    }

    return true;
}

bool UGBFGameInstanceIdentitySubsystem::ValidatePlayerIsSignedIn( ULocalPlayer * local_player ) const
{
    if ( !IsLocalPlayerSignedIn( local_player ) )
    {
        if ( auto * pc = Cast< AGBFPlayerController >( local_player->GetPlayerController( GetWorld() ) ) )
        {
            pc->GetUIDialogManagerComponent()->ShowConfirmationPopup(
                NSLOCTEXT( "NetworkFailures", "MustBeSignedIn", "You must be signed in to play online" ),
                NSLOCTEXT( "NetworkFailures", "MustBeSignedIn", "You must be signed in to play online" ),
                EGBFUIDialogType::AdditiveOnTop );
        }

        return false;
    }

    return true;
}

void UGBFGameInstanceIdentitySubsystem::HandleUserLoginChanged( const int32 game_user_index, ELoginStatus::Type /* previous_login_status */, const ELoginStatus::Type login_status, const FUniqueNetId & user_id )
{
    const auto offline_mode = GetSubsystem< UGBFGameInstanceSessionSubsystem >()->GetOnlineMode();

#if PLATFORM_SWITCH
    const auto is_downgraded = LoginStatus == ELoginStatus::NotLoggedIn || ( offline_mode == EGBFOnlineMode::Online && LoginStatus == ELoginStatus::UsingLocalProfile );
#else
    const auto is_downgraded = ( login_status == ELoginStatus::NotLoggedIn && offline_mode == EGBFOnlineMode::Offline ) || ( login_status != ELoginStatus::LoggedIn && offline_mode != EGBFOnlineMode::Offline );
#endif

    UE_LOG( LogGBF_OSS, Verbose, TEXT( "HandleUserLoginChanged: bDownGraded: %i" ), ( int ) is_downgraded );

    LocalPlayerOnlineStatus[ game_user_index ] = login_status;

    auto * game_instance = GetOuterUGameInstance();
    if ( auto * local_player = game_instance->FindLocalPlayerFromUniqueNetId( user_id ) )
    {
        if ( is_downgraded )
        {
            UE_LOG( LogGBF_OSS, Log, TEXT( "HandleUserLoginChanged: Player logged out: %s" ), *user_id.ToString() );

            // :TODO: Broadcast an event
            //LabelPlayerAsQuitter(LocalPlayer);

            if ( local_player == game_instance->GetFirstGamePlayer() || offline_mode == EGBFOnlineMode::Offline )
            {
                GetGBFGameInstance()->HandleSignInChangeMessaging();
            }
            else
            {
                GetGBFGameInstance()->RemoveExistingLocalPlayer( local_player );
            }
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

void UGBFGameInstanceIdentitySubsystem::OnGameStateChanged( FName /*state_name*/, const UGBFGameState * /*new_state*/ )
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