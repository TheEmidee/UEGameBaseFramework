#pragma once

#include "GBFSaveGame.h"

#include <CoreMinimal.h>
#include <Subsystems/GameInstanceSubsystem.h>

#include "GBFSaveGameSubsystem.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFSaveGameSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    void NotifyPlayerAdded( ULocalPlayer * local_player );

    UFUNCTION( BlueprintCallable )
    void Load();

    UFUNCTION( BlueprintCallable )
    void Save();

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
};

template < typename _SAVE_GAME_CLASS_ >
_SAVE_GAME_CLASS_ * UGBFSaveGameSubsystem::GetSaveGame() const
{
    return Cast< _SAVE_GAME_CLASS_ >( SaveGame );
}
