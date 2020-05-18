#include "Engine/GBFGameInstance.h"

#include "Components/GBFUIDialogManagerComponent.h"
#include "Engine/GBFLocalPlayer.h"
#include "Engine/GBFGameState.h"
#include "Engine/SubSystems/GBFGameInstanceCoreDelegatesSubsystem.h"
#include "Engine/SubSystems/GBFGameInstanceGameStateSystem.h"
#include "Engine/SubSystems/GBFGameInstanceIdentitySubsystem.h"
#include "Engine/SubSystems/GBFGameInstanceSessionSubsystem.h"
#include "GBFTypes.h"
#include "GameBaseFrameworkSettings.h"
#include "GameFramework/GBFGameModeBase.h"
#include "GameFramework/GBFPlayerController.h"
#include "Log/GBFLog.h"
#include "Online/GBFOnlineSessionClient.h"

#include <Engine/AssetManager.h>
#include <Kismet/GameplayStatics.h>
#include <Sound/SoundMix.h>

#if PLATFORM_PS4
static void ExtendedSaveGameInfoDelegate( const TCHAR * save_name, const EGameDelegates_SaveGame key, FString & value )
{
    // Fill save parameter : size - icon path - etc...
}
#endif

UGBFGameInstance::UGBFGameInstance()
{}

void UGBFGameInstance::Init()
{
    Super::Init();

    Settings = GetDefault< UGameBaseFrameworkSettings >();

    check( Settings != nullptr );

#if PLATFORM_PS4
    FGameDelegates::Get().GetExtendedSaveGameInfoDelegate() = FExtendedSaveGameInfoDelegate::CreateStatic( LOCAL_ExtendedSaveGameInfoDelegate );
#endif

    if ( SoundMix.IsValid() )
    {
        UAssetManager::GetStreamableManager().RequestAsyncLoad( SoundMix.ToSoftObjectPath() );
    }

    if ( !IsDedicatedServerInstance() )
    {
        TickDelegate = FTickerDelegate::CreateUObject( this, &UGBFGameInstance::Tick );
        TickDelegateHandle = FTicker::GetCoreTicker().AddTicker( TickDelegate );
    }

    GameStateSubsystem = GetSubsystem< UGBFGameInstanceGameStateSystem >();
    IdentitySubsystem = GetSubsystem< UGBFGameInstanceIdentitySubsystem >();
    SessionSubsystem = GetSubsystem< UGBFGameInstanceSessionSubsystem >();
}

void UGBFGameInstance::Shutdown()
{
    Super::Shutdown();

    FTicker::GetCoreTicker().RemoveTicker( TickDelegateHandle );
}

AGameModeBase * UGBFGameInstance::CreateGameModeForURL( const FURL url )
{
    auto * game_mode = Super::CreateGameModeForURL( url );

    GameStateSubsystem->UpdateCurrentGameStateFromCurrentWorld();

    return game_mode;
}

bool UGBFGameInstance::Tick( float /*delta_seconds*/ )
{
    if ( !GameStateSubsystem->IsOnWelcomeScreenState() && LocalPlayers.Num() > 0 )
    {
        if ( auto * local_player = Cast< UGBFLocalPlayer >( LocalPlayers[ 0 ] ) )
        {
            if ( auto * player_controller = Cast< AGBFPlayerController >( local_player->PlayerController ) )
            {
                const auto is_displaying_dialog = player_controller->GetUIDialogManagerComponent()->IsDisplayingDialog();

                // If at any point we aren't licensed (but we are after welcome screen) bounce them back to the welcome screen
                if ( !GetSubsystem< UGBFGameInstanceCoreDelegatesSubsystem >()->IsLicensed() && !is_displaying_dialog )
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

    SessionSubsystem->HandlePendingSessionInvite();

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

ULocalPlayer * UGBFGameInstance::GetFirstLocalPlayer() const
{
    return LocalPlayers.Num() > 0
               ? LocalPlayers[ 0 ]
               : nullptr;
}

void UGBFGameInstance::ShowMessageThenGotoState( const FText & title, const FText & content, UGBFGameState * next_state )
{
    if ( auto * player_controller = Cast< AGBFPlayerController >( GetWorld()->GetFirstPlayerController() ) )
    {
        if ( auto * dialog_manager_component = player_controller->GetUIDialogManagerComponent() )
        {
            const auto on_ok_clicked = FGBFConfirmationPopupButtonClicked::CreateLambda(
                [this, &next_state]() {
                    if ( GameStateSubsystem->IsStateWelcomeScreenState( next_state ) )
                    {
                        GameStateSubsystem->GoToWelcomeScreenState();
                    }
                    else
                    {
                        GameStateSubsystem->GoToState( next_state );
                    }
                } );

            dialog_manager_component->ShowConfirmationPopup( title, content, EGBFUIDialogType::AdditiveOnlyOneVisible, on_ok_clicked );
        }
    }
}

void UGBFGameInstance::ShowMessageThenGotoWelcomeScreenState( const FText & title, const FText & content )
{
    ShowMessageThenGotoState( title, content, Settings->WelcomeScreenGameState.Get() );
}

void UGBFGameInstance::ShowMessageThenGotoMainMenuState( const FText & title, const FText & content )
{
    ShowMessageThenGotoState( title, content, Settings->MainMenuGameState.Get() );
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UGBFGameInstance::HandleSignInChangeMessaging()
{
    if ( !GameStateSubsystem->IsOnWelcomeScreenState() )
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
        GameStateSubsystem->GoToWelcomeScreenState();
#endif
    }
}

void UGBFGameInstance::RemoveSplitScreenPlayers()
{
    while ( LocalPlayers.Num() > 1 )
    {
        auto * player_to_remove = LocalPlayers.Last();
        RemoveExistingLocalPlayer( player_to_remove );
    }
}

void UGBFGameInstance::RemoveExistingLocalPlayer( ULocalPlayer * local_player )
{
    check( local_player );
    if ( local_player->PlayerController != nullptr )
    {
        OnExistingLocalPlayerRemovedDelegate.Broadcast( local_player );
    }

    // Remove local split-screen players from the list
    RemoveLocalPlayer( local_player );
}

TSubclassOf< UOnlineSession > UGBFGameInstance::GetOnlineSessionClass()
{
    return UGBFOnlineSessionClient::StaticClass();
}

void UGBFGameInstance::OnAppReactivateOrForeground()
{
    if ( !GameStateSubsystem->IsOnWelcomeScreenState() )
    {
        UE_LOG( LogGBF_OSS, Warning, TEXT( "UGBFGameInstanceCoreDelegatesSubsystem::HandleAppReactivateOrForeground: Attempting to sign out players" ) );

        for ( auto player_index = 0; player_index < LocalPlayers.Num(); ++player_index )
        {
            if ( auto * lp = Cast< UGBFLocalPlayer >( LocalPlayers[ player_index ] ) )
            {
                const auto local_player_online_status = IdentitySubsystem->GetLocalPlayerOnlineStatus( player_index );
                if ( lp->GetCachedUniqueNetId().IsValid() && local_player_online_status && lp->GetLoginStatus() != ELoginStatus::LoggedIn )
                {
                    UE_LOG( LogGBF_OSS, Log, TEXT( "UGBFGameInstanceCoreDelegatesSubsystem::HandleAppReactivateOrForeground: Signed out during resume." ) );
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
