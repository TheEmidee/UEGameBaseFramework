#include "GBFGameInstance.h"

#include "SoftObjectPtr.h"
#include "GameFramework/GameModeBase.h"

#include "GBFGameState.h"
#include "GameBaseFrameworkSettings.h"

#if PLATFORM_XBOXONE
    // Use SlateApp input processor to show Profile Swap UI if FaceButtonBottom is pressed
#endif

#if PLATFORM_PS4
static void LOCAL_ExtendedSaveGameInfoDelegate( const TCHAR* save_name, const EGameDelegates_SaveGame key, FString & value )
{
    // Fill save parameter : size - icon path - etc...
}
#endif

UGBFGameInstance::UGBFGameInstance()
    :
    LoginStatus( ELoginStatus::NotLoggedIn )
    , bIsLicensed( true ) // Default to licensed (should have been checked by OS on boot)
    , IgnorePairingChangeForControllerId( -1 )
{
}

void UGBFGameInstance::Init()
{
    Super::Init();

    LoadGameStates();

    /*
    CurrentConnectionStatus = EOnlineServerConnectionStatus::Connected;

    const auto oss = IOnlineSubsystem::Get();
    check( oss.IsValid() );

    const auto identity_interface = oss->GetIdentityInterface();
    check( identity_interface.IsValid() );

    for ( int i = 0; i < MAX_LOCAL_PLAYERS; ++i )
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
    */
}

void UGBFGameInstance::Shutdown()
{
    Super::Shutdown();

    FTicker::GetCoreTicker().RemoveTicker( TickDelegateHandle );
}

AGameModeBase* UGBFGameInstance::CreateGameModeForURL( FURL in_url )
{
    auto * game_mode = Super::CreateGameModeForURL( in_url );

    // Workaround for when running in PIE, to set the correct state based on the game mode created by the URL
    if ( GetWorld()->WorldType != EWorldType::Game )
    {
        if ( !CurrentGameState.IsValid() )
        {
            if ( auto * current_state = GetGameStateFromGameMode( game_mode->GetClass() ) )
            {
                CurrentGameState = current_state;
            }
        }
    }

    return game_mode;
}

// -- PRIVATE

UGBFGameState * UGBFGameInstance::GetGameStateFromGameMode( const TSubclassOf< AGameModeBase > & game_mode_class ) const
{
    UGBFGameState * result = nullptr;

    if ( auto * settings = GetDefault< UGameBaseFrameworkSettings >() )
    {
        for ( auto & pair : settings->GameStates )
        {
            if ( pair.Value.Get()->GameModeClass == game_mode_class )
            {
                result = pair.Value.Get();
            }
        }
    }

    return result;
}

void UGBFGameInstance::LoadGameStates()
{
    if ( auto * settings = GetDefault< UGameBaseFrameworkSettings >() )
    {
        for ( auto & pair : settings->GameStates )
        {
            if ( pair.Value.Get() == nullptr )
            {
                pair.Value.LoadSynchronous();
            }
        }
    }
}