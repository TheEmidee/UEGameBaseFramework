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

    virtual void Init() override;
    virtual void Shutdown() override;
    virtual class AGameModeBase* CreateGameModeForURL( FURL InURL ) override;

    /*
    bool Tick( float delta_seconds );
    
    UFUNCTION( BlueprintCallable )
    bool ProfileUISwap( const int controller_index );

    bool ShowLoginUI( const int controller_index, const FOnLoginUIClosedDelegate & delegate = FOnLoginUIClosedDelegate() );
    void SetPresenceForLocalPlayer( const FText & status );
    */
   
private:

    UGBFGameState * GetGameStateFromGameMode( const TSubclassOf< AGameModeBase > & game_mode_class ) const;

    void LoadGameStates();
    //void OnLoginUIClosed( TSharedPtr<const FUniqueNetId> UniqueId, const int controller_index );

    ELoginStatus::Type LoginStatus;
    EOnlineServerConnectionStatus::Type CurrentConnectionStatus;
    bool bIsLicensed;
    int IgnorePairingChangeForControllerId;
    FTickerDelegate TickDelegate;
    FDelegateHandle TickDelegateHandle;
    TSharedPtr<const FUniqueNetId> CurrentUniqueNetId;
    FOnLoginUIClosedDelegate LoginUIClosedDelegate;
    TWeakObjectPtr< UGBFGameState > CurrentGameState;
};