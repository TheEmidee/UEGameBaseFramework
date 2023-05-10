#pragma once

#include "GBFGameUserSettings.h"
#include "GameFramework/GBFSaveGame.h"

#include <CoreMinimal.h>
#include <DataSource/GameSettingDataSourceDynamic.h>
#include <GameSettingRegistry.h>

#include "GBFGameSettingRegistry.generated.h"

class UGameSettingCollection;
class UGBFLocalPlayer;

#define GET_SHARED_SETTINGS_FUNCTION_PATH( FunctionOrPropertyName )                                                                           \
    MakeShared< FGameSettingDataSourceDynamic >( TArray< FString >( { GET_FUNCTION_NAME_STRING_CHECKED( UGBFLocalPlayer, GetSharedSettings ), \
        GET_FUNCTION_NAME_STRING_CHECKED( UGBFSaveGame, FunctionOrPropertyName ) } ) )

#define GET_LOCAL_SETTINGS_FUNCTION_PATH( FunctionOrPropertyName )                                                                           \
    MakeShared< FGameSettingDataSourceDynamic >( TArray< FString >( { GET_FUNCTION_NAME_STRING_CHECKED( UGBFLocalPlayer, GetLocalSettings ), \
        GET_FUNCTION_NAME_STRING_CHECKED( UGBFGameUserSettings, FunctionOrPropertyName ) } ) )

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameSettingRegistry : public UGameSettingRegistry
{
    GENERATED_BODY()

protected:
    void OnInitialize( ULocalPlayer * local_player ) override;
    bool IsFinishedInitializing() const override;
    void SaveChanges() override;

    virtual UGameSettingCollection * InitializeVideoSettings( UGBFLocalPlayer * InLocalPlayer );
    /*virtual UGameSettingCollection * InitializeAudioSettings( UGBFLocalPlayer * InLocalPlayer );
    virtual UGameSettingCollection * InitializeGameplaySettings( UGBFLocalPlayer * InLocalPlayer );
    virtual UGameSettingCollection * InitializeMouseAndKeyboardSettings( UGBFLocalPlayer * InLocalPlayer );
    virtual UGameSettingCollection * InitializeGamepadSettings( UGBFLocalPlayer * InLocalPlayer );*/

    UPROPERTY()
    TObjectPtr< UGameSettingCollection > VideoSettings;

    UPROPERTY()
    TObjectPtr< UGameSettingCollection > AudioSettings;

    UPROPERTY()
    TObjectPtr< UGameSettingCollection > GameplaySettings;

    UPROPERTY()
    TObjectPtr< UGameSettingCollection > MouseAndKeyboardSettings;

    UPROPERTY()
    TObjectPtr< UGameSettingCollection > GamepadSettings;
};
