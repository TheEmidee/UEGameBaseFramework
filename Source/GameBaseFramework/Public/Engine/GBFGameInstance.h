#pragma once

#include "Engine/GameInstance.h"

#include "OnlineSubsystemTypes.h"
#include "Interfaces/OnlineExternalUIInterface.h"

#include "GBFGameInstance.generated.h"

class UGBFGameState;
class UGameBaseFrameworkSettings;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:

    UGBFGameInstance();

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnStateChangedEvent, const UGBFGameState *, new_state );
    FORCEINLINE FOnStateChangedEvent & OnStateChanged() { return OnStateChangedEvent; }

    virtual void Init() override;
    virtual void Shutdown() override;
    virtual class AGameModeBase* CreateGameModeForURL( FURL InURL ) override;

    bool IsOnWelcomeScreenState() const;

    bool Tick( float delta_seconds );
    
    UFUNCTION( BlueprintCallable )
    void GoToWelcomeScreenState();

    /*UFUNCTION( BlueprintCallable )
    bool ProfileUISwap( const int controller_index );

    bool ShowLoginUI( const int controller_index, const FOnLoginUIClosedDelegate & delegate = FOnLoginUIClosedDelegate() );
    */

private:

    const UGBFGameState * GetGameStateFromGameMode( const TSubclassOf< AGameModeBase > & game_mode_class ) const;
    const UGBFGameState * GetGameStateFromName( FName state_name ) const;
    bool IsStateWelcomeScreenState( const UGBFGameState & state ) const;

    void LoadGameStates();
    void GoToState( const UGBFGameState & new_state );
    void HandleAppWillDeactivate();
    void HandleAppHasReactivated();
    void HandleAppWillEnterBackground();
    void HandleAppHasEnteredForeground();
    void HandleAppDeactivateOrBackground();
    void HandleAppReactivateOrForeground();
    void HandleSafeFrameChanged();
    void HandleAppLicenseUpdate();
    void HandleUserLoginChanged( int32 game_user_index, ELoginStatus::Type previous_login_status, ELoginStatus::Type login_status, const FUniqueNetId & user_id );
    void HandleControllerPairingChanged( int game_user_index, const FUniqueNetId & previous_user, const FUniqueNetId & new_user );
    void HandleNetworkConnectionStatusChanged( const FString & service_name, EOnlineServerConnectionStatus::Type last_connection_status, EOnlineServerConnectionStatus::Type connection_status );
    void HandleControllerConnectionChange( bool b_is_connection, int32 unused, int32 game_user_index );
    void HandleSignInChangeMessaging();
    void ShowMessageThenGotoState( const FText & title, const FText & content, const UGBFGameState & next_state );

    UPROPERTY( BlueprintAssignable )
    FOnStateChangedEvent OnStateChangedEvent;

    UPROPERTY()
    const UGameBaseFrameworkSettings * Settings;

    EOnlineServerConnectionStatus::Type CurrentConnectionStatus;
    bool bIsLicensed;
    int IgnorePairingChangeForControllerId;
    FTickerDelegate TickDelegate;
    FDelegateHandle TickDelegateHandle;
    TSharedPtr<const FUniqueNetId> CurrentUniqueNetId;
    FOnLoginUIClosedDelegate LoginUIClosedDelegate;
    TWeakObjectPtr< const UGBFGameState > CurrentGameState;
    TArray<ELoginStatus::Type> LocalPlayerOnlineStatus;
};