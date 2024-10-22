#pragma once

#include <CoreMinimal.h>
#include <GameFramework/SaveGame.h>

#include "GBFSaveGame.generated.h"

class IGBFSavableInterface;

USTRUCT()
struct FGBFSavableData
{
    GENERATED_BODY()

    UPROPERTY( Transient )
    TObjectPtr< UObject > Object;

    UPROPERTY()
    FSoftClassPath ClassPath;

    UPROPERTY()
    TArray< uint8 > Data;
};

DECLARE_MULTICAST_DELEGATE( FGBFOnOnSaveGameResetDelegate );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFSaveGame : public ULocalPlayerSaveGame
{
    GENERATED_BODY()

public:
    void RegisterSavable( UObject * savable );
    void UnRegisterSavable( UObject * savable );

    void HandlePreSave() override;
    void HandlePostLoad() override;

    FGBFOnOnSaveGameResetDelegate & OnSaveGameReset();

    void ResetToDefault() override;

private:
    UPROPERTY( SaveGame )
    TArray< FGBFSavableData > SavablesData;

    FGBFOnOnSaveGameResetDelegate OnSaveGameResetDelegate;
};

FORCEINLINE FGBFOnOnSaveGameResetDelegate & UGBFSaveGame::OnSaveGameReset()
{
    return OnSaveGameResetDelegate;
}