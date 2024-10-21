#pragma once

#include <CoreMinimal.h>
#include <Subsystems/GameInstanceSubsystem.h>

#include "GBFSaveGameSubsystem.generated.h"

class UGBFSaveGame;

DECLARE_DYNAMIC_DELEGATE_OneParam( FGBFOnSaveGameLoadedDynamicDelegate, UGBFSaveGame *, SaveGame );
DECLARE_DELEGATE_OneParam( FGBFOnSaveGameLoadedDelegate, UGBFSaveGame * SaveGame );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFSaveGameSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    void Initialize( FSubsystemCollectionBase & collection ) override;
    void NotifyPlayerAdded( ULocalPlayer * local_player );

    UFUNCTION( BlueprintCallable )
    void Load();

    UFUNCTION( BlueprintCallable )
    void Save();

    template < typename _SAVE_GAME_CLASS_ >
    _SAVE_GAME_CLASS_ * GetSaveGame() const;

    void WhenSaveGameIsLoaded( const FGBFOnSaveGameLoadedDelegate & when_save_game_is_loaded );

    static UGBFSaveGameSubsystem * Get( const UObject * world_context );

private:
    UPROPERTY( transient, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    TObjectPtr< UGBFSaveGame > SaveGame;

    TArray< FGBFOnSaveGameLoadedDelegate > SaveGameLoadedObservers;
    TWeakObjectPtr< ULocalPlayer > PrimaryPlayer;
};

template < typename _SAVE_GAME_CLASS_ >
_SAVE_GAME_CLASS_ * UGBFSaveGameSubsystem::GetSaveGame() const
{
    return Cast< _SAVE_GAME_CLASS_ >( SaveGame );
}
