#pragma once

#include "GBFSaveGame.h"
#include "Containers/Deque.h"
#include "Math/MovingAverage.h"

#include <CoreMinimal.h>
#include <Subsystems/GameInstanceSubsystem.h>

#include "GBFSaveGameSubsystem.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam( FGBFOnSaveGameLoaded, UGBFSaveGame *, SaveGame );
DECLARE_DYNAMIC_DELEGATE_TwoParams( FGBFOnSaveGameSaved, UGBFSaveGame *, SaveGame, bool, Success );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFSaveGameSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    void Initialize( FSubsystemCollectionBase & collection ) override;

    void NotifyPlayerAdded( ULocalPlayer * local_player );

    UFUNCTION( BlueprintCallable )
    bool Load( FGBFOnSaveGameLoaded on_save_game_loaded );

    UFUNCTION( BlueprintCallable )
    bool Save( FGBFOnSaveGameSaved on_save_game_saved );

    UFUNCTION( BlueprintCallable )
    void SaveNextTick( FGBFOnSaveGameSaved on_save_game_saved );

    UFUNCTION( BlueprintCallable )
    void SaveWithDelay( float delay, FGBFOnSaveGameSaved on_save_game_saved );

    UFUNCTION( BlueprintCallable )
    void Reset();

    void RegisterSavable( const TScriptInterface< IGBFSaveGameSystemSavableInterface > & savable );
    void UnRegisterSavable( const TScriptInterface< IGBFSaveGameSystemSavableInterface > & savable );

    template < typename _SAVE_GAME_CLASS_ >
    _SAVE_GAME_CLASS_ * GetSaveGame() const;

    static UGBFSaveGameSubsystem * Get( const UObject * world_context );

private:
    UPROPERTY( transient, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    TObjectPtr< UGBFSaveGame > SaveGame;

    UPROPERTY()
    TArray< TScriptInterface< IGBFSaveGameSystemSavableInterface > > PendingSavables;

    TWeakObjectPtr< ULocalPlayer > PrimaryPlayer;

    TDeque< float > LoadGameCallTimes;
    TDeque< float > SaveGameCallTimes;
};

template < typename _SAVE_GAME_CLASS_ >
_SAVE_GAME_CLASS_ * UGBFSaveGameSubsystem::GetSaveGame() const
{
    return Cast< _SAVE_GAME_CLASS_ >( SaveGame );
}
