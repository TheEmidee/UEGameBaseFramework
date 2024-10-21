#pragma once

#include <CoreMinimal.h>
#include <GameFramework/SaveGame.h>

#include "GBFSaveGame.generated.h"

class IGBFSavableInterface;

USTRUCT()
struct FGBFSavableData
{
    GENERATED_BODY()

    UPROPERTY()
    TArray< uint8 > Data;
};

DECLARE_MULTICAST_DELEGATE( FGBFOnOnSaveGameResetDelegate );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFSaveGame : public ULocalPlayerSaveGame
{
    GENERATED_BODY()

public:
    void RegisterSavable( IGBFSavableInterface * savable );
    void UnRegisterSavable( IGBFSavableInterface * savable );

    void HandlePreSave() override;
    void HandlePostLoad() override;

    FGBFOnOnSaveGameResetDelegate & OnSaveGameReset();

    void ResetToDefault() override;

private:
    void SaveSavable( FName identifier, UObject * object );
    void LoadSavable( FName identifier, UObject * object );

    UPROPERTY( Transient )
    TMap< FName, TObjectPtr< UObject > > Savables;

    UPROPERTY( SaveGame )
    TMap< FName, FGBFSavableData > SavablesData;

    FGBFOnOnSaveGameResetDelegate OnSaveGameResetDelegate;
};

FORCEINLINE FGBFOnOnSaveGameResetDelegate & UGBFSaveGame::OnSaveGameReset()
{
    return OnSaveGameResetDelegate;
}