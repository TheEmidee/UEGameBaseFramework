#pragma once

#include "GameBaseFrameworkSettings.generated.h"

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

    // Begin UDeveloperSettings Interface
    virtual FName GetCategoryName() const override;

#if WITH_EDITOR
    virtual FText GetSectionText() const override;
#endif

    UPROPERTY( config, EditAnywhere, Category = GameBaseFramework )
    FGBFInputSwitchConfig InputSwitchConfig;
};