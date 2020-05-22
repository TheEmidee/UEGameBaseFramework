#include "GameFramework/GBFPlayerControllerLogin.h"

#include "Components/GBFUIDialogManagerComponent.h"
#include "Engine/GBFLocalPlayer.h"
#include "GameFramework/GBFGameModeLogin.h"
#include "Log/GBFLog.h"

#include <Framework/Application/SlateApplication.h>
#include <OnlineSubsystem.h>

void AGBFPlayerControllerLogin::TryLogIn( const int player_index )
{
    if ( IsHandlingLoginFlow || player_index == INDEX_NONE )
    {
        return;
    }

    SetItIsHandlingLoginFlow( true );

    if ( const auto * oss = IOnlineSubsystem::Get() )
    {
        const auto identity_interface = oss->GetIdentityInterface();

        if ( identity_interface.IsValid() )
        {
            auto generic_application = FSlateApplication::Get().GetPlatformApplication();

            // ReSharper disable once CppLocalVariableMayBeConst
            auto can_move_to_main_menu = true;

#if !PLATFORM_DESKTOP
            const auto login_status = identity_interface->GetLoginStatus( player_index );
            const auto is_licensed = generic_application->ApplicationLicenseValid();

            if ( login_status == ELoginStatus::NotLoggedIn || !is_licensed )
            {
                if ( GameInstance->ShowLoginUI( player_index, FOnLoginUIClosedDelegate::CreateUObject( this, &AGBFPlayerControllerLogin::HandleLoginUIClosed ) ) )
                {
                    can_move_to_main_menu = false;
                }
            }
#endif

            if ( can_move_to_main_menu )
            {
                const auto player_id = identity_interface->GetUniquePlayerId( player_index );

#if PLATFORM_DESKTOP
                PendingControllerIndex = player_index;
                OnUserCanPlay( *player_id, EUserPrivileges::CanPlay, 0 );
#else
                // If we couldn't show the external login UI for any reason, or if the user is
                // already logged in, just advance to the main menu immediately.
                GameInstance->OnLoginUIClosed( player_id, player_index );
                HandleLoginUIClosed( player_id, player_index );
#endif
            }
        }
    }
}

void AGBFPlayerControllerLogin::SetControllerAndAdvanceToMainMenu( const int controller_index )
{
    SetItIsHandlingLoginFlow( false );

    if ( !ensure( GetGameInstance() != nullptr ) )
    {
        return;
    }

    if ( controller_index == -1 )
    {
        return;
    }

    if ( auto * local_player = Cast< UGBFLocalPlayer >( GetLocalPlayer() ) )
    {
        local_player->InitializeAfterLogin( controller_index );
    }

    if ( auto * login_game_mode = GetWorld()->GetAuthGameMode< AGBFGameModeLogin >() )
    {
        login_game_mode->InitializeLocalPlayer( controller_index );
    }
}

void AGBFPlayerControllerLogin::HandleLoginUIClosed( const TSharedPtr< const FUniqueNetId > unique_id, const int controller_index )
{
    if ( !ensure( GetGameInstance() != nullptr ) )
    {
        return;
    }

    if ( unique_id.IsValid() && !unique_id->IsValid() )
    {
        OnUserCannotPlay();
        return;
    }

    auto generic_application = FSlateApplication::Get().GetPlatformApplication();
    const auto is_licensed = generic_application->ApplicationLicenseValid();

    // If they don't currently have a license, let them know, but don't let them proceed
    if ( !is_licensed )
    {
        GetUIDialogManagerComponent()->ShowConfirmationPopup(
            NSLOCTEXT( "GBF", "LocKey_NeedLicenseTitle", "Invalid license" ),
            NSLOCTEXT( "GBF", "LocKey_NeedLicenseContent", "The signed in users do not have a license for this game. Please purchase that game or sign in a user with a valid license." ),
            EGBFUIDialogType::AdditiveOnlyOneVisible,
            FGBFConfirmationPopupButtonClicked::CreateUObject( this, &AGBFPlayerControllerLogin::OnUserCannotPlay ) );
        return;
    }

    if ( unique_id.IsValid() && unique_id->IsValid() )
    {
        PendingControllerIndex = controller_index;

        // Next step, check privileges
        if ( const auto * oss = IOnlineSubsystem::Get() )
        {
            const auto identity_interface = oss->GetIdentityInterface();

            if ( identity_interface.IsValid() )
            {
                identity_interface->GetUserPrivilege( *unique_id, EUserPrivileges::CanPlay, IOnlineIdentity::FOnGetUserPrivilegeCompleteDelegate::CreateUObject( this, &AGBFPlayerControllerLogin::OnUserCanPlay ) );
            }
        }
    }
    else
    {
        GetUIDialogManagerComponent()->ShowConfirmationPopup(
            NSLOCTEXT( "GBF", "LocKey_ProgressWillNotBeSavedTitle", "Sign in" ),
            NSLOCTEXT( "GBF", "LocKey_ProgressWillNotBeSavedContent", "You need to sign in to play Shift Quantum." ),
            EGBFUIDialogType::AdditiveOnlyOneVisible,
            FGBFConfirmationPopupButtonClicked::CreateUObject( this, &AGBFPlayerControllerLogin::OnUserCannotPlay ) );
    }
}

void AGBFPlayerControllerLogin::OnUserCanPlay( const FUniqueNetId & /*user_id*/, const EUserPrivileges::Type /*privilege*/, const uint32 privilege_result )
{
    if ( privilege_result == static_cast< uint32 >( IOnlineIdentity::EPrivilegeResults::NoFailures ) )
    {
        TryToConnectToOnlineInterface();
    }
    else if ( privilege_result & static_cast< uint32 >( IOnlineIdentity::EPrivilegeResults::UserNotLoggedIn ) )
    {
        GetUIDialogManagerComponent()->ShowConfirmationPopup(
            NSLOCTEXT( "GBF", "LocKey_PrivilegeFailuresTitleLoggedIn", "Not logged in" ),
            NSLOCTEXT( "GBF", "LocKey_PrivilegeFailuresContentLoggedIn", "You must be logged in to play this game." ),
            EGBFUIDialogType::AdditiveOnlyOneVisible,
            FGBFConfirmationPopupButtonClicked::CreateUObject( this, &AGBFPlayerControllerLogin::OnUserCannotPlay ) );
    }
    else if ( privilege_result & static_cast< uint32 >( IOnlineIdentity::EPrivilegeResults::AgeRestrictionFailure ) )
    {
        GetUIDialogManagerComponent()->ShowConfirmationPopup(
            NSLOCTEXT( "GBF", "LocKey_PrivilegeFailuresTitleAge", "Age restriction" ),
            NSLOCTEXT( "GBF", "LocKey_PrivilegeFailuresContentAge", "You cannot play this game due to age restrictions." ),
            EGBFUIDialogType::AdditiveOnlyOneVisible,
            FGBFConfirmationPopupButtonClicked::CreateUObject( this, &AGBFPlayerControllerLogin::OnUserCannotPlay ) );
    }
    else
    {
        GetUIDialogManagerComponent()->ShowConfirmationPopup(
            NSLOCTEXT( "GBF", "LocKey_PrivilegeFailuresTitleDefault", "Not enough privileges" ),
            NSLOCTEXT( "GBF", "LocKey_PrivilegeFailuresContentDefault", "You are not allowed to play this game because of privilege restrictions." ),
            EGBFUIDialogType::AdditiveOnlyOneVisible,
            FGBFConfirmationPopupButtonClicked::CreateUObject( this, &AGBFPlayerControllerLogin::OnUserCannotPlay ) );
    }
}

void AGBFPlayerControllerLogin::TryToConnectToOnlineInterface()
{
    if ( OnLoginCompleteDelegateHandle.IsValid() )
    {
        return;
    }

    IsHandlingLoginSuccess = false;

    if ( const auto * oss = IOnlineSubsystem::Get() )
    {
        const auto identity_interface = oss->GetIdentityInterface();

        if ( identity_interface.IsValid() )
        {
            OnLoginCompleteDelegateHandle = identity_interface->AddOnLoginCompleteDelegate_Handle( PendingControllerIndex, FOnLoginCompleteDelegate::CreateUObject( this, &AGBFPlayerControllerLogin::OnLoginSucceeded ) );

            if ( oss->GetSubsystemName() != NULL_SUBSYSTEM )
            {
                if ( !identity_interface->Login( PendingControllerIndex, FOnlineAccountCredentials() ) )
                {
                    // Login task was not started. Just skip and move to OnLoginSucceeded.
                    OnLoginSucceeded( PendingControllerIndex, true, FUniqueNetIdString( "" ), "" );
                }
            }
            else
            {
                OnLoginSucceeded( PendingControllerIndex, true, FUniqueNetIdString( "" ), "" );
            }
        }
    }
}

void AGBFPlayerControllerLogin::OnLoginSucceeded( const int32 local_user_num, const bool was_successful, const FUniqueNetId & /*user_id*/, const FString & error )
{
    // Some online interfaces (Steam for example) call OnLoginCompleteDelegate directly in Login, and return immediately.
    // If for some reason its not possible to connect to the OSS, we end up here twice. Don't allow that, using that flag
    if ( IsHandlingLoginSuccess )
    {
        return;
    }

    IsHandlingLoginSuccess = true;

    IOnlineSubsystem::Get()->GetIdentityInterface()->ClearOnLoginCompleteDelegate_Handle( local_user_num, OnLoginCompleteDelegateHandle );

    if ( was_successful )
    {
        SetControllerAndAdvanceToMainMenu( PendingControllerIndex );
    }
    else
    {
        UE_LOG( LogGBF_OSS, Error, TEXT( "Error logging in: %s" ), *error );
        OnContinueOffline();
    }
}

void AGBFPlayerControllerLogin::OnContinueWithoutSavingConfirm()
{
    SetControllerAndAdvanceToMainMenu( PendingControllerIndex );
}

void AGBFPlayerControllerLogin::SetItIsHandlingLoginFlow( const bool result )
{
    if ( IsHandlingLoginFlow != result )
    {
        IsHandlingLoginFlow = result;
        OnRequiresUserAction.Broadcast( !IsHandlingLoginFlow );
    }
}

void AGBFPlayerControllerLogin::OnUserCannotPlay()
{
    SetItIsHandlingLoginFlow( false );
}

void AGBFPlayerControllerLogin::OnContinueOffline()
{
    GetUIDialogManagerComponent()->ShowConfirmationPopup(
        NSLOCTEXT( "GBF", "LocKey_LoginFailuresTitle", "ERROR LOGGING IN" ),
        NSLOCTEXT( "GBF", "LocKey_LoginFailuresContent", "THERE WAS A PROBLEM CONNECTING YOU TO THE ONLINE SERVICE. YOU WILL NOT BE ABLE TO USE THE COMMUNITY FEATURES OF THE GAME." ),
        EGBFUIDialogType::AdditiveOnlyOneVisible,
        FGBFConfirmationPopupButtonClicked::CreateUObject( this, &AGBFPlayerControllerLogin::TryToConnectToOnlineInterface ),
        FGBFConfirmationPopupButtonClicked::CreateUObject( this, &AGBFPlayerControllerLogin::OnContinueWithoutSavingConfirm ),
        NSLOCTEXT( "GBF", "LocKey_Retry", "RETRY" ),
        NSLOCTEXT( "GBF", "LocKey_Continue", "CONTINUE" ) );
}
