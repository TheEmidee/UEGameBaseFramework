#pragma once

#include "GBFGameUserSettings.h"
#include "GameFramework/GBFSettingsShared.h"

#include <CoreMinimal.h>
#include <GameSettingRegistry.h>

#include "GBFGameSettingRegistry.generated.h"

class UGameSettingCollection;
class UGBFLocalPlayer;

DECLARE_LOG_CATEGORY_EXTERN( LogGBFGameSettingRegistry, Log, Log );

#define GET_SHARED_SETTINGS_FUNCTION_PATH( FunctionOrPropertyName )                                                                           \
    MakeShared< FGameSettingDataSourceDynamic >( TArray< FString >( { GET_FUNCTION_NAME_STRING_CHECKED( UGBFLocalPlayer, GetSharedSettings ), \
        GET_FUNCTION_NAME_STRING_CHECKED( UGBFSettingsShared, FunctionOrPropertyName ) } ) )

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

    virtual UGameSettingCollection * InitializeVideoSettings( UGBFLocalPlayer * local_player );
    virtual UGameSettingCollection * InitializeAudioSettings( UGBFLocalPlayer * local_player );
    virtual UGameSettingCollection * InitializeGameplaySettings( UGBFLocalPlayer * local_player );
    virtual UGameSettingCollection * InitializeMouseAndKeyboardSettings( UGBFLocalPlayer * local_player );
    virtual UGameSettingCollection * InitializeGamepadSettings( UGBFLocalPlayer * local_player );

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
