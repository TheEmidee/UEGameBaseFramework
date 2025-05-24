#pragma once

#include <CoreMinimal.h>
#include <GameFramework/SaveGame.h>
#include <Serialization/MemoryReader.h>
#include <Serialization/MemoryWriter.h>
#include <Serialization/ObjectAndNameAsStringProxyArchive.h>
#include <Templates/ChooseClass.h>

#include "GBFSaveGame.generated.h"

namespace GameBaseFramework
{
    namespace SaveGameSystem
    {
        namespace Helpers
        {
            template < bool bIsLoading >
            FORCEINLINE void SerializeObject( UObject & object, TArray< uint8 > & data )
            {
                using TMemoryClass = typename TChooseClass< bIsLoading, FMemoryReader, FMemoryWriter >::Result;

                TMemoryClass memory_archive( data );
                FObjectAndNameAsStringProxyArchive archive( memory_archive, true );
                archive.ArIsSaveGame = true;
                object.Serialize( archive );
            }
        }
    }
}

UINTERFACE( MinimalAPI, meta = ( CannotImplementInterfaceInBlueprint ) )
class UGBFSaveGameSystemSavableInterface : public UInterface
{
    GENERATED_UINTERFACE_BODY()
};

class GAMEBASEFRAMEWORK_API IGBFSaveGameSystemSavableInterface
{
    GENERATED_IINTERFACE_BODY()

    virtual void OnSaveGameReset();
    virtual bool CanBeSerialized() const;
};

USTRUCT()
struct FGBFSavableData
{
    GENERATED_BODY()

    FGBFSavableData() = default;

    FGBFSavableData( const TScriptInterface< IGBFSaveGameSystemSavableInterface > & savable, const FSoftClassPath & class_path, const TArray< uint8 > & data ) :
        Savable( savable ),
        ClassPath( class_path ),
        Data( data )
    {
    }

    void Reset();

    UPROPERTY( Transient )
    TScriptInterface< IGBFSaveGameSystemSavableInterface > Savable;

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
    friend class UGBFSaveGameSubsystem;

    void HandlePreSave() override;
    void HandlePostLoad() override;

    FGBFOnOnSaveGameResetDelegate & OnSaveGameReset();

    void ResetToDefault() override;

private:
    void RegisterSavable( TScriptInterface< IGBFSaveGameSystemSavableInterface > savable );
    void UnRegisterSavable( const TScriptInterface< IGBFSaveGameSystemSavableInterface > & savable );

    UPROPERTY( SaveGame )
    TArray< FGBFSavableData > SavablesData;

    FGBFOnOnSaveGameResetDelegate OnSaveGameResetDelegate;
};

FORCEINLINE FGBFOnOnSaveGameResetDelegate & UGBFSaveGame::OnSaveGameReset()
{
    return OnSaveGameResetDelegate;
}