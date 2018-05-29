#include "GBFGameInstance.h"

#include "SoftObjectPtr.h"
#include "Containers/Ticker.h"
#include "GameFramework/GameModeBase.h"

#include "GBFGameState.h"
#include "GBFLocalPlayer.h"
#include "GameBaseFrameworkSettings.h"
#include "BlueprintLibraries/GBFHelperBlueprintLibrary.h"

#if PLATFORM_XBOXONE
class FGBFXBoxOneDisconnectedInputProcessor : public IInputProcessor
{
public:
    virtual void Tick( const float delta_time, FSlateApplication & slate_application, TSharedRef<ICursor> cursor ) {};

    virtual bool HandleKeyUpEvent( FSlateApplication & slate_application, const FKeyEvent & key_event) override
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
    */
    
    TickDelegate = FTickerDelegate::CreateUObject( this, &UGBFGameInstance::Tick );
    TickDelegateHandle = FTicker::GetCoreTicker().AddTicker( TickDelegate );
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

bool UGBFGameInstance::Tick( float delta_seconds )
{
    if ( const auto * settings = GetDefault< UGameBaseFrameworkSettings >() )
    {
        if ( CurrentGameState.Get() != settings->WelcomeScreenGameState.Get()
            && LocalPlayers.Num() > 0 )
        {
            if ( auto * local_player = Cast< UGBFLocalPlayer >( LocalPlayers[ 0 ] ) )
            {
                /*if ( auto * player_controller = local_player->GetPlayerControllerBase() )
                {
                    const auto is_displaying_dialog = player_controller->GetDialogManagerComponent().IsDisplayingDialog();

                    // If at any point we aren't licensed (but we are after welcome screen) bounce them back to the welcome screen
                    if ( !bIsLicensed
                        && !is_displaying_dialog
                        )
                    {
                        ShowMessageThenGotoState(
                            SQLocalization::NeedLicenseTextTitle,
                            SQLocalization::NeedLicenseTextContent,
                            ESQGameState::WelcomeScreen
                        );

                        return true;
                    }

                    TRY TO HANDLE THE FOLLOWING CODE IN HandleControllerConnectionChange

    #if SQ_CONSOLE_UI
                    if ( GamePadDisconnectedConfirmationWidget == nullptr )
                    {
                        if ( local_player->GetGamepadDisconnected() )
                        {
    #if PLATFORM_XBOXONE
                            auto & slate_app = FSlateApplication::Get();
                            TSharedPtr<IInputProcessor> input_preprocessor = MakeShared< FGBFXBoxOneDisconnectedInputProcessor >();

                            slate_app.RegisterInputPreProcessor( input_preprocessor );
    #endif

                            GamePadDisconnectedConfirmationWidget = player_controller->GetDialogManagerComponent().ShowConfirmationPopup(
                                NSLOCTEXT( "SQ", "LocKey_SignInChange", "Sign in status change occurred." ),
                                NSLOCTEXT( "SQ", "LocKey_PlayerReconnectControllerFmt", "Please reconnect your controller." ),
                                FSQConfirmationPopupButtonClicked::CreateLambda( [ this
    #if PLATFORM_XBOXONE
                                    , &slate_app, input_preprocessor
    #endif
                                ] ( )
                            {
                                GamePadDisconnectedConfirmationWidget = nullptr;
    #if PLATFORM_XBOXONE
                                slate_app.UnregisterInputPreProcessor( input_preprocessor );
    #endif
                            } )
                            );
                        }
                    }
    #endif
                }
                */
            }
        }
    }

    return true;
}

void UGBFGameInstance::GoToWelcomeScreenState()
{
    if ( const auto * settings = GetDefault< UGameBaseFrameworkSettings >() )
    {
        if ( CurrentGameState == settings->WelcomeScreenGameState.Get() )
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

        GoToState( *settings->WelcomeScreenGameState );
    }
}

// -- PRIVATE

const UGBFGameState * UGBFGameInstance::GetGameStateFromGameMode( const TSubclassOf< AGameModeBase > & game_mode_class ) const
{
    UGBFGameState * result = nullptr;

    if ( auto * settings = GetDefault< UGameBaseFrameworkSettings >() )
    {
        auto predicate = [ game_mode_class ] ( auto state_soft_ptr )
        {
            return state_soft_ptr.Get()->GameModeClass == game_mode_class;
        };

        return settings->GameStates.FindByPredicate( predicate )->Get();
    }

    return result;
}

const UGBFGameState * UGBFGameInstance::GetGameStateFromName( FName state_name ) const
{
    UGBFGameState * result = nullptr;

    if ( auto * settings = GetDefault< UGameBaseFrameworkSettings >() )
    {
        auto predicate = [ state_name ] ( auto state_soft_ptr )
        {
            return state_soft_ptr.Get()->Name == state_name;
        };

        return settings->GameStates.FindByPredicate( predicate )->Get();
    }

    return result;
}

void UGBFGameInstance::LoadGameStates()
{
    if ( auto * settings = GetDefault< UGameBaseFrameworkSettings >() )
    {
        settings->WelcomeScreenGameState.LoadSynchronous();

        for ( auto & game_state : settings->GameStates )
        {
            if ( game_state.Get() == nullptr )
            {
                game_state.LoadSynchronous();
            }
        }
    }
}

void UGBFGameInstance::GoToState( const UGBFGameState & new_state )
{
    if ( CurrentGameState.Get() != &new_state )
    {
        CurrentGameState = &new_state;

        UGBFHelperBlueprintLibrary::OpenMap( this, new_state.Map );

        OnStateChangedEvent.Broadcast( &new_state );
    }
}