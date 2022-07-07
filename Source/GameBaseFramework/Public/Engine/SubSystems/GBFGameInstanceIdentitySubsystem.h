#pragma once

#include "GBFGameInstanceSubsystemBase.h"

#include <CoreMinimal.h>
#include <Online.h>

#include "GBFGameInstanceIdentitySubsystem.generated.h"

class UGBFGameState;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameInstanceIdentitySubsystem final : public UGBFGameInstanceSubsystemBase
{
    GENERATED_BODY()

public:
    const TSharedPtr< const FUniqueNetId > & GetCurrentUniqueNetId() const;
    ELoginStatus::Type GetLocalPlayerOnlineStatus( int player_index ) const;

    void Initialize( FSubsystemCollectionBase & collection ) override;

    UFUNCTION( BlueprintCallable )
    bool ProfileUISwap( int controller_index );

    bool ShowLoginUI( int controller_index, const FOnLoginUIClosedDelegate & delegate = FOnLoginUIClosedDelegate() );
    ULocalPlayer * GetLocalPlayerFromUniqueNetId( const FUniqueNetId & user_id ) const;
    APlayerController * GetPlayerControllerFromUniqueNetId( const FUniqueNetId & user_id ) const;

    bool IsLocalPlayerOnline( ULocalPlayer * local_player ) const;
    bool IsLocalPlayerSignedIn( ULocalPlayer * local_player ) const;
    bool ValidatePlayerForOnlinePlay( ULocalPlayer * local_player ) const;
    bool ValidatePlayerIsSignedIn( ULocalPlayer * local_player ) const;

private:
    void HandleUserLoginChanged( int32 game_user_index, ELoginStatus::Type previous_login_status, ELoginStatus::Type login_status, const FUniqueNetId & user_id );
    void OnLoginUIClosed( TSharedPtr< const FUniqueNetId > unique_id, int controller_index, const FOnlineError & error );

    FOnLoginUIClosedDelegate LoginUIClosedDelegate;

    TArray< ELoginStatus::Type > LocalPlayerOnlineStatus;
    TSharedPtr< const FUniqueNetId > CurrentUniqueNetId;
};

FORCEINLINE const TSharedPtr< const FUniqueNetId > & UGBFGameInstanceIdentitySubsystem::GetCurrentUniqueNetId() const
{
    return CurrentUniqueNetId;
}

FORCEINLINE ELoginStatus::Type UGBFGameInstanceIdentitySubsystem::GetLocalPlayerOnlineStatus( const int player_index ) const
{
    return LocalPlayerOnlineStatus[ player_index ];
}
