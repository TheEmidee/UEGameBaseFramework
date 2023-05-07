#pragma once

#include "Player/GBFCheatManager.h"

#include <CoreMinimal.h>
#include <Engine/DeveloperSettings.h>
#include <Templates/SubclassOf.h>

#include "GameBaseFrameworkSettings.generated.h"

class UUserWidget;
class UGBFGameState;
class UCommonGameDialog;

UCLASS( config = Game, defaultconfig, MinimalAPI, meta = ( DisplayName = "GameBaseFramework" ) )
class UGameBaseFrameworkSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UGameBaseFrameworkSettings();

    FName GetCategoryName() const override;

#if WITH_EDITOR
    FText GetSectionText() const override;
    void PostEditChangeProperty( FPropertyChangedEvent & property_change_event ) override;

    GAMEBASEFRAMEWORK_API void OnPlayInEditorStarted() const;

    EDataValidationResult IsDataValid( TArray< FText > & validation_errors ) override;
#endif

    UPROPERTY( config, EditAnywhere, Category = Input )
    FGBFInputSwitchOptions InputSwitchConfig;

    UPROPERTY( config, EditAnywhere, Category = InputTextures )
    TSoftObjectPtr< UGBFPlatformInputTextures > PlatformInputTextures;

    UPROPERTY( config, EditAnywhere, Category = UI )
    FGBFUIOptions UIOptions;

    UPROPERTY( Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Tools|LevelCreator" )
    FString LevelPath;

    UPROPERTY( Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Tools|LevelCreator" )
    FString MainLevelSuffix;

    UPROPERTY( Config, EditDefaultsOnly, BlueprintReadOnly, Category = "Tools|LevelCreator" )
    TArray< FString > SubLevelSuffixes;

    UPROPERTY( config, EditAnywhere, Category = "Cheats" )
    TArray< FGBFCheatToRun > CheatsToRun;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, config, Category = "Experience", meta = ( AllowedTypes = "ExperienceDefinition" ) )
    FPrimaryAssetId DefaultExperience;

    // The experience override to use for Play in Editor (if not set, the default for the world settings of the open map will be used)
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, config, Category = "Experience", meta = ( AllowedTypes = "ExperienceDefinition" ) )
    FPrimaryAssetId ExperienceOverride;

protected:
#if WITH_EDITOR
    static FOnGameBaseFrameworkettingsChanged SettingsChangedDelegate;
#endif
};
