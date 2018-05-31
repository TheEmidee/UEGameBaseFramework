#pragma once

#include "Engine/DeveloperSettings.h"
#include "GameBaseFrameworkSettings.generated.h"

class UGBFPlatformInputTextures;
class UGBFGameState;

USTRUCT()
struct FGBFInputSwitchConfig
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

    virtual FName GetCategoryName() const override;

#if WITH_EDITOR
    virtual FText GetSectionText() const override;
    virtual void PostEditChangeProperty( FPropertyChangedEvent & property_change_event ) override;

    DECLARE_MULTICAST_DELEGATE_TwoParams( FOnGameBaseFrameworkettingsChanged, const FString &, const UGameBaseFrameworkSettings * );

    static FOnGameBaseFrameworkettingsChanged & OnSettingsChanged();
#endif

    UPROPERTY( config, EditAnywhere, Category = Input )
    FGBFInputSwitchConfig InputSwitchConfig;

    UPROPERTY( config, EditAnywhere, Category = InputTextures )
    TSoftObjectPtr< UGBFPlatformInputTextures > PlatformInputTextures;

    UPROPERTY( config, EditAnywhere, Category = GameStates )
    TSoftObjectPtr< UGBFGameState > WelcomeScreenGameState;

    UPROPERTY( config, EditAnywhere, Category = GameStates )
    TArray< TSoftObjectPtr< UGBFGameState > > GameStates;

protected:

#if WITH_EDITOR
    static FOnGameBaseFrameworkettingsChanged SettingsChangedDelegate;
#endif
};