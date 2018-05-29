#pragma once

#include "Engine/GameInstance.h"

#include "OnlineSubsystemTypes.h"
#include "Interfaces/OnlineExternalUIInterface.h"

#include "GBFGameInstance.generated.h"

class UGBFGameState;

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

    bool Tick( float delta_seconds );
    
    UFUNCTION( BlueprintCallable )
    void GoToWelcomeScreenState();

    /*UFUNCTION( BlueprintCallable )
    bool ProfileUISwap( const int controller_index );

    bool ShowLoginUI( const int controller_index, const FOnLoginUIClosedDelegate & delegate = FOnLoginUIClosedDelegate() );
    void SetPresenceForLocalPlayer( const FText & status );
    */

private:

    const UGBFGameState * GetGameStateFromGameMode( const TSubclassOf< AGameModeBase > & game_mode_class ) const;
    const UGBFGameState * GetGameStateFromName( FName state_name ) const;

    void LoadGameStates();
    void GoToState( const UGBFGameState & new_state );

    //void OnLoginUIClosed( TSharedPtr<const FUniqueNetId> UniqueId, const int controller_index );

    UPROPERTY( BlueprintAssignable )
    FOnStateChangedEvent OnStateChangedEvent;

    ELoginStatus::Type LoginStatus;
    EOnlineServerConnectionStatus::Type CurrentConnectionStatus;
    bool bIsLicensed;
    int IgnorePairingChangeForControllerId;
    FTickerDelegate TickDelegate;
    FDelegateHandle TickDelegateHandle;
    TSharedPtr<const FUniqueNetId> CurrentUniqueNetId;
    FOnLoginUIClosedDelegate LoginUIClosedDelegate;
    TWeakObjectPtr< const UGBFGameState > CurrentGameState;
};