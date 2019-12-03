#include "GBFGameInstance.h"

#include "BlueprintLibraries/GBFHelperBlueprintLibrary.h"
#include "Components/GBFUIDialogManagerComponent.h"
#include "GBFGameInstanceGameStateSystem.h"
#include "GBFGameState.h"
#include "GBFLocalPlayer.h"
#include "GBFTypes.h"
#include "GameBaseFrameworkSettings.h"
#include "GameFramework/GBFGameModeBase.h"
#include "GameFramework/GBFPlayerController.h"
#include "Log/GBFLog.h"
#include "Sound/SoundMix.h"
#include "UI/GBFConfirmationWidget.h"

#include <Containers/Ticker.h>
#include <CoreDelegates.h>
#include <Engine/AssetManager.h>
#include <Engine/Canvas.h>
#include <GameFramework/GameModeBase.h>
#include <Kismet/GameplayStatics.h>
#include <Online.h>
#include <OnlineExternalUIInterface.h>
#include <OnlineSubsystem.h>
#include <SlateApplication.h>
#include <SoftObjectPtr.h>

#if PLATFORM_XBOXONE
class FGBFXBoxOneDisconnectedInputProcessor : public IInputProcessor
{
public:
    virtual void Tick( const float delta_time, FSlateApplication & slate_application, TSharedRef< ICursor > cursor ) {};

    virtual bool HandleKeyUpEvent( FSlateApplication & slate_application, const FKeyEvent & key_event ) override
    {
        auto controller_index = key_event.GetUserIndex();

        if ( key_event.GetKey() == FGamepadKeyNames::FaceButtonBottom )
        {
            auto * game_instance = UGBFGameInstance::Get();
            auto * local_player = game_instance->FindLocalPlayerFromControllerId( controller_index );

            if ( local_player == nullptr )
            {
                game_instance->ProfileUISwap( controller_index );
            }
        }

        return false;
    }
};
#endif

#if PLATFORM_PS4
static void ExtendedSaveGameInfoDelegate( const TCHAR * save_name, const EGameDelegates_SaveGame key, FString & value )
{
    // Fill save parameter : size - icon path - etc...
}
#endif

UGBFGameInstance::UGBFGameInstance() :
    IsLicensed( true ) // Default to licensed (should have been checked by OS on boot)
    ,
    IgnorePairingChangeForControllerId( -1 )
{
}

void UGBFGameInstance::Init()
{
    Super::Init();

    Settings = GetDefault< UGameBaseFrameworkSettings >();

    check( Settings != nullptr );

    CurrentConnectionStatus = EOnlineServerConnectionStatus::Connected;

    const auto oss = IOnlineSubsystem::Get();
    check( oss != nullptr );

    const auto identity_interface = oss->GetIdentityInterface();
    check( identity_interface.IsValid() );

    LocalPlayerOnlineStatus.InsertDefaulted( 0, MAX_LOCAL_PLAYERS );

    for ( auto i = 0; i < MAX_LOCAL_PLAYERS; ++i )
    {
        identity_interface->AddOnLoginStatusChangedDelegate_Handle( i, FOnLoginStatusChangedDelegate::CreateUObject( this, &UGBFGameInstance::HandleUserLoginChanged ) );
    }

    identity_interface->AddOnControllerPairingChangedDelegate_Handle( FOnControllerPairingChangedDelegate::CreateUObject( this, &UGBFGameInstance::HandleControllerPairingChanged ) );

    FCoreDelegates::ApplicationWillDeactivateDelegate.AddUObject( this, &UGBFGameInstance::HandleAppWillDeactivate );
    FCoreDelegates::ApplicationHasReactivatedDelegate.AddUObject( this, &UGBFGameInstance::HandleAppHasReactivated );
    FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddUObject( this, &UGBFGameInstance::HandleAppWillEnterBackground );
    FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddUObject( this, &UGBFGameInstance::HandleAppHasEnteredForeground );
    FCoreDelegates::OnSafeFrameChangedEvent.AddUObject( this, &UGBFGameInstance::HandleSafeFrameChanged );
    FCoreDelegates::OnControllerConnectionChange.AddUObject( this, &UGBFGameInstance::HandleControllerConnectionChange );
    FCoreDelegates::ApplicationLicenseChange.AddUObject( this, &UGBFGameInstance::HandleAppLicenseUpdate );

#if PLATFORM_PS4
    FGameDelegates::Get().GetExtendedSaveGameInfoDelegate() = FExtendedSaveGameInfoDelegate::CreateStatic( LOCAL_ExtendedSaveGameInfoDelegate );
#endif

    oss->AddOnConnectionStatusChangedDelegate_Handle( FOnConnectionStatusChangedDelegate::CreateUObject( this, &UGBFGameInstance::HandleNetworkConnectionStatusChanged ) );

    TickDelegate = FTickerDelegate::CreateUObject( this, &UGBFGameInstance::Tick );
    TickDelegateHandle = FTicker::GetCoreTicker().AddTicker( TickDelegate );

    if ( SoundMix.IsValid() )
    {
        UAssetManager::GetStreamableManager().RequestAsyncLoad( SoundMix.ToSoftObjectPath() );
    }

    GetSubsystem< UGBFGameInstanceGameStateSystem >()->OnStateChanged().AddDynamic( this, &UGBFGameInstance::OnGameStateChanged );
}

void UGBFGameInstance::Shutdown()
{
    Super::Shutdown();

    FTicker::GetCoreTicker().RemoveTicker( TickDelegateHandle );
}

AGameModeBase * UGBFGameInstance::CreateGameModeForURL( const FURL url )
{
    auto * game_mode = Super::CreateGameModeForURL( url );

    GetSubsystem< UGBFGameInstanceGameStateSystem >()->UpdateCurrentGameStateFromCurrentWorld();

    return game_mode;
}

bool UGBFGameInstance::Tick( float /*delta_seconds*/ )
{
    if ( !GetSubsystem< UGBFGameInstanceGameStateSystem >()->IsOnWelcomeScreenState() && LocalPlayers.Num() > 0 )
    {
        if ( auto * local_player = Cast< UGBFLocalPlayer >( LocalPlayers[ 0 ] ) )
        {
            if ( auto * player_controller = Cast< AGBFPlayerController >( local_player->PlayerController ) )
            {
                const auto is_displaying_dialog = player_controller->GetUIDialogManagerComponent()->IsDisplayingDialog();

                // If at any point we aren't licensed (but we are after welcome screen) bounce them back to the welcome screen
                if ( !IsLicensed && !is_displaying_dialog )
                {
                    ShowMessageThenGotoState(
                        NSLOCTEXT( "GBF", "LocKey_NeedLicenseTitle", "Invalid license" ),
                        NSLOCTEXT( "GBF", "LocKey_NeedLicenseContent", "The signed in users do not have a license for this game. Please purchase that game or sign in a user with a valid license." ),
                        Settings->WelcomeScreenGameState.Get() );

                    return true;
                }
            }
        }
    }

    return true;
}

void UGBFGameInstance::PushSoundMixModifier() const
{
    if ( auto * sound_mix = SoundMix.Get() )
    {
        UGameplayStatics::PushSoundMixModifier( this, sound_mix );
    }
}

void UGBFGameInstance::PopSoundMixModifier() const
{
    if ( auto * sound_mix = SoundMix.Get() )
    {
        UGameplayStatics::PopSoundMixModifier( this, sound_mix );
    }
}

bool UGBFGameInstance::ProfileUISwap( const int controller_index )
{
    return ShowLoginUI( controller_index, FOnLoginUIClosedDelegate::CreateLambda( [ this ]( const TSharedPtr< const FUniqueNetId > unique_net_id, const int, const FOnlineError & ) {
        if ( unique_net_id->IsValid() )
        {
            GetSubsystem< UGBFGameInstanceGameStateSystem >()->GoToWelcomeScreenState();
        }
    } ) );
}

bool UGBFGameInstance::ShowLoginUI( const int controller_index, const FOnLoginUIClosedDelegate & delegate )
{
    if ( ensure( !LoginUIClosedDelegate.IsBound() ) )
    {
        if ( const auto * oss = IOnlineSubsystem::Get() )
        {
            const auto external_ui_interface = oss->GetExternalUIInterface();

            if ( external_ui_interface.IsValid() )
            {
                LoginUIClosedDelegate = delegate;
                IgnorePairingChangeForControllerId = controller_index;

                if ( external_ui_interface->ShowLoginUI( controller_index, false, false, FOnLoginUIClosedDelegate::CreateUObject( this, &UGBFGameInstance::OnLoginUIClosed ) ) )
                {
                    return true;
                }
                IgnorePairingChangeForControllerId = -1;
            }
        }
    }

    return false;
}

ULocalPlayer * UGBFGameInstance::GetFirstLocalPlayer() const
{
    return LocalPlayers.Num() > 0
               ? LocalPlayers[ 0 ]
               : nullptr;
}

// -- PRIVATE

// ReSharper disable CppMemberFunctionMayBeStatic
void UGBFGameInstance::HandleAppWillDeactivate()
// ReSharper restore CppMemberFunctionMayBeStatic
{
    UE_LOG( LogGBF_OSS, Warning, TEXT( "UGBFGameInstance::HandleAppWillDeactivate" ) );

#if PLATFORM_PS4
    HandleAppDeactivateOrBackground();
#endif
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void UGBFGameInstance::HandleAppHasReactivated()
{
    UE_LOG( LogGBF_OSS, Warning, TEXT( "UGBFGameInstance::HandleAppHasReactivated" ) );

#if PLATFORM_PS4
    HandleAppReactivateOrForeground();
#endif
}

// ReSharper disable CppMemberFunctionMayBeStatic
void UGBFGameInstance::HandleAppWillEnterBackground()
// ReSharper restore CppMemberFunctionMayBeStatic
{
    UE_LOG( LogGBF_OSS, Warning, TEXT( "UGBFGameInstance::HandleAppWillEnterBackground" ) );

#if PLATFORM_SWITCH || PLATFORM_XBOXONE
    HandleAppDeactivateOrBackground();
#endif
}

// ReSharper disable CppMemberFunctionMayBeStatic
void UGBFGameInstance::HandleAppHasEnteredForeground()
// ReSharper restore CppMemberFunctionMayBeStatic
{
    UE_LOG( LogGBF_OSS, Log, TEXT( "UGBFGameInstance::HandleAppHasEnteredForeground" ) );

#if PLATFORM_SWITCH || PLATFORM_XBOXONE
    HandleAppReactivateOrForeground();
#endif
}

void UGBFGameInstance::HandleAppDeactivateOrBackground() const
{
    if ( auto * gm = GetWorld()->GetAuthGameMode< AGBFGameModeBase >() )
    {
        gm->HandleAppSuspended();
    }
}

void UGBFGameInstance::HandleAppReactivateOrForeground()
{
    if ( !GetSubsystem< UGBFGameInstanceGameStateSystem >()->IsOnWelcomeScreenState() )
    {
        UE_LOG( LogGBF_OSS, Warning, TEXT( "UGBFGameInstance::HandleAppReactivateOrForeground: Attempting to sign out players" ) );

        for ( auto i = 0; i < LocalPlayers.Num(); ++i )
        {
            if ( auto * lp = Cast< UGBFLocalPlayer >( LocalPlayers[ i ] ) )
            {
                if ( lp->GetCachedUniqueNetId().IsValid() && LocalPlayerOnlineStatus[ i ] == ELoginStatus::LoggedIn && lp->GetLoginStatus() != ELoginStatus::LoggedIn )
                {
                    UE_LOG( LogGBF_OSS, Log, TEXT( "UGBFGameInstance::HandleAppReactivateOrForeground: Signed out during resume." ) );
                    HandleSignInChangeMessaging();
                    return;
                }
            }
        }
    }

    if ( auto * gm = GetWorld()->GetAuthGameMode< AGBFGameModeBase >() )
    {
        gm->HandleAppResumed();
    }
}

// ReSharper disable CppMemberFunctionMayBeStatic
void UGBFGameInstance::HandleSafeFrameChanged()
// ReSharper restore CppMemberFunctionMayBeStatic
{
    UCanvas::UpdateAllCanvasSafeZoneData();
}

void UGBFGameInstance::HandleAppLicenseUpdate()
{
    auto generic_application = FSlateApplication::Get().GetPlatformApplication();
    IsLicensed = generic_application->ApplicationLicenseValid();
}

void UGBFGameInstance::HandleUserLoginChanged( const int32 game_user_index, const ELoginStatus::Type /*previous_login_status*/, const ELoginStatus::Type login_status, const FUniqueNetId & user_id )
{
    const auto is_downgraded = login_status == ELoginStatus::NotLoggedIn;

    UE_LOG( LogGBF_OSS, Verbose, TEXT( "HandleUserLoginChanged: bDownGraded: %i" ), ( int ) is_downgraded );

    HandleAppLicenseUpdate();

    LocalPlayerOnlineStatus[ game_user_index ] = login_status;

    if ( FindLocalPlayerFromUniqueNetId( user_id ) )
    {
        if ( is_downgraded )
        {
            UE_LOG( LogGBF_OSS, Log, TEXT( "HandleUserLoginChanged: Player logged out: %s" ), *user_id.ToString() );

            HandleSignInChangeMessaging();
        }
    }
}

// ReSharper disable CppMemberFunctionMayBeConst
void UGBFGameInstance::HandleControllerPairingChanged( const int game_user_index, const FUniqueNetId & previous_user, const FUniqueNetId & new_user )
// ReSharper restore CppMemberFunctionMayBeConst
{
#if PLATFORM_XBOXONE
    // update game_user_index based on previous controller index from stable index
#endif

    UE_LOG( LogGBF_OSS, Log, TEXT( "UGBFGameInstance::HandleControllerPairingChanged GameUserIndex %d PreviousUser '%s' NewUser '%s'" ), game_user_index, *previous_user.ToString(), *new_user.ToString() );

    if ( GetSubsystem< UGBFGameInstanceGameStateSystem >()->IsOnWelcomeScreenState() )
    {
    }

#if PLATFORM_XBOXONE
    if ( IgnorePairingChangeForControllerId != -1 && game_user_index == IgnorePairingChangeForControllerId )
    {
        // We were told to ignore
        IgnorePairingChangeForControllerId = -1; // Reset now so there there is no chance this remains in a bad state
        return;
    }

    if ( previous_user.IsValid() && !new_user.IsValid() )
    {
        // Treat this as a disconnect or sign-out, which is handled somewhere else
        return;
    }

    if ( !previous_user.IsValid() && new_user.IsValid() )
    {
        // Treat this as a signin
        ULocalPlayer * controlled_local_player = FindLocalPlayerFromControllerId( game_user_index );

        if ( controlled_local_player != nullptr && !controlled_local_player->GetCachedUniqueNetId().IsValid() )
        {
            // If a player that previously selected "continue without saving" signs into this controller, move them back to welcome screen
            HandleSignInChangeMessaging();
        }

        return;
    }

    if ( previous_user.IsValid() && new_user.IsValid() )
    {
        check( previous_user != new_user );
        check( previous_user == *CurrentUniqueNetId );

        GoToWelcomeScreenState();
    }
#endif
}

void UGBFGameInstance::HandleNetworkConnectionStatusChanged( const FString & /*service_name*/, const EOnlineServerConnectionStatus::Type /*last_connection_status*/, const EOnlineServerConnectionStatus::Type connection_status )
{
    UE_LOG( LogGBF_OSS, Warning, TEXT( "UGBFGameInstance::HandleNetworkConnectionStatusChanged: %s" ), EOnlineServerConnectionStatus::ToString( connection_status ) );

    if ( !GetSubsystem< UGBFGameInstanceGameStateSystem >()->IsOnWelcomeScreenState() && connection_status != EOnlineServerConnectionStatus::Connected )
    {
        UE_LOG( LogGBF_OSS, Log, TEXT( "UGBFGameInstance::HandleNetworkConnectionStatusChanged: Going to main menu" ) );

        const auto oss = IOnlineSubsystem::Get();
        FText return_reason;

        if ( oss != nullptr )
        {
            return_reason = FText::Format( NSLOCTEXT( "GBF", "LocKey_ServiceUnavailableContent", "Connection to {OnlineSystemName} has been lost." ), oss->GetOnlineServiceName() );
        }
        else
        {
            return_reason = NSLOCTEXT( "GBF", "LocKey_ServiceUnavailableContentFallback", "Connection to the online service has been lost." );
        }

        ShowMessageThenGotoState( NSLOCTEXT( "GBF", "LocKey_ServiceUnAvailableTitle", "Service Unavailable" ), return_reason, Settings->WelcomeScreenGameState.Get() );
    }

    CurrentConnectionStatus = connection_status;
}

void UGBFGameInstance::HandleControllerConnectionChange( const bool is_connection, const int32 /*unused*/, const int32 game_user_index )
{
#if PLATFORM_XBOXONE
    // update game_user_index based on previous controller index from stable index
#endif

    UE_LOG( LogGBF_OSS, Log, TEXT( "UGBFGameInstance::HandleControllerConnectionChange bIsConnection %d GameUserIndex %d" ), is_connection, game_user_index );

    if ( auto * local_player = Cast< UGBFLocalPlayer >( FindLocalPlayerFromControllerId( game_user_index ) ) )
    {
        if ( !is_connection )
        {
#if PLATFORM_XBOXONE
            auto & slate_app = FSlateApplication::Get();
            TSharedPtr< IInputProcessor > input_preprocessor = MakeShared< FGBFXBoxOneDisconnectedInputProcessor >();

            slate_app.RegisterInputPreProcessor( input_preprocessor );
#endif
            if ( auto * pc = Cast< AGBFPlayerController >( local_player->PlayerController ) )
            {
                pc->GetUIDialogManagerComponent()->ShowConfirmationPopup(
                    NSLOCTEXT( "GBF", "LocKey_SignInChange", "Gamepad disconnected" ),
                    NSLOCTEXT( "GBF", "LocKey_PlayerReconnectControllerFmt", "Please reconnect your controller." ),
                    EGBFUIDialogType::AdditiveOnlyOneVisible,
                    FGBFConfirmationPopupButtonClicked::CreateLambda( [ this
#if PLATFORM_XBOXONE
                                                                          ,
                                                                          &slate_app,
                                                                          input_preprocessor
#endif
                ]() {
#if PLATFORM_XBOXONE
                        slate_app.UnregisterInputPreProcessor( input_preprocessor );
#endif
                    } ) );
            }
        }
#if PLATFORM_PS4
        else
        {
            if ( const auto * oss = IOnlineSubsystem::Get() )
            {
                const auto identity_interface = oss->GetIdentityInterface();

                TSharedPtr< const FUniqueNetId > unique_id = identity_interface->GetUniquePlayerId( game_user_index );

                if ( ensure( unique_id.IsValid() ) && unique_id->IsValid() && CurrentUniqueNetId.IsValid() && *CurrentUniqueNetId == *unique_id )
                {
                    return;
                }

                HandleSignInChangeMessaging();
            }
        }
#endif
    }
}

void UGBFGameInstance::HandleSignInChangeMessaging()
{
    if ( !GetSubsystem< UGBFGameInstanceGameStateSystem >()->IsOnWelcomeScreenState() )
    {
#if GBF_CONSOLE_UI
        // Master user signed out, go to initial state (if we aren't there already)
        if ( const auto * settings = GetDefault< UGameBaseFrameworkSettings >() )
        {
            ShowMessageThenGotoState(
                NSLOCTEXT( "GBF", "LocKey_SignInChangeTitle", "Sign in status change" ),
                NSLOCTEXT( "GBF", "LocKey_SignInChangeContent", "Sign in status change occurred." ),
                settings->WelcomeScreenGameState.Get() );
        }
#else
        GetSubsystem< UGBFGameInstanceGameStateSystem >()->GoToWelcomeScreenState();
#endif
    }
}

void UGBFGameInstance::ShowMessageThenGotoState( const FText & title, const FText & content, UGBFGameState * next_state )
{
    if ( auto * player_controller = Cast< AGBFPlayerController >( GetWorld()->GetFirstPlayerController() ) )
    {
        if ( auto * dialog_manager_component = player_controller->GetUIDialogManagerComponent() )
        {
            const auto on_ok_clicked = FGBFConfirmationPopupButtonClicked::CreateLambda(
                [ this, &next_state ]() {
                    if ( GetSubsystem< UGBFGameInstanceGameStateSystem >()->IsStateWelcomeScreenState( next_state ) )
                    {
                        GetSubsystem< UGBFGameInstanceGameStateSystem >()->GoToWelcomeScreenState();
                    }
                    else
                    {
                        GetSubsystem< UGBFGameInstanceGameStateSystem >()->GoToState( next_state );
                    }
                } );

            dialog_manager_component->ShowConfirmationPopup( title, content, EGBFUIDialogType::AdditiveOnlyOneVisible, on_ok_clicked );
        }
    }
}

void UGBFGameInstance::OnLoginUIClosed( const TSharedPtr< const FUniqueNetId > unique_id, const int controller_index, const FOnlineError & error )
{
    IgnorePairingChangeForControllerId = -1;

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

void UGBFGameInstance::OnGameStateChanged( const UGBFGameState * new_state )
{
    if ( !GetSubsystem< UGBFGameInstanceGameStateSystem >()->IsOnWelcomeScreenState() )
    {
        return;
    }

    if ( CurrentUniqueNetId.IsValid() )
    {
        if ( auto * local_player = FindLocalPlayerFromUniqueNetId( *CurrentUniqueNetId ) )
        {
            local_player->SetCachedUniqueNetId( nullptr );
        }
    }

    CurrentUniqueNetId = nullptr;
}
