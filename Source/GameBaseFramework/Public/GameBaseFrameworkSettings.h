#pragma once

#include "Player/GBFCheatManager.h"

#include <CoreMinimal.h>
#include <Engine/DeveloperSettings.h>
#include <Templates/SubclassOf.h>

#include "GameBaseFrameworkSettings.generated.h"

class UUserWidget;

class UGBFPlatformInputTextures;
class UGBFGameState;
class UGBFConfirmationWidget;

USTRUCT()
struct FGBFUIOptions
{
    GENERATED_BODY()

    UPROPERTY( config, EditAnywhere, Category = UI )
    TSubclassOf< UUserWidget > BackgroundBlurWidgetClass;

    UPROPERTY( config, EditAnywhere, Category = UI )
    TSubclassOf< UGBFConfirmationWidget > ConfirmationWidgetClass;

    UPROPERTY( config, EditAnywhere, Category = UI )
    uint8 IsConfirmationButtonOnTheLeft : 1;
};

USTRUCT()
struct FGBFInputSwitchOptions
{
    GENERATED_BODY()

    /* Minimum time to wait before switching to another input type */
    UPROPERTY( config, EditAnywhere, Category = InputSwitch )
    float MinTimeToSwitch;

    /* Minimum value for an axis to trigger an input switch */
    UPROPERTY( config, EditAnywhere, Category = InputSwitch )
    float AxisMinThreshold;

    /* Minimum delta when the mouse moves to trigger an input switch */
    UPROPERTY( config, EditAnywhere, Category = InputSwitch )
    float MouseMoveMinDelta;
};

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

    DECLARE_MULTICAST_DELEGATE_TwoParams( FOnGameBaseFrameworkettingsChanged, const FString &, const UGameBaseFrameworkSettings * )

    GAMEBASEFRAMEWORK_API static FOnGameBaseFrameworkettingsChanged & OnSettingsChanged();
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

    // The experience override to use for Play in Editor (if not set, the default for the world settings of the open map will be used)
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, config, Category = "Experience", meta = ( AllowedTypes = "GBFExperienceDefinition" ) )
    FPrimaryAssetId ExperienceOverride;

protected:
#if WITH_EDITOR
    static FOnGameBaseFrameworkettingsChanged SettingsChangedDelegate;
#endif
};
