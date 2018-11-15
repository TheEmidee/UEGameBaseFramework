#pragma once

#include "Engine/DeveloperSettings.h"
#include "GameBaseFrameworkSettings.generated.h"

class UUserWidget;

class UGBFPlatformInputTextures;
class UGBFGameState;
class UGBFConfirmationWidget;

USTRUCT()
struct FGBFUIOptions
{
    GENERATED_BODY()

    UPROPERTY( EditAnywhere )
    TSubclassOf< UUserWidget > BackgroundBlurWidgetClass;

    UPROPERTY( EditAnywhere )
    TSubclassOf< UGBFConfirmationWidget > ConfirmationWidgetClass;

    UPROPERTY( EditAnywhere )
    uint8 bConfirmationButtonOnTheLeft : 1;
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

UCLASS( config = GameBaseFramework, defaultconfig, meta=( DisplayName="GameBaseFramework" ) )
class GAMEBASEFRAMEWORK_API UGameBaseFrameworkSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:

    FName GetCategoryName() const override;

#if WITH_EDITOR
    FText GetSectionText() const override;
    void PostEditChangeProperty( FPropertyChangedEvent & property_change_event ) override;

    DECLARE_MULTICAST_DELEGATE_TwoParams( FOnGameBaseFrameworkettingsChanged, const FString &, const UGameBaseFrameworkSettings * );

    static FOnGameBaseFrameworkettingsChanged & OnSettingsChanged();
#endif

    UPROPERTY( config, EditAnywhere, Category = Input )
    FGBFInputSwitchOptions InputSwitchConfig;

    UPROPERTY( config, EditAnywhere, Category = InputTextures )
    TSoftObjectPtr< UGBFPlatformInputTextures > PlatformInputTextures;

    UPROPERTY( config, EditAnywhere, Category = GameStates )
    TSoftObjectPtr< UGBFGameState > WelcomeScreenGameState;

    UPROPERTY( config, EditAnywhere, Category = GameStates )
    TArray< TSoftObjectPtr< UGBFGameState > > GameStates;

    UPROPERTY( config, EditAnywhere, Category = UI )
    FGBFUIOptions UIOptions;

protected:

#if WITH_EDITOR
    static FOnGameBaseFrameworkettingsChanged SettingsChangedDelegate;
#endif
};
